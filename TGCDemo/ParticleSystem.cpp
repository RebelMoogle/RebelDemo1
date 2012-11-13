#include "DXUT.h"
#include "ParticleSystem.h"
#include "constants.h"
#include <random>


ParticleSystem::ParticleSystem(void)
{
	mMaxParticleCount = 50000;
	mSystemChange = false;
	mSystemCB = new ConstantBuffer<PARTICLESYSTEMDATA>();
	mFrameCB = new ConstantBuffer<PARTICLEFRAMEDATA>();
	mInitParticle = true;
}


ParticleSystem::~ParticleSystem(void)
{
}

bool ParticleSystem::D3DCreateDevice( ID3D11Device* Device, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc )
{

	// ## Particle System Shaders ##

	// load Particle System Computation shaders
	mParticleSystemVS = new VertexShader(Device, L"Shaders\\GPUParticles.hlsl", "ParticleSystemVS");
	mParticleSystemGS = NULL; //have to create GS with output buffer

	// ## -> create Geometry Shader with stream output
	{
		UINT shaderFlags = D3D10_SHADER_ENABLE_STRICTNESS | D3D10_SHADER_PACK_MATRIX_ROW_MAJOR;

#if defined( DEBUG ) || defined( _DEBUG )
		shaderFlags |= D3D10_SHADER_DEBUG;
#endif

		ID3D10Blob *bytecode = 0;
		ID3D10Blob *errors = 0;
		HRESULT hr = D3DX11CompileFromFile( L"Shaders\\GPUParticles.hlsl", NULL, 0, "ParticleSystemGS", "gs_5_0", shaderFlags, 0, 0, &bytecode, &errors, 0);
		if (errors) {
			OutputDebugStringA(static_cast<const char *>(errors->GetBufferPointer()));
			//return false;
		}

		if (FAILED(hr)) {
			DXUT_ERR_MSGBOX(L"failed to compile Particle System Geometry Shader", hr);
			return false;
		}
		D3D11_SO_DECLARATION_ENTRY ParticleBufferOutputDecl[] =
		{
			{ 0, "SV_POSITION", 0, 0, 4, 0},
			{ 0, "INITIALRANDOM", 0, 0, 4, 0},
			{ 0, "COLOR", 0, 0, 4, 0},
			{ 0, "DIRECTION", 0, 0, 3, 0},
			{ 0, "DURATION", 0, 0, 1, 0},
			{ 0, "SPEED", 0, 0, 1, 0},
			{ 0, "ROTATION", 0, 0, 1, 0},
			{ 0, "SIZE", 0, 0, 1, 0},
			{ 0, "FLAGS", 0, 0, 1, 0}
		};
		UINT strides[] = { sizeof(PARTICLEDATA) };

		hr = Device->CreateGeometryShaderWithStreamOutput(	bytecode->GetBufferPointer(), 
			bytecode->GetBufferSize(), 
			ParticleBufferOutputDecl, 
			ARRAYSIZE(ParticleBufferOutputDecl), 
			strides, 
			1, 
			D3D11_SO_NO_RASTERIZED_STREAM, 
			NULL, 
			&mParticleSystemGS );

		if (FAILED(hr)) {
			DXUT_ERR_MSGBOX(L"failed to create Particle System Geometry Shader with Stream Output", hr);
			return false;
		}

		bytecode->Release();
	}
	
	// load Particle Rendering shaders
	mRenderParticlesVS = new VertexShader(Device, L"Shaders\\RenderParticles.hlsl", "RenderParticlesVS");
	mRenderParticlesGS = new GeometryShader(Device, L"Shaders\\RenderParticles.hlsl", "RenderParticlesGS");
	mRenderParticlesPS = new PixelShader(Device, L"Shaders\\RenderParticles.hlsl", "RenderParticlesPS");

	// create particle input layout
	{
		ID3D10Blob *bytecode = mRenderParticlesVS->GetByteCode();

		const D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{"POSITION",		0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,	0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"INITIALRANDOM",	0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR",			0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"DIRECTION",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0,	48, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"DURATION",		0, DXGI_FORMAT_R32_FLOAT,		0, 60, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"SPEED",			0, DXGI_FORMAT_R32_FLOAT,		0, 64, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"ROTATION",		0, DXGI_FORMAT_R32_FLOAT,		0, 68, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"SIZE",			0, DXGI_FORMAT_R32_FLOAT,		0, 72, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"FLAGS",			0, DXGI_FORMAT_R32_UINT,		0, 76, D3D11_INPUT_PER_VERTEX_DATA, 0}
		}; //total size is 64

		HRESULT hr = Device->CreateInputLayout(
								layout, ARRAYSIZE(layout),
								bytecode->GetBufferPointer(),
								bytecode->GetBufferSize(),
								&mParticleLayout);
		if (FAILED(hr)) { 
			DXUT_ERR_MSGBOX(L"failed to create Particle input layout", hr);
			return false;
		};
	}

	// create Rasterizer state
	{
		CD3D11_RASTERIZER_DESC desc(D3D11_DEFAULT);
		desc.CullMode = D3D11_CULL_NONE;
		desc.DepthClipEnable = FALSE;
		desc.FillMode = D3D11_FILL_SOLID;
		desc.FrontCounterClockwise = FALSE;
		HRESULT hr = Device->CreateRasterizerState(&desc, &mRasterizerState);

		if (FAILED(hr)) {
			DXUT_ERR_MSGBOX(L"failed to create Particle Render rasterizer state", hr);
			return false;
		}
	}

	// Create sampler state
	{
		CD3D11_SAMPLER_DESC desc(D3D11_DEFAULT);
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		Device->CreateSamplerState(&desc, &mDiffuseSampler);
	}

	// Create Depth State
	{
		CD3D11_DEPTH_STENCIL_DESC desc(D3D11_DEFAULT);
		desc.StencilEnable = false;
		desc.DepthEnable = true;
		desc.DepthFunc = D3D11_COMPARISON_LESS;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		HRESULT hr = Device->CreateDepthStencilState(&desc, &mDepthState);

		if (FAILED(hr)) {
			DXUT_ERR_MSGBOX(L"failed to create Particle Render depth state", hr);
			return false;
		}
	}

	// Create particle blend state
	{
		// splat particles, blend onto backbuffer or rendertarget
		CD3D11_BLEND_DESC* blendDesc = new CD3D11_BLEND_DESC();
		blendDesc->AlphaToCoverageEnable = false;
		blendDesc->IndependentBlendEnable = false;

		D3D11_RENDER_TARGET_BLEND_DESC RTBlendDesc = D3D11_RENDER_TARGET_BLEND_DESC();
		RTBlendDesc.BlendEnable = true;
		RTBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
		RTBlendDesc.SrcBlend = D3D11_BLEND_ONE;
		RTBlendDesc.DestBlend = D3D11_BLEND_ONE;
		RTBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		RTBlendDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
		RTBlendDesc.DestBlendAlpha = D3D11_BLEND_ONE;
		RTBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		blendDesc->RenderTarget[0] = RTBlendDesc;


		HRESULT hr = Device->CreateBlendState(blendDesc, &mParticleBlend);
		if (FAILED(hr)) {
			DXUT_ERR_MSGBOX(L"failed to create particle blend state", hr);
			return false;
		}

	}

	// create vertex, index and constant Buffers

	// create swap buffers (2, size of max photons
	D3D11_BUFFER_DESC bd = D3D11_BUFFER_DESC();
	ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = mMaxParticleCount * sizeof(PARTICLEDATA);//MAXPARTICLES setting of ParticleSystem!
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA particleInitData; //dead particles, shader will create new particles as it sees fit.
	ZeroMemory( &particleInitData, sizeof(D3D11_SUBRESOURCE_DATA));

	PARTICLEDATA* deadParticles = new PARTICLEDATA[mMaxParticleCount];
	GimmeParticles(deadParticles);
	particleInitData.pSysMem = deadParticles;
	//particleInitData.SysMemPitch = sizeof(PARTICLEDATA)*mMaxParticleCount; // should only have meaning for textures


	HRESULT hr = Device->CreateBuffer(&bd, &particleInitData, &mParticleBufferDraw);
	if (FAILED(hr)) {
		DXUT_ERR_MSGBOX(L"failed to create first Particle swap Buffer", hr);
		return false;
	}

	hr = Device->CreateBuffer(&bd, &particleInitData, &mParticleBufferStream);
	if (FAILED(hr)) {
		DXUT_ERR_MSGBOX(L"failed to create second Particle swap Buffer", hr);
		return false;
	}

	// create constant buffers

	// load particle texture
	hr = D3DX11CreateShaderResourceViewFromFile(Device, L"Media\\Particle2.png", NULL, NULL, &mParticleTexture, NULL);
	if (FAILED(hr)) {
		DXUT_ERR_MSGBOX(L"failed to create Particle Texture Resource View", hr);
		return false;
	}

	// load noise texture
	hr = D3DX11CreateShaderResourceViewFromFile(Device, L"Media\\noise.png", NULL, NULL, &mNoiseTexture, NULL);
	if (FAILED(hr)) {
		DXUT_ERR_MSGBOX(L"failed to create Noise Texture Resource View", hr);
		return false;
	}

	// Constant Buffers
	mSystemCB->D3DCreateDevice(Device,BackBufferSurfaceDesc);
	mFrameCB->D3DCreateDevice(Device,BackBufferSurfaceDesc);

	return true;
}

