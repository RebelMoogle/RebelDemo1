#include "DXUT.h"
#include "MainApp.h"
#include "constants.h"
#include "TestSceneBuilder.h"


MainApp::MainApp(void)
{
	InitScene();
	mTextureToScreenCBuffer = new ConstantBuffer<TextureToScreenConstants>();
}


MainApp::~MainApp(void)
{
	D3DReleaseDevice();
	D3DReleaseSwapChain();

	SAFE_RELEASE(mDefaultDepthState);
	SAFE_RELEASE(mNoDepthRasterizerState);
	SAFE_RELEASE(mWireRasterizerState);
	SAFE_RELEASE(mPositionTexCoordLayout);	
}

bool MainApp::D3DCreateDevice( ID3D11Device* Device, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc )
{
	//render Texture To Screen
	mTextureToScreenVS = new VertexShader(Device, L"Shaders\\TextureToScreen.hlsl", "TextureToScreenVS");
	mTextureToScreenGS = new GeometryShader(Device, L"Shaders\\TextureToScreen.hlsl", "TextureToScreenGS");
	mTextureToScreenPS = new PixelShader(Device, L"Shaders\\TextureToScreen.hlsl", "TextureToScreenPS");

	// Create  texture to screen input layout
	{
		ID3D10Blob *bytecode = mTextureToScreenVS->GetByteCode();


		const D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{"POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};

		Device->CreateInputLayout( 
			layout, ARRAYSIZE(layout), 
			bytecode->GetBufferPointer(),
			bytecode->GetBufferSize(), 
			&mPositionTexCoordLayout);
	}
	
	
	// Create standard rasterizer state
	{
		CD3D11_RASTERIZER_DESC desc(D3D11_DEFAULT);

		desc.CullMode = D3D11_CULL_NONE;
		desc.DepthClipEnable = FALSE;
		Device->CreateRasterizerState(&desc, &mNoDepthRasterizerState);

		desc.CullMode = D3D11_CULL_NONE;
		desc.FillMode = D3D11_FILL_WIREFRAME;
		Device->CreateRasterizerState(&desc, &mWireRasterizerState);


	}

	{
		CD3D11_DEPTH_STENCIL_DESC desc(D3D11_DEFAULT);

		desc.DepthEnable = true;
		desc.StencilEnable = false;
		desc.DepthFunc = D3D11_COMPARISON_LESS;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		Device->CreateDepthStencilState(&desc, &mDefaultDepthState);
	}

	// ### Render Texture To Screen Buffer and Vertex Buffer
	{
		mTextureToScreenCBuffer->D3DCreateDevice(Device, BackBufferSurfaceDesc);
		mTextureToScreenCBuffer->Data.mDestRect = D3DXVECTOR4(0, 0, 1, 1);

		// create vertices
		// the full QUAD will be created in the Geometry shader, we only need the upper left corner.
		PositionTexCoordVertex vertices[] =
		{
			{ D3DXVECTOR3( -1.0f, 1.0f, 0.5f), D3DXVECTOR2(0.0f, 0.0f) },
		};

		D3D11_BUFFER_DESC bd;
		ZeroMemory( &bd, sizeof(bd) );
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof( PositionTexCoordVertex );
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		bd.StructureByteStride = 0;
		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory( &InitData, sizeof(InitData) );
		InitData.pSysMem = vertices;
		InitData.SysMemPitch = 0;
		InitData.SysMemSlicePitch = 0;
		Device->CreateBuffer( &bd, &InitData, &mTextureToScreenVertexBuffer );
	}

	if(!sceneBuilder->GetScene()->D3DCreateDevice(Device, BackBufferSurfaceDesc))
		DXUT_ERR_MSGBOX(L"Scene D3D device not created", S_FALSE);

	return true;
}
void MainApp::D3DReleaseDevice()
{
	sceneBuilder->GetScene()->D3DReleaseDevice();
	mTextureToScreenCBuffer->D3DReleaseDevice();
	SAFE_DELETE(mTextureToScreenCBuffer);
	SAFE_RELEASE(mTextureToScreenVertexBuffer);
	SAFE_DELETE(mTextureToScreenVS);
	SAFE_DELETE(mTextureToScreenGS);
	SAFE_DELETE(mTextureToScreenPS);
}

bool MainApp::D3DCreateSwapChain( ID3D11Device* Device, IDXGISwapChain* SwapChain, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc )
{

	sceneBuilder->GetScene()->D3DCreateSwapChain(Device, SwapChain, BackBufferSurfaceDesc);

	return true;
}
void MainApp::D3DReleaseSwapChain()
{
	sceneBuilder->GetScene()->D3DReleaseSwapChain();
}

void MainApp::Update( float elapsedTime )
{
	sceneBuilder->GetScene()->Move(elapsedTime);
}

void MainApp::Render( ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext, const D3D11_VIEWPORT* viewPort, float fElapsedTime )
{
	//d3dDeviceContext->OMSetDepthStencilState(mDepthState, 0);
	d3dDeviceContext->RSSetState(mNoDepthRasterizerState);
	d3dDeviceContext->OMSetDepthStencilState(mDefaultDepthState, 0);


	sceneBuilder->GetScene()->Render(d3dDeviceContext, viewPort, fElapsedTime);
}

void MainApp::HandleMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	sceneBuilder->GetScene()->HandleMessages(hWnd, uMsg, wParam, lParam);
}

