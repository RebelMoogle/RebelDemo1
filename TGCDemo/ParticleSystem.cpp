#include "DXUT.h"
#include "ParticleSystem.h"


ParticleSystem::ParticleSystem(void)
{
	mMaxParticleCount = 100;
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
			return false;
		}

		if (FAILED(hr)) {
			DXUT_ERR_MSGBOX(L"failed to compile Particle System Geometry Shader", hr);
			return false;
		}
		D3D11_SO_DECLARATION_ENTRY BounceBufferOutputDecl[] =
		{
			// stream, sem name, sem index, start component, component count, output slot
			{ 0, "SV_POSITION", 0, 0, 3, 0},
			{ 0, "DURATION", 0, 0, 1, 0},
		};
		UINT strides[] = { (4 + 1)  * sizeof(float) };

		hr = Device->CreateGeometryShaderWithStreamOutput(	bytecode->GetBufferPointer(), 
			bytecode->GetBufferSize(), 
			BounceBufferOutputDecl, 
			ARRAYSIZE(BounceBufferOutputDecl), 
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

	// create mesh input layout
	{
		ID3D10Blob *bytecode = mParticleSystemVS->GetByteCode();

		const D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{"position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"duration", 0, DXGI_FORMAT_R32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};

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
		desc.DepthClipEnable = TRUE;
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
		desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
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

	PARTICLEDATA deadParticle;
	ZeroMemory(&deadParticle, sizeof(PARTICLEDATA));
	deadParticle.position = D3DXVECTOR3(10,10,0);
	deadParticle.duration = 0.0f; //zero duration == dead

	particleInitData.pSysMem = &deadParticle;
	particleInitData.SysMemPitch = sizeof(PARTICLEDATA);


	HRESULT hr = Device->CreateBuffer(&bd, &particleInitData, &mParticleBufferDraw);
	if (FAILED(hr)) {
		DXUT_ERR_MSGBOX(L"failed to create first Particle swap Buffer", hr);
		return false;
	}

	hr = Device->CreateBuffer(&bd, NULL, &mParticleBufferStream);
	if (FAILED(hr)) {
		DXUT_ERR_MSGBOX(L"failed to create second Particle swap Buffer", hr);
		return false;
	}

	// create constant buffers

	// load particle texture
	hr = D3DX11CreateShaderResourceViewFromFile(Device, L"Media\\Particle1.png", NULL, NULL, &mParticleTexture, NULL);
	if (FAILED(hr)) {
		DXUT_ERR_MSGBOX(L"failed to create Particle Texture Resource View", hr);
		return false;
	}

	return true;
}

bool ParticleSystem::D3DCreateSwapChain( ID3D11Device* Device, IDXGISwapChain* SwapChain, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc )
{


	return true;
}

void ParticleSystem::Update()
{
	// update shaders/particle system with new time and changed values.
	// access constant buffer


}

void ParticleSystem::Render(ID3D11DeviceContext* d3dDeviceContext)
{
	// simulate particles
	d3dDeviceContext->IASetInputLayout(mParticleLayout);
	UINT stride = sizeof(PARTICLEDATA); // create struct for size or something (equal to layout)
	UINT offset = 0;
	d3dDeviceContext->IASetVertexBuffers(0, 1, &mParticleBufferDraw, &stride, &offset );
	d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	d3dDeviceContext->VSSetShader(mParticleSystemVS->GetShader(), NULL, 0);
	d3dDeviceContext->GSSetShader(mParticleSystemGS, NULL, 0);

	//set shader resource (textures, etc)
	d3dDeviceContext->GSSetShaderResources(1, 1, &mParticleTexture);

	// SET stream output targets
	d3dDeviceContext->SOSetTargets(1, &mParticleBufferStream, &offset);

	// render
	// first frame: we haven't streamed out yet.
	if (mInitParticle)
	{
		d3dDeviceContext->Draw(1,0);
		mInitParticle = false;
	}
	else
		d3dDeviceContext->DrawAuto();
	
	
	//reset render targets and states.
	ID3D11Buffer* swapTemp = NULL;
	d3dDeviceContext->SOSetTargets(1, &swapTemp, 0);


	// Swap particle buffers
	swapTemp = mParticleBufferDraw;
	mParticleBufferDraw = mParticleBufferStream;
	mParticleBufferStream = swapTemp;

	SAFE_RELEASE(swapTemp);

	// sync constant buffer

	// set buffers and shaders for rendering
	d3dDeviceContext->IASetVertexBuffers(0, 1, &mParticleBufferDraw, &stride, &offset);
	d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	d3dDeviceContext->VSSetShader(mRenderParticlesVS->GetShader(), NULL, 0);
	d3dDeviceContext->GSSetShader(mRenderParticlesGS->GetShader(), NULL, 0);
	d3dDeviceContext->PSSetShader(mRenderParticlesPS->GetShader(), NULL, 0);

	//set render target and stats.
	float blendFactor[] = {1,1,1,1}; 
	ID3D11RenderTargetView* tempRTV[] = {DXUTGetD3D11RenderTargetView()};
	d3dDeviceContext->OMSetRenderTargets(1, tempRTV, NULL);
	d3dDeviceContext->OMSetBlendState(mParticleBlend, blendFactor, 0xffffffff );
	d3dDeviceContext->OMSetDepthStencilState(mDepthState, 0);
	d3dDeviceContext->RSSetState(mRasterizerState);

	d3dDeviceContext->PSSetShaderResources(1, 1, &mParticleTexture);
	// render particles. (to screen)

	d3dDeviceContext->DrawAuto();

	// clean up
	d3dDeviceContext->OMSetRenderTargets(0, 0, 0);
	d3dDeviceContext->OMSetBlendState(NULL, NULL, 0xffffffff);
	d3dDeviceContext->OMSetDepthStencilState(NULL, 0);
	d3dDeviceContext->RSSetState(NULL);
}

void ParticleSystem::D3DReleaseDevice()
{

}

void ParticleSystem::D3DReleaseSwapChain()
{

}