bool ParticleSystem::D3DCreateSwapChain( ID3D11Device* Device, IDXGISwapChain* SwapChain, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc )
{

	mFrameCB->Data.delta = 0;
	mFrameCB->Data.CenterGravity = 0;

	mSystemCB->Data.colorStart			= D3DXVECTOR4(0.3f,0.6f,0.8f,1);
	mSystemCB->Data.colorEnd            = D3DXVECTOR4(0,0,0,1);
	mSystemCB->Data.colorDeviation      = D3DXVECTOR4(0.1,0.1,0.2,1);
	mSystemCB->Data.positionStart       = D3DXVECTOR4(0,-1,0,0);
	mSystemCB->Data.positionDeviation	= D3DXVECTOR4(100,0,100,0);
	mSystemCB->Data.directionStart		= D3DXVECTOR4(1, 0, 0, 1);
	mSystemCB->Data.directionDeviation	= D3DXVECTOR4(0, 10, 0, 1);
	mSystemCB->Data.spawnTime			= 2.0f;
	mSystemCB->Data.spawnDeviation		= 1.0f;
	mSystemCB->Data.durationTime		= 30.0f;
	mSystemCB->Data.durationDeviation	= 5.0f;
	mSystemCB->Data.speedStart			= 5.0f;
	mSystemCB->Data.speedDeviation		= 10.0f;
	mSystemCB->Data.rotationStart		= 0.0f;
	mSystemCB->Data.rotationDeviation	= 0.0f;
	mSystemCB->Data.sizeStart			= 0.2f;
	mSystemCB->Data.sizeDeviation		= 0.1f;
	mSystemChange = true;

	//immCon->Release();
	return true;
}

