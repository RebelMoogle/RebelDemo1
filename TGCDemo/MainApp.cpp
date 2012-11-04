#include "DXUT.h"
#include "MainApp.h"
#include "constants.h"


MainApp::MainApp(void)
{
	cameraCBuffer = new ConstantBuffer<CameraConstants>();
}


MainApp::~MainApp(void)
{
}

bool MainApp::D3DCreateDevice( ID3D11Device* Device, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc )
{
	mViewerCamera = new CFirstPersonCamera();
	cameraCBuffer->D3DCreateDevice(Device, BackBufferSurfaceDesc);
}
void MainApp::D3DReleaseDevice()
{
	SAFE_DELETE(mViewerCamera);
	cameraCBuffer->D3DReleaseDevice();
}

bool MainApp::D3DCreateSwapChain( ID3D11Device* Device, IDXGISwapChain* SwapChain, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc )
{
	if(mViewerCamera)
		mViewerCamera->SetProjParams(D3DX_PI / 4, (float)BackBufferSurfaceDesc->Width / (float) BackBufferSurfaceDesc->Height, CAMERA_NEAR, CAMERA_FAR);
	
	D3DXMATRIX identityMatrix;
	D3DXMatrixIdentity(&identityMatrix);
	cameraCBuffer->Data.World = identityMatrix;
	cameraCBuffer->Data.View = *mViewerCamera->GetViewMatrix();
	cameraCBuffer->Data.cameraPosition = *mViewerCamera->GetEyePt();
	cameraCBuffer->Data.WorldViewProjection = identityMatrix * cameraCBuffer->Data.View * (*mViewerCamera->GetProjMatrix());

	return true;
}
void MainApp::D3DReleaseSwapChain()
{

}

void MainApp::Update( float elapsedTime )
{
	mViewerCamera->FrameMove(elapsedTime);

	D3DXMATRIX identityMatrix;
	D3DXMatrixIdentity(&identityMatrix);
	cameraCBuffer->Data.World = identityMatrix;
	cameraCBuffer->Data.View = *mViewerCamera->GetViewMatrix();
	cameraCBuffer->Data.cameraPosition = *mViewerCamera->GetEyePt();
	cameraCBuffer->Data.WorldViewProjection = identityMatrix * cameraCBuffer->Data.View * (*mViewerCamera->GetProjMatrix());

}

void MainApp::Render( ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext, const D3D11_VIEWPORT* viewPort, float fElapsedTime )
{
	// get matrices from camera, set them for the shaders.
}

void MainApp::HandleMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	mViewerCamera->HandleMessages(hWnd, uMsg, wParam, lParam);
}


