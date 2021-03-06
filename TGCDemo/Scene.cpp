#include "DXUT.h"
#include "Scene.h"
#include "BaseGeometry.h"
#include "BaseMaterial.h"
#include "ParticleSystem.h"


Scene::Scene(MainApp* givenApp) :
parentApp(givenApp)
{
		mGeometries.clear();
		mMaterials.clear();
		mGeometryInstances.clear();
		mViewerCamera = new CFirstPersonCamera();
		cameraCBuffer = new ConstantBuffer<CameraConstants>();
		mLineParticles = new ParticleSystem();
}



Scene::~Scene(void)
{
	mGeometries.clear();
	mMaterials.clear();
	mGeometryInstances.clear();
	SAFE_DELETE(mViewerCamera);
	SAFE_DELETE(mLineParticles);

}

MainApp* Scene::GetApp()
{
	return parentApp;
}

void Scene::AddGeometry( BaseGeometry* Geometry )
{
	mGeometries.push_back(Geometry);
}

void Scene::AddMaterial( BaseMaterial* Material )
{
	mMaterials.push_back(Material);
}

void Scene::CreateGeometryInstance( BaseGeometry* Geometry, BaseMaterial* Material )
{
	GeometryInstance* inst = new GeometryInstance(Geometry, Material, this);
	mGeometryInstances.push_back(inst);
}

void Scene::SetCamera(CFirstPersonCamera* givenCamera)
{
	mViewerCamera = givenCamera;
}