void ParticleSystem::Update(float elapsedTime)
{
	// update shaders/particle system with new time and changed values.
	// access constant buffer for: delta and other things that change every frame
	mFrameCB->Data.delta = elapsedTime;

}

void ParticleSystem::Render(ID3D11DeviceContext* ImmediateContext, ID3D11RenderTargetView* particleRTV, ID3D11Buffer* cameraBuffer)
{
	//keep frame constant buffer up to date
	mFrameCB->UpdateBuffer(ImmediateContext);

	if (mSystemChange) // only update when changed, to save bandwidth, should only be updated once per sys.
	{
		mSystemCB->UpdateBuffer(ImmediateContext);
		mSystemChange = false;
	}

	// simulate particles
	ImmediateContext->IASetInputLayout(mParticleLayout);
	UINT stride = sizeof(PARTICLEDATA);
	UINT offset = 0;
	ImmediateContext->IASetVertexBuffers(0, 1, &mParticleBufferDraw, &stride, &offset );
	ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	ImmediateContext->VSSetShader(mParticleSystemVS->GetShader(), NULL, 0);
	ImmediateContext->GSSetShader(mParticleSystemGS, NULL, 0);

	//ImmediateContext->PSSetShaderResources(9, 1, &mNoiseTexture);

	// set Frame Buffer
	ID3D11Buffer* tempBuffer = mFrameCB->GetBuffer();

	ImmediateContext->VSSetConstantBuffers(3, 1, &tempBuffer);
	ImmediateContext->GSSetConstantBuffers(3, 1, &tempBuffer);
	ImmediateContext->PSSetConstantBuffers(3, 1, &tempBuffer);

	//Set System Data Buffer
	tempBuffer = mSystemCB->GetBuffer();

	ImmediateContext->VSSetConstantBuffers(2, 1, &tempBuffer);
	ImmediateContext->GSSetConstantBuffers(2, 1, &tempBuffer);
	ImmediateContext->PSSetConstantBuffers(2, 1, &tempBuffer);
	
	ImmediateContext->PSSetSamplers(0, 1, &mDiffuseSampler);
	ImmediateContext->GSSetSamplers(0, 1, &mDiffuseSampler);

	// SET stream output targets
	ImmediateContext->SOSetTargets(1, &mParticleBufferStream, &offset);

	// render
	// first frame: we haven't streamed out yet.
	if (mInitParticle)
	{
		ImmediateContext->Draw(mMaxParticleCount,0);
		mInitParticle = false;
	}
	else
	{
		// draw all of them for now.
		// TODO: manage particle creation on GPU
		ImmediateContext->Draw(mMaxParticleCount,0); //DrawAuto();
	}
	
	
	//reset render targets and states.
	ID3D11Buffer* swapTemp = NULL;
	ImmediateContext->SOSetTargets(1, &swapTemp, 0);


	// Swap particle buffers
	swapTemp = mParticleBufferDraw;
	mParticleBufferDraw = mParticleBufferStream;
	mParticleBufferStream = swapTemp;

	//SAFE_RELEASE(swapTemp);

	// sync constant buffer
	ImmediateContext->VSSetConstantBuffers(0, 1, &cameraBuffer);
	ImmediateContext->GSSetConstantBuffers(0, 1, &cameraBuffer);
	ImmediateContext->PSSetConstantBuffers(0, 1, &cameraBuffer);

	// set buffers and shaders for rendering
	ImmediateContext->IASetVertexBuffers(0, 1, &mParticleBufferDraw, &stride, &offset);
	ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	ImmediateContext->VSSetShader(mRenderParticlesVS->GetShader(), NULL, 0);
	ImmediateContext->GSSetShader(mRenderParticlesGS->GetShader(), NULL, 0);
	ImmediateContext->PSSetShader(mRenderParticlesPS->GetShader(), NULL, 0);

	//tempBuffer = mFrameCB->GetBuffer();

	//ImmediateContext->VSSetConstantBuffers(2, 1, &tempBuffer);
	//ImmediateContext->GSSetConstantBuffers(2, 1, &tempBuffer);
	//ImmediateContext->PSSetConstantBuffers(2, 1, &tempBuffer);

	////Set System Data Buffer
	//tempBuffer = mSystemCB->GetBuffer();

	//ImmediateContext->VSSetConstantBuffers(3, 1, &tempBuffer);
	//ImmediateContext->GSSetConstantBuffers(3, 1, &tempBuffer);
	//ImmediateContext->PSSetConstantBuffers(3, 1, &tempBuffer);

	ImmediateContext->PSSetSamplers(0, 1, &mDiffuseSampler);
	ImmediateContext->GSSetSamplers(0, 1, &mDiffuseSampler);

	//set render target and stats.
	float blendFactor[] = {1,1,1,1}; 
	ImmediateContext->OMSetRenderTargets(1, &particleRTV, NULL);//DXUTGetD3D11DepthStencilView());
	ImmediateContext->OMSetBlendState(mParticleBlend, blendFactor, 0xffffffff );
	ImmediateContext->OMSetDepthStencilState(mDepthState, 0);
	//ImmediateContext->RSSetState(mRasterizerState);

	ImmediateContext->PSSetShaderResources(4, 1, &mParticleTexture);
	ImmediateContext->PSSetSamplers(0, 1, &mDiffuseSampler);
	// render particles. (to screen)

	ImmediateContext->DrawAuto();
	//ImmediateContext->DrawAuto();

	// clean up
	ImmediateContext->OMSetRenderTargets(0, 0, 0);
	ImmediateContext->OMSetBlendState(NULL, NULL, 0xffffffff);
	ImmediateContext->OMSetDepthStencilState(NULL, 0);
	ImmediateContext->RSSetState(NULL);
}

