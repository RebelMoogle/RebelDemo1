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
#include "ConstantBuffer.h"
#include "constants.h"

class BaseSceneBuilder;

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

	void RenderTextureToScreen(	ID3D11DeviceContext* d3dDeviceContext, ID3D11RenderTargetView* backBuffer, D3DXVECTOR4* destRectangle, // const UIConstants* ui,
		ID3D11ShaderResourceView* textureSRV);
	
	ID3D11InputLayout* mPositionTexCoordLayout;

private:

	void InitScene();

	BaseSceneBuilder* sceneBuilder;

	Scene* currentScene;

	ID3D11Buffer* craftMyGBufferChangesCamera;

	//render Texture
	VertexShader* mTextureToScreenVS;
	GeometryShader* mTextureToScreenGS;
	PixelShader* mTextureToScreenPS;
	ID3D11Buffer* mTextureToScreenVertexBuffer;
	ConstantBuffer<TextureToScreenConstants>* mTextureToScreenCBuffer;

	ID3D11RasterizerState* mWireRasterizerState;
	ID3D11RasterizerState* mNoDepthRasterizerState;

	ID3D11DepthStencilState* mDefaultDepthState;


};