bool Scene::D3DCreateDevice(ID3D11Device* Device, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc)
{
	for (std::vector<BaseGeometry*>::iterator it = mGeometries.begin(); it != mGeometries.end(); ++it)
		if (!(*it)->D3DCreateDevice(Device, BackBufferSurfaceDesc))
			return false;
	for (std::vector<BaseMaterial*>::iterator it = mMaterials.begin(); it != mMaterials.end(); ++it)
		if (!(*it)->D3DCreateDevice(Device, BackBufferSurfaceDesc))
			return false;
	
	//load shader
	craftMyGBufferVS = new VertexShader(Device, L"Shaders\\CraftMyGBuffer.hlsl", "CraftMyGBufferVS");
	craftMyGBufferPS = new PixelShader(Device, L"Shaders\\CraftMyGBuffer.hlsl", "CraftMyGBufferPS");

	// Create  mesh input layout
	{
		ID3D10Blob *bytecode = craftMyGBufferVS->GetByteCode();

		const D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{"position",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"normal",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"texCoord",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};

		Device->CreateInputLayout( 
			layout, ARRAYSIZE(layout), 
			bytecode->GetBufferPointer(),
			bytecode->GetBufferSize(), 
			&mMeshVertexLayout);
	}

	{
		CD3D11_RASTERIZER_DESC desc(D3D11_DEFAULT);
		desc.CullMode = D3D11_CULL_BACK;
		desc.FrontCounterClockwise = FALSE;
		Device->CreateRasterizerState(&desc, &mRasterizerState);
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

	// Create sampler state
	{
		CD3D11_SAMPLER_DESC desc(D3D11_DEFAULT);
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		desc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
		Device->CreateSamplerState(&desc, &mMirrorSampler);
	}

	{
		CD3D11_DEPTH_STENCIL_DESC desc(D3D11_DEFAULT);
		desc.StencilEnable = false;
		desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		desc.DepthEnable = true;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		Device->CreateDepthStencilState(&desc, &mDepthState);
	}

	// Create geometry phase blend state
	{
		CD3D11_BLEND_DESC desc(D3D11_DEFAULT);
		Device->CreateBlendState(&desc, &mGeometryBlendState);
	}

	// Create deferred shade (direct light) blend state
	{
		CD3D11_BLEND_DESC* blendDesc = new CD3D11_BLEND_DESC();
		blendDesc->AlphaToCoverageEnable = false;
		blendDesc->IndependentBlendEnable = false;

		D3D11_RENDER_TARGET_BLEND_DESC RTBlendDesc = D3D11_RENDER_TARGET_BLEND_DESC();
		RTBlendDesc.BlendEnable = true;
		RTBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
		RTBlendDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
		RTBlendDesc.DestBlend = D3D11_BLEND_ONE;
		RTBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		RTBlendDesc.SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
		RTBlendDesc.DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
		RTBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		blendDesc->RenderTarget[0] = RTBlendDesc;


		HRESULT hr = Device->CreateBlendState(blendDesc, &mDeferredShadeBlend);
		if (FAILED(hr)) 
			DXUT_ERR_MSGBOX(L"failed to create deferred shade blend state", hr);
		
	}

	cameraCBuffer->D3DCreateDevice(Device, BackBufferSurfaceDesc);

	// ### deferred shading direct light shaders ###
	DeferredShadeVS = new VertexShader(Device, L"Shaders\\DeferredShade.hlsl", "DeferredShadeVS");
	//DeferredShadePS = new PixelShader(Device, L"Shaders\\DeferredShade.hlsl", "DeferredShadePS");
	DeferredSunShadePS = new PixelShader(Device, L"Shaders\\DeferredShade.hlsl", "DeferredSunShadePS");

	//D3D11_SAMPLER_DESC shadowDesc = D3D11_SAMPLER_DESC();
	//ZeroMemory(&shadowDesc, sizeof(D3D11_SAMPLER_DESC));
	//shadowDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	//shadowDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	//shadowDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	//shadowDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	//shadowDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	//shadowDesc.BorderColor[0] = shadowDesc.BorderColor[1] =shadowDesc.BorderColor[2] =shadowDesc.BorderColor[3] = 1;
	//Device->CreateSamplerState(&shadowDesc, &shadowSampler);

	// ### full screen quad for direct light render ###
	PositionTexCoordVertex vertices[] =
	{
		{ D3DXVECTOR3( -1.0f, 1.0f, 0.0f),	D3DXVECTOR2(0.0f, 0.0f) }, //top left
		{ D3DXVECTOR3(	1.0f, 1.0f, 0.0f),	D3DXVECTOR2(1.0f, 0.0f) }, //top right
		{ D3DXVECTOR3( -1.0f, -1.0f, 0.0f), D3DXVECTOR2(0.0f, 1.0f) }, //bottom left
		{ D3DXVECTOR3( 1.0f, -1.0f, 0.0f),	D3DXVECTOR2(1.0f, 1.0f) }, //bottom right
	};

	D3D11_BUFFER_DESC screenQuadDesc;
	ZeroMemory( &screenQuadDesc, sizeof(D3D11_BUFFER_DESC) );
	screenQuadDesc.Usage = D3D11_USAGE_DEFAULT;
	screenQuadDesc.ByteWidth = sizeof( PositionTexCoordVertex ) * ARRAYSIZE(vertices);
	screenQuadDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	screenQuadDesc.CPUAccessFlags = 0;
	screenQuadDesc.MiscFlags = 0;
	screenQuadDesc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory( &InitData, sizeof(InitData) );
	InitData.pSysMem = vertices;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;
	Device->CreateBuffer( &screenQuadDesc, &InitData, &screenQuadVertices);

	{
		// ## Light constant buffer ##
		D3DXMATRIX identityMatrix;
		D3DXMatrixIdentity(&identityMatrix);
		LightConstants lightConstData;
//		lightConstData.lightViewProj = identityMatrix;
		lightConstData.lightPositionAngle = D3DXVECTOR4(0,0,0, D3DX_PI/4);
		lightConstData.lightPower = D3DXVECTOR4(1,1,1,1);
		lightConstData.lightDirectionDistance = D3DXVECTOR4(0,1,0,100.0f);


		D3D11_BUFFER_DESC constantbd = CD3D11_BUFFER_DESC();
		constantbd.Usage = D3D11_USAGE_DEFAULT;
		constantbd.ByteWidth = sizeof( LightConstants );
		constantbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantbd.CPUAccessFlags = 0;
		constantbd.MiscFlags = 0;
		constantbd.StructureByteStride = 0;

		// Fill in the subresource data.
		D3D11_SUBRESOURCE_DATA ConstInitData;
		ConstInitData.pSysMem = &lightConstData;
		ConstInitData.SysMemPitch = 0;
		ConstInitData.SysMemSlicePitch = 0;

		Device->CreateBuffer(&constantbd, &ConstInitData, &lightCBuffer);
	}

	// ### tone mapping shader
	toneMappingVS = new VertexShader(Device, L"Shaders\\ToneMapping.hlsl", "ToneMappingVS");
	toneMappingPS = new PixelShader(Device, L"Shaders\\ToneMapping.hlsl", "ToneMappingPS");

	// Particle System
	mLineParticles->D3DCreateDevice(Device,BackBufferSurfaceDesc);

	return true;
}

void Scene::D3DReleaseDevice()
{
	for (std::vector<BaseGeometry*>::iterator it = mGeometries.begin(); it != mGeometries.end(); ++it)
		(*it)->D3DReleaseDevice();
	for (std::vector<BaseMaterial*>::iterator it = mMaterials.begin(); it != mMaterials.end(); ++it)
		(*it)->D3DReleaseDevice();
	for (std::vector<ID3D11Texture2D*>::iterator it = mGBuffer.begin(); it != mGBuffer.end(); ++it)
	{
		SAFE_RELEASE((*it));
	}
	for (std::vector<ID3D11RenderTargetView*>::iterator it = mGBufferRTV.begin(); it != mGBufferRTV.end(); ++it)
	{
		SAFE_RELEASE((*it));
	}
	for (std::vector<ID3D11ShaderResourceView*>::iterator it = mGBufferSRV.begin(); it != mGBufferSRV.end(); ++it)
	{
		SAFE_RELEASE((*it));
	}

	mLineParticles->D3DReleaseDevice();

	cameraCBuffer->D3DReleaseDevice();
	SAFE_DELETE(cameraCBuffer);
	SAFE_DELETE(mViewerCamera);

	SAFE_RELEASE(mMeshVertexLayout);
	SAFE_DELETE(craftMyGBufferVS);
	SAFE_DELETE(craftMyGBufferPS);

	SAFE_RELEASE(mRasterizerState);
	SAFE_RELEASE(mDiffuseSampler);
	SAFE_RELEASE(mMirrorSampler);
	SAFE_RELEASE(mDepthState);
	SAFE_RELEASE(mGeometryBlendState);
	SAFE_RELEASE(mDeferredShadeBlend);
	SAFE_RELEASE(mGeometryBlendState);
	SAFE_RELEASE(mDeferredShadeBlend);
	//SAFE_DELETE(DeferredShadePS);
	SAFE_DELETE(DeferredShadeVS);
	SAFE_DELETE(DeferredSunShadePS);
	//SAFE_RELEASE(shadowSampler);

	
	SAFE_RELEASE(splattingTexture);
	SAFE_RELEASE(linesTexture);
	SAFE_RELEASE(splattingRTV);
	SAFE_RELEASE(splattingSRV);
	SAFE_DELETE(toneMappingPS);
	SAFE_DELETE(toneMappingVS);
	SAFE_RELEASE(screenQuadVertices);
	SAFE_RELEASE(lightCBuffer);
	
}

// ================================================================================
bool Scene::D3DCreateSwapChain(ID3D11Device* Device, IDXGISwapChain* SwapChain, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc)
{
	if (mViewerCamera)
	{
		mViewerCamera->SetProjParams(D3DX_PI / 4, (float)BackBufferSurfaceDesc->Width / (float)BackBufferSurfaceDesc->Height, CAMERA_NEAR, CAMERA_FAR);
		
		D3DXMATRIX identityMatrix;
		D3DXMatrixIdentity(&identityMatrix);
		
		cameraCBuffer->Data.World	= identityMatrix;
		cameraCBuffer->Data.refractiveIndexETA = 1.0f;
		cameraCBuffer->Data.View	= *mViewerCamera->GetViewMatrix();
		cameraCBuffer->Data.cameraPosition = *mViewerCamera->GetEyePt();
		cameraCBuffer->Data.WorldViewProjection = identityMatrix * cameraCBuffer->Data.View * (*mViewerCamera->GetProjMatrix());

	}
	mGBufferWidth = BackBufferSurfaceDesc->Width;
	mGBufferHeight = BackBufferSurfaceDesc->Height;

	// Create/recreate any textures related to screen size
	mGBuffer.resize(0);
	mGBufferRTV.resize(0);
	mGBufferSRV.resize(0);

	////light GBuffer
	//mLightGBufferWidth = mGBufferWidth; //BOUNCEMAP_WIDTH;
	//mLightGBufferHeight = mGBufferHeight; //BOUNCEMAP_HEIGHT;
	//mLightGBuffer.resize(0);
	//mLightGBufferRTV.resize(0);
	//mLightGBufferSRV.resize(0);


	// GBuffer viewercamera
	{
		ID3D11Texture2D *tempTexture;
		CD3D11_TEXTURE2D_DESC texDesc = CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R16G16B16A16_FLOAT, mGBufferWidth, mGBufferHeight, 1, 1, D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_DEFAULT);
		Device->CreateTexture2D(&texDesc, 0, &tempTexture);

		//diffuse
		mGBuffer.push_back(tempTexture);

		Device->CreateTexture2D(&texDesc, 0, &tempTexture);
		//specular
		mGBuffer.push_back(tempTexture);

		Device->CreateTexture2D(&texDesc, 0, &tempTexture);
		//transmittance
		mGBuffer.push_back(tempTexture);

		Device->CreateTexture2D(&texDesc, 0, &tempTexture);
		//normal
		mGBuffer.push_back(tempTexture);

		texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		Device->CreateTexture2D(&texDesc, 0, &tempTexture);
		//normal
		mGBuffer.push_back(tempTexture);
	};

	// ### Set up GBuffer resource list
	mGBufferRTV.resize(mGBuffer.size(), 0);
	mGBufferSRV.resize(mGBuffer.size(), 0);

	D3D11_RENDER_TARGET_VIEW_DESC* rtViewDesc = &D3D11_RENDER_TARGET_VIEW_DESC();
	D3D11_SHADER_RESOURCE_VIEW_DESC* srViewDesc = &D3D11_SHADER_RESOURCE_VIEW_DESC();
	CD3D11_TEXTURE2D_DESC* tempTexDesc = &CD3D11_TEXTURE2D_DESC();
	rtViewDesc->ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	srViewDesc->ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srViewDesc->Texture2D.MipLevels = 1;
	srViewDesc->Texture2D.MostDetailedMip = 0;

	for (std::size_t i = 0; i < mGBuffer.size(); ++i) 
	{
		mGBuffer[i]->GetDesc(tempTexDesc);

		rtViewDesc->Format = tempTexDesc->Format;
		srViewDesc->Format = tempTexDesc->Format;
		Device->CreateRenderTargetView(mGBuffer[i], rtViewDesc, &mGBufferRTV[i]);
		Device->CreateShaderResourceView(mGBuffer[i], srViewDesc,&mGBufferSRV[i] );
	}

	////GBuffer light
	//{
	//	ID3D11Texture2D *tempTexture;

	//	CD3D11_TEXTURE2D_DESC texDesc = CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R16G16B16A16_FLOAT, mLightGBufferWidth, mLightGBufferHeight, 1, 1, D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_DEFAULT);
	//	d3dDevice->CreateTexture2D(&texDesc, 0, &tempTexture);

	//	//diffuse
	//	mLightGBuffer.push_back(tempTexture);

	//	//texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	//	d3dDevice->CreateTexture2D(&texDesc, 0, &tempTexture);
	//	//specular
	//	mLightGBuffer.push_back(tempTexture);

	//	d3dDevice->CreateTexture2D(&texDesc, 0, &tempTexture);
	//	//transmittance
	//	mLightGBuffer.push_back(tempTexture);

	//	d3dDevice->CreateTexture2D(&texDesc, 0, &tempTexture);
	//	//normal
	//	mLightGBuffer.push_back(tempTexture);

	//	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	//	d3dDevice->CreateTexture2D(&texDesc, 0, &tempTexture);
	//	//normal
	//	mLightGBuffer.push_back(tempTexture);
	//};

	//// ### Set up GBuffer resource list
	//mLightGBufferRTV.resize(mLightGBuffer.size(), 0);
	//mLightGBufferSRV.resize(mLightGBuffer.size(), 0);
	//
	//for (std::size_t i = 0; i < mLightGBuffer.size(); ++i) {
	//	mLightGBuffer[i]->GetDesc(tempTexDesc);

	//	rtViewDesc->Format = tempTexDesc->Format;
	//	srViewDesc->Format = tempTexDesc->Format;
	//	d3dDevice->CreateRenderTargetView(mLightGBuffer[i], rtViewDesc, &mLightGBufferRTV[i]);
	//	d3dDevice->CreateShaderResourceView(mLightGBuffer[i], srViewDesc,&mLightGBufferSRV[i] );
	//}

	//{
	//	ID3D11Texture2D *tempTexture;
	//	tempTexDesc = new CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_D32_FLOAT, mLightGBufferWidth, mLightGBufferHeight);
	//	tempTexDesc->MipLevels = 1;
	//	tempTexDesc->BindFlags = D3D11_BIND_DEPTH_STENCIL;
	//	HRESULT hr = d3dDevice->CreateTexture2D(tempTexDesc, NULL, &tempTexture);
	//	if(FAILED(hr))
	//		DXUT_ERR_MSGBOX(L" DEPTH texture not created", hr);

	//	// light depth View
	//	CD3D11_DEPTH_STENCIL_VIEW_DESC depthDesc = CD3D11_DEPTH_STENCIL_VIEW_DESC(D3D11_DSV_DIMENSION_TEXTURE2D);
	//	depthDesc.Format = DXGI_FORMAT_D32_FLOAT;

	//	hr =d3dDevice->CreateDepthStencilView(tempTexture, &depthDesc, &lightDepthView);

	//	if(FAILED(hr))
	//		DXUT_ERR_MSGBOX(L"DEPTH View not created", hr);

	//	SAFE_RELEASE(tempTexture);
	//}

	// load lines texture
	HRESULT hr = D3DX11CreateShaderResourceViewFromFile(Device, L"Media\\lines.png", NULL, NULL, &linesTexture, NULL);
	if (FAILED(hr)) {
		DXUT_ERR_MSGBOX(L"failed to create Particle Texture Resource View", hr);
		return false;
	}

	// splatting Render Target
	{
		CD3D11_TEXTURE2D_DESC texDesc = CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R32G32B32A32_FLOAT, BackBufferSurfaceDesc->Width, BackBufferSurfaceDesc->Height, 1, 1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_DEFAULT);
		Device->CreateTexture2D(&texDesc, 0, &splattingTexture);

		rtViewDesc->Format = texDesc.Format;
		srViewDesc->Format = texDesc.Format;
		Device->CreateRenderTargetView(splattingTexture, rtViewDesc, &splattingRTV);
		Device->CreateShaderResourceView(splattingTexture, srViewDesc, &splattingSRV);
	}

	mLineParticles->D3DCreateSwapChain(Device, SwapChain, BackBufferSurfaceDesc);

	return true;
}