void MainApp::InitScene()
{
	sceneBuilder = new TestSceneBuilder(this);
	if (!sceneBuilder->Init()) return;
}

void MainApp::RenderTextureToScreen( ID3D11DeviceContext* d3dDeviceContext, ID3D11RenderTargetView* backBuffer, D3DXVECTOR4* destRectangle, /* const UIConstants* ui, */ ID3D11ShaderResourceView* textureSRV )
{
	
	mTextureToScreenCBuffer->Data.mDestRect = *destRectangle;
	mTextureToScreenCBuffer->UpdateBuffer(d3dDeviceContext);
	ID3D11Buffer* tempTexToScreenCB = mTextureToScreenCBuffer->GetBuffer();


	d3dDeviceContext->IASetInputLayout(mPositionTexCoordLayout);
	// Set vertex buffer
	UINT stride = sizeof( PositionTexCoordVertex );
	UINT offset = 0;
	d3dDeviceContext->IASetVertexBuffers( 0, 1, &mTextureToScreenVertexBuffer, &stride, &offset );
	d3dDeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );

	d3dDeviceContext->VSSetShader(mTextureToScreenVS->GetShader(), NULL, 0 );
	d3dDeviceContext->VSSetConstantBuffers(2, 1, &tempTexToScreenCB);

	d3dDeviceContext->GSSetShader(mTextureToScreenGS->GetShader(), NULL, 0);
	d3dDeviceContext->GSSetConstantBuffers(2, 1, &tempTexToScreenCB);

	d3dDeviceContext->PSSetShader( mTextureToScreenPS->GetShader(), NULL, 0 );
	d3dDeviceContext->PSSetShaderResources( 0, 1, &textureSRV );
	//d3dDeviceContext->PSSetSamplers( 0, 1, &mDiffuseSampler );

	//d3dDeviceContext->OMSetDepthStencilState(mDepthState, 0);
	d3dDeviceContext->OMSetRenderTargets(1, &backBuffer, NULL);
	//d3dDeviceContext->RSSetState(mRasterizerState);
	d3dDeviceContext->Draw(1, 0);

	d3dDeviceContext->OMSetRenderTargets(0, 0, 0);
	ID3D11ShaderResourceView* pSRV = {NULL};
	d3dDeviceContext->PSSetShaderResources(0, 1, &pSRV);
	
	//SAFE_RELEASE(tempTexToScreenCB);
	//SAFE_RELEASE(pSRV);

}

void MainApp::ChangeValue( int change)
{
	sceneBuilder->GetScene()->ChangeVale(change);
}