void ParticleSystem::D3DReleaseDevice()
{
	mFrameCB->D3DReleaseDevice();
	mSystemCB->D3DReleaseDevice();
	SAFE_DELETE(mRenderParticlesVS);
	SAFE_DELETE(mRenderParticlesGS);
	SAFE_DELETE(mRenderParticlesPS);
	
	SAFE_RELEASE(mParticleBlend);
	SAFE_RELEASE(mParticleBufferDraw);
	SAFE_RELEASE(mParticleBufferStream);
	SAFE_RELEASE(mParticleLayout);
	//SAFE_DELETE(mParticleSystemVS);
	//SAFE_DELETE(mParticleSystemGS);
	SAFE_RELEASE(mDiffuseSampler);
	SAFE_RELEASE(mDepthState);
	SAFE_RELEASE(mNoiseTexture);
	SAFE_RELEASE(mRasterizerState);

}

void ParticleSystem::D3DReleaseSwapChain()
{

}

void ParticleSystem::SetSystemData( PARTICLESYSTEMDATA* newSysData )
{
	mSystemCB->Data = *newSysData;
	mSystemChange = true;
}

//fills up given with mMaxParticleCount particles
void ParticleSystem::GimmeParticles(PARTICLEDATA* fillerUp)
{
	// a little random engine (mersenne twister) 
	std::random_device rd;
	std::mt19937 mRandomEngine(rd());
	std::normal_distribution<float> mRandomDist(0, 1);

	ZeroMemory(fillerUp, sizeof(PARTICLEDATA) * mMaxParticleCount);
	for (UINT i = 0; i < mMaxParticleCount; i++)
	{
		fillerUp[i].position = D3DXVECTOR4(	0,0,0,1) ;
		fillerUp[i].initRandom = D3DXVECTOR4(mRandomDist(mRandomEngine), mRandomDist(mRandomEngine), mRandomDist(mRandomEngine), mRandomDist(mRandomEngine));
		fillerUp[i].duration = 0.0f; //zero duration == dead
		fillerUp[i].color = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 1.0f);
		fillerUp[i].direction = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		fillerUp[i].speed = 0.0f;
		fillerUp[i].rotation = 0.0f;
		fillerUp[i].size = 1.0f;
		fillerUp[i].flags = 0; // no flags set, all false
	}
}

void ParticleSystem::ChangeGravity( int gravity )
{
	if(gravity == 0)
	{
		mFrameCB->Data.CenterGravity = 0.0f;
		return;
	}

	mFrameCB->Data.CenterGravity = min(max(mFrameCB->Data.CenterGravity + float(gravity)/10.0, -2.0), 2.0);
	//mFrameCB->Data.CenterGravity += gravity;
}