// ================================================================================
void Scene::D3DReleaseSwapChain()
{
	for (std::size_t i = 0; i < mGBuffer.size(); ++i) {
		SAFE_RELEASE(mGBuffer[i]);
		SAFE_RELEASE(mGBufferRTV[i]);
		SAFE_RELEASE(mGBufferSRV[i]);
	}
	mGBuffer.clear();
	mGBufferRTV.clear();
	mGBufferSRV.clear();

	mLineParticles->D3DReleaseSwapChain();

	//for (std::size_t i = 0; i < mLightGBuffer.size(); ++i) {
	//	SAFE_RELEASE(mLightGBuffer[i]);
	//	SAFE_RELEASE(mLightGBufferRTV[i]);
	//	SAFE_RELEASE(mLightGBufferSRV[i]);
	//}
	//mLightGBuffer.clear();
	//mLightGBufferRTV.clear();
	//mLightGBufferSRV.clear();

	//SAFE_RELEASE(lightDepthView);
}
void Scene::Render(ID3D11DeviceContext* ImmediateContext, const D3D11_VIEWPORT* mainViewPort, float fElapsedTime)
{
	//TODO: also do for one directional light
	//clear backbuffer
	float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
	ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
	ImmediateContext->ClearRenderTargetView(pRTV, ClearColor );
	ImmediateContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	ImmediateContext->ClearRenderTargetView(splattingRTV, ClearColor );

	// render gbuffer for viewer camera rendering
	if (!mViewerCamera) return;

	cameraCBuffer->UpdateBuffer(ImmediateContext);
	ID3D11Buffer* tempCameraCBuffer = cameraCBuffer->GetBuffer();
	D3DXVECTOR3 camPos = *mViewerCamera->GetEyePt();
	D3DXVECTOR3 camLook = *mViewerCamera->GetLookAtPt();

	ImmediateContext->VSSetConstantBuffers(0, 1, &tempCameraCBuffer);
	ImmediateContext->GSSetConstantBuffers(0, 1, &tempCameraCBuffer);
	ImmediateContext->PSSetConstantBuffers(0, 1, &tempCameraCBuffer);
	ImmediateContext->PSSetSamplers(1, 1, &mMirrorSampler);
	ImmediateContext->GSSetSamplers(1, 1, &mMirrorSampler);

	RenderGBuffer(ImmediateContext, mainViewPort);

	// ### render all the lights

	// SUNLIGHT
	// start rendering all the lights
	//if(mSunLight)
	{
		////move sun with camera
		////D3DXVECTOR3 sunPos = D3DXVECTOR3(viewerCamera->GetEyePt()->x, 800.0f, viewerCamera->GetEyePt()->z);
		////mSunLight->SetPosition(sunPos);
		////mSunLight->ResetCamera();
		//mSunLight->GetCamera()->GetCameraCBuffer()->UpdateBuffer(ImmediateContext);
		//ID3D11Buffer* tempCameraCBuffer = mSunLight->GetCamera()->GetCameraCBuffer()->GetBuffer();

		//ImmediateContext->VSSetConstantBuffers(1, 1, &tempCameraCBuffer);
		//ImmediateContext->GSSetConstantBuffers(1, 1, &tempCameraCBuffer);
		//ImmediateContext->PSSetConstantBuffers(1, 1, &tempCameraCBuffer);

		//// render GBuffer
		////RenderLightGBuffer(ImmediateContext, mainViewPort, mSunLight->GetCamera());

		//set light constant data
		LightConstants lightConstData;
		//lightConstData.lightViewProj = (*mSunLight->GetCamera()->GetViewMatrix()) * (*mSunLight->GetCamera()->GetProjMatrix());
		lightConstData.lightPositionAngle = D3DXVECTOR4(*mSunPosition, 0.0f);
		lightConstData.lightPower = *mSunPower;
		lightConstData.lightDirectionDistance = D3DXVECTOR4(*mSunDirection, 1.0f);
		RenderForward(ImmediateContext, mainViewPort, &lightConstData);

	}

	ImmediateContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// put together particle system and gbuffer stuff //get data from splattingRTV or Gbuffer RTVs, bind as SRVs.
	mLineParticles->Render(ImmediateContext, splattingRTV, tempCameraCBuffer);


	// call tonemapping
	ImmediateContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	ImmediateContext->RSSetState(mRasterizerState);
	ToneMapping(ImmediateContext, splattingSRV, screenQuadVertices);

	//parentApp->RenderTextureToScreen(ImmediateContext, pRTV, new D3DXVECTOR4(0.1f,-0.1f,0.6f,- (0.6f)), mGBufferSRV[0]); //specular
	//parentApp->RenderTextureToScreen(ImmediateContext, pRTV, new D3DXVECTOR4(0.1f,-0.71f,0.6f,- (0.6f)), mGBufferSRV[1]); //diffuse
	//parentApp->RenderTextureToScreen(ImmediateContext, pRTV, new D3DXVECTOR4(0.1f,-1.31f,0.6f,- (0.6f)), splattingSRV); //transmissive
	//parentApp->RenderTextureToScreen(ImmediateContext, pRTV, new D3DXVECTOR4(0.7f,-0.1f,0.6f,- (0.6f)), mGBufferSRV[3]); //normal
	//parentApp->RenderTextureToScreen(ImmediateContext, pRTV, new D3DXVECTOR4(0.7f,-0.71f,0.6f,- (0.6f)), mGBufferSRV[4]); //position


	//SAFE_RELEASE(tempCameraCBuffer);
}

