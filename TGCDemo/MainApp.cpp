#include "DXUT.h"
#include "MainApp.h"
#include "constants.h"
#include "TestSceneBuilder.h"


MainApp::MainApp(void)
{
	InitScene();
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

bool MainApp::D3DCreateDevice( ID3D11Device* d3dDevice, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc )
{
	// Create standard rasterizer state
	{
		CD3D11_RASTERIZER_DESC desc(D3D11_DEFAULT);

		desc.CullMode = D3D11_CULL_NONE;
		desc.DepthClipEnable = FALSE;
		d3dDevice->CreateRasterizerState(&desc, &mNoDepthRasterizerState);

		desc.CullMode = D3D11_CULL_NONE;
		desc.FillMode = D3D11_FILL_WIREFRAME;
		d3dDevice->CreateRasterizerState(&desc, &mWireRasterizerState);


	}

	{
		CD3D11_DEPTH_STENCIL_DESC desc(D3D11_DEFAULT);

		desc.DepthEnable = true;
		desc.StencilEnable = false;
		desc.DepthFunc = D3D11_COMPARISON_LESS;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		d3dDevice->CreateDepthStencilState(&desc, &mDefaultDepthState);
	}

	if(!sceneBuilder->GetScene()->D3DCreateDevice(d3dDevice, BackBufferSurfaceDesc))
		DXUT_ERR_MSGBOX(L"Scene D3D device not created", S_FALSE);

	return true;
}
void MainApp::D3DReleaseDevice()
{
	sceneBuilder->GetScene()->D3DReleaseDevice();
}

bool MainApp::D3DCreateSwapChain( ID3D11Device* Device, IDXGISwapChain* SwapChain, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc )
{
	DXGI_SAMPLE_DESC sampleDesc;
	sampleDesc.Count = 1;
	sampleDesc.Quality = 0;


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


