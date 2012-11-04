#pragma once

/************************************************************************/
/* Main App Management
	- manages worlds, input, GBuffer render
	- init, update, render, messages 
	- calls appropriate functions for input (movement, create block or swirl, etc.)

	- Debug stuff

*/
/************************************************************************/

#include "DXUT.h"
#include "DXUT\Optional\DXUTcamera.h"
#include "Scene.h"
#include "ConstantBuffer.h"

class MainApp
{
public:
	MainApp(void);
	~MainApp(void);

	void OnD3D11ResizedSwapChain(ID3D11Device* d3dDevice, const DXGI_SURFACE_DESC* backBufferDesc);

	void Update(float elapsedTime);

	void Render(ID3D11Device*, ID3D11DeviceContext*, 
		const D3D11_VIEWPORT*, 
		float fElapsedTime
		);

	void HandleMessages(HWND, UINT, WPARAM, LPARAM);

	bool D3DCreateDevice(ID3D11Device* Device, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc);
	void D3DReleaseDevice();

	bool D3DCreateSwapChain(ID3D11Device* Device, IDXGISwapChain* SwapChain, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc);
	void D3DReleaseSwapChain();

private:

	CFirstPersonCamera* mViewerCamera;
	ConstantBuffer<CameraConstants>* cameraCBuffer;


	Scene* currentScene;
	//include SceneBuilder?
};