void Scene::HandleMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	mViewerCamera->HandleMessages(hWnd, uMsg, wParam, lParam);
}

void Scene::Move( float elapsedTime )
{
	mViewerCamera->FrameMove(elapsedTime);

	D3DXMATRIX identityMatrix;
	D3DXMatrixIdentity(&identityMatrix);

	cameraCBuffer->Data.World	= identityMatrix;
	cameraCBuffer->Data.refractiveIndexETA = 1.0f;
	cameraCBuffer->Data.View	= *mViewerCamera->GetViewMatrix();
	cameraCBuffer->Data.cameraPosition = *mViewerCamera->GetEyePt();
	cameraCBuffer->Data.WorldViewProjection = identityMatrix * cameraCBuffer->Data.View * (*mViewerCamera->GetProjMatrix());

	//update particle system
	mLineParticles->Update(elapsedTime);

	// update geometry (make it update its own pos/ world matrix)

}

void Scene::RenderForward( ID3D11DeviceContext* d3dDeviceContext, const D3D11_VIEWPORT* mainViewPort, LightConstants* lightConstData)
{
	//render information from gBuffer. //put it all together.

	ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();

	//D3DXMatrixInverse(&lightConstData->shadowToScreen, NULL, &lightConstData->lightViewProj);
	//lightConstData->shadowToScreen *= (*viewerCamera->GetViewMatrix());

	d3dDeviceContext->UpdateSubresource(lightCBuffer, 0, NULL, lightConstData, 0, 0);

	d3dDeviceContext->OMSetDepthStencilState(mDepthState, 0);
	d3dDeviceContext->OMSetBlendState(mDeferredShadeBlend, 0, 0xFFFFFFFF);
	d3dDeviceContext->OMSetRenderTargets(1, &splattingRTV, NULL);

	UINT stride = sizeof(PositionTexCoordVertex);
	UINT offset = 0;
	d3dDeviceContext->IASetVertexBuffers(0,1, &screenQuadVertices, &stride, &offset);
	d3dDeviceContext->IASetInputLayout(parentApp->mPositionTexCoordLayout);
	d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// set shaders
	d3dDeviceContext->VSSetShader(DeferredShadeVS->GetShader(), 0, 0);
	d3dDeviceContext->GSSetShader(NULL, 0, 0);
	//if(sun)
		d3dDeviceContext->PSSetShader(DeferredSunShadePS->GetShader(), 0, 0);
	//else
	//	d3dDeviceContext->PSSetShader(DeferredShadePS->GetShader(), 0, 0);

	d3dDeviceContext->RSSetState(mRasterizerState);
	d3dDeviceContext->RSSetViewports(1, mainViewPort);

	// set shader resources
	// light worldposition map (need for shadows)
	//d3dDeviceContext->PSSetShaderResources(3, 1, &mLightGBufferSRV[4]);
	//viewer camera gbuffer
	d3dDeviceContext->PSSetShaderResources(4, static_cast<UINT>(mGBufferSRV.size()), &mGBufferSRV.front());

	// set constant buffers
	// camera is already set in Render function
	d3dDeviceContext->PSSetConstantBuffers(4, 1, &lightCBuffer);

	// draw
	d3dDeviceContext->Draw(4,0);

	d3dDeviceContext->OMSetRenderTargets(0, 0, 0);
	d3dDeviceContext->OMSetBlendState(NULL, NULL, 0xffffffff);

	ID3D11ShaderResourceView *const pSRV[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
	d3dDeviceContext->PSSetShaderResources(3, 6, pSRV);
	//d3dDeviceContext->PSSetSamplers(1, 0, 0);



}

void Scene::RenderGBuffer( ID3D11DeviceContext* d3dDeviceContext, const D3D11_VIEWPORT* givenViewPort)
{

	//float ClearColor[4] = { 0.2f, 0.1f, 0.8f, 1.0f };
	float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	d3dDeviceContext->IASetInputLayout(mMeshVertexLayout);

	ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
	ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
	d3dDeviceContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//d3dDeviceContext->ClearDepthStencilView(mDepthBufferView, D3D11_CLEAR_DEPTH, 1.0f, NULL);
	for (std::size_t i = 0; i < mGBufferRTV.size(); ++i) {
		d3dDeviceContext->ClearRenderTargetView(mGBufferRTV[i], ClearColor);
	}
	d3dDeviceContext->OMSetRenderTargets(static_cast<UINT>(mGBufferRTV.size()), &mGBufferRTV.front(), pDSV);

	d3dDeviceContext->VSSetShader(craftMyGBufferVS->GetShader(), 0, 0);
	d3dDeviceContext->GSSetShader(0, 0, 0);
	d3dDeviceContext->PSSetShader(craftMyGBufferPS->GetShader(), 0, 0);

	d3dDeviceContext->RSSetState(mRasterizerState);
	d3dDeviceContext->RSSetViewports(1, givenViewPort);

	ID3D11ShaderResourceView *const pSRV[4] = {NULL, NULL, NULL, NULL};
	d3dDeviceContext->PSSetShaderResources(0, 4, pSRV);
	d3dDeviceContext->PSSetShaderResources(4, 1, &linesTexture);
	//d3dDeviceContext->PSSetSamplers(0, 1, &mDiffuseSampler);

	d3dDeviceContext->OMSetDepthStencilState(mDepthState, 0);
	d3dDeviceContext->OMSetBlendState(mGeometryBlendState, 0, 0xFFFFFFFF);

	// TODO ### RENDER ###
	for (std::vector<GeometryInstance*>::iterator it = mGeometryInstances.begin(); it != mGeometryInstances.end(); ++it)
	{
		if((*it)->GetGeometry()->IsStatic())
			(*it)->RenderToGBuffer(d3dDeviceContext);
		else
		{
			//save old matrix
			D3DXMATRIX oldWorldMatrix = cameraCBuffer->Data.World;
			D3DXMATRIX oldWVPMatrix = 	cameraCBuffer->Data.WorldViewProjection;

			cameraCBuffer->Data.World = (*it)->GetTransform();
			cameraCBuffer->Data.WorldViewProjection = cameraCBuffer->Data.World * cameraCBuffer->Data.View * (*mViewerCamera->GetProjMatrix());
			cameraCBuffer->UpdateBuffer(d3dDeviceContext);
			ID3D11Buffer* tempCameraCBuffer = cameraCBuffer->GetBuffer();

			d3dDeviceContext->VSSetConstantBuffers(0, 1, &tempCameraCBuffer);
			d3dDeviceContext->GSSetConstantBuffers(0, 1, &tempCameraCBuffer);
			d3dDeviceContext->PSSetConstantBuffers(0, 1, &tempCameraCBuffer);

			(*it)->RenderToGBuffer(d3dDeviceContext);

			// set to old matrix
			cameraCBuffer->Data.World = oldWorldMatrix;
			cameraCBuffer->Data.WorldViewProjection = oldWVPMatrix;
			cameraCBuffer->UpdateBuffer(d3dDeviceContext);
			tempCameraCBuffer = cameraCBuffer->GetBuffer();

			d3dDeviceContext->VSSetConstantBuffers(0, 1, &tempCameraCBuffer);
			d3dDeviceContext->GSSetConstantBuffers(0, 1, &tempCameraCBuffer);
			d3dDeviceContext->PSSetConstantBuffers(0, 1, &tempCameraCBuffer);
		}
	}

	// Cleanup (aka make the runtime happy)
	d3dDeviceContext->OMSetRenderTargets(1, &pRTV, pDSV); 
};

void Scene::SetSun( D3DXVECTOR3 Position, D3DXVECTOR3 Direction, D3DXVECTOR4 Power )
{
	mSunPosition = new D3DXVECTOR3(Position);
	mSunDirection = new D3DXVECTOR3(Direction);
	mSunPower = new D3DXVECTOR4(Power);
}

void Scene::ToneMapping( ID3D11DeviceContext* d3dDeviceContext, ID3D11ShaderResourceView* inputRender, ID3D11Buffer* quadVertices )
{
	d3dDeviceContext->IASetInputLayout(parentApp->mPositionTexCoordLayout);
	UINT stride = sizeof(PositionTexCoordVertex);
	UINT offset = 0;
	d3dDeviceContext->IASetVertexBuffers(0, 1, &quadVertices, &stride, &offset);
	d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	d3dDeviceContext->VSSetShader(toneMappingVS->GetShader(), NULL, 0);
	d3dDeviceContext->GSSetShader(NULL, NULL, 0);
	d3dDeviceContext->PSSetShader(toneMappingPS->GetShader(), NULL, 0);
	d3dDeviceContext->PSSetShaderResources(0, 1, &inputRender);

	ID3D11RenderTargetView* tempRTV[] = {DXUTGetD3D11RenderTargetView()};
	d3dDeviceContext->OMSetRenderTargets(1, tempRTV, NULL);
	d3dDeviceContext->OMSetBlendState(NULL, NULL, 0xffffffff);

	d3dDeviceContext->Draw(4, 0); //quad

	d3dDeviceContext->OMSetRenderTargets(0, 0, 0);
	d3dDeviceContext->OMSetBlendState(NULL, NULL, 0xffffffff);

	ID3D11ShaderResourceView* pSRV[] = {NULL};
	d3dDeviceContext->PSSetShaderResources(0, 1, pSRV);
}

void Scene::ChangeVale( int change )
{
	mLineParticles->ChangeGravity(change);
}
