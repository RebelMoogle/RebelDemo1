#pragma once

/************************************************************************/
/* World Management class
	- used for managing the world
	- geometry and materials can be added
	- renders itself (to GBuffer)
	- handles geometry, camera, particle systems and lights of this world instance.
		- init, update, render, destroy

*/
/************************************************************************/



#include "DXUT.h"
#include <vector>
#include "Shader.h"

class App;
class BaseGeometry;
class BaseMaterial;
class BaseGeometryInstance;

class Scene
{
public:
	explicit Scene(App*);
	~Scene(void);

	App* GetApp();

	void AddGeometry(BaseGeometry* Geometry);

	void AddMaterial(BaseMaterial* BaseMaterial);

	void CreateGeometryInstance(BaseGeometry* Geometry, BaseMaterial* Material);

	void SetCamera();

	bool D3DCreateDevice(ID3D11Device* Device, const DXGI_SURFACE_DESC* BackBudderSurfaceDesc);

	void D3DReleaseDevice();

	bool D3DCreateSwapChain(ID3D11Device* Device, IDXGISwapChain* SwapChain, const DXGI_SURFACE_DESC BackBufferSurfaceDesc);
	void D3DReleaseSwapChain();

	void Render(ID3D11DeviceContext*, const D3D11_VIEWPORT*, float);

	void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void Move( float elapsedTime );

private:

	void RenderForward(ID3D11DeviceContext*, const D3D11_VIEWPORT*);

	// !camera constant buffer needs to be set before calling RenderGBuffer!
	void RenderGBuffer(ID3D11DeviceContext* d3dDeviceContext, const D3D11_VIEWPORT* givenViewport);
	void RenderLightGBuffer(ID3D11DeviceContext*, const D3D11_VIEWPORT* lightViewPorts);

	// Parent App.
	App* const parentApp;

	std::vector<BaseGeometry*> _Geometries;
	std::vector<BaseMaterial*> _Materials;
	std::vector<BaseGeometryInstance*> _GeometryInstances;

	// ### GBuffer stuff ###
	VertexShader* craftMyGBufferVS;
	PixelShader* craftMyGBufferPS;

	VertexShader* craftMyLightGBufferVS;
	PixelShader* craftMyLightGBufferPS;

	ID3D11InputLayout* mMeshVertexLayout;

	std::vector<ID3D11Texture2D* > mGBuffer;
	//cache of list of RT pointers for GBuffer
	std::vector<ID3D11RenderTargetView*> mGBufferRTV;
	//cache of list of SRV pointers for Gbuffer
	std::vector<ID3D11ShaderResourceView*> mGBufferSRV;
	unsigned int mGBufferWidth;
	unsigned int mGBufferHeight;

	//Light GBuffer
	//light GBuffer
	std::vector< ID3D11Texture2D* > mLightGBuffer;
	//cache of list of RT pointers for GBuffer
	std::vector<ID3D11RenderTargetView*> mLightGBufferRTV;
	//cache of list of SRV pointers for Gbuffer
	std::vector<ID3D11ShaderResourceView*> mLightGBufferSRV;
	unsigned int mLightGBufferWidth;
	unsigned int mLightGBufferHeight;
	ID3D11DepthStencilView* lightDepthView;

	// final Splatting render target
	ID3D11Texture2D* splattingTexture;
	ID3D11RenderTargetView* splattingRTV;
	ID3D11ShaderResourceView* splattingSRV;

	ID3D11RasterizerState* mRasterizerState;
	ID3D11SamplerState* mDiffuseSampler;
	ID3D11DepthStencilState* mDepthState;
	ID3D11BlendState* mGeometryBlendState;
	ID3D11BlendState* mDeferredShadeBlend;

	// ### normal fullscreen quad (position and texcoord) ###
	ID3D11Buffer* screenQuadVertices;
	//ID3D11InputLayout* screenQuadLayout;

	// ### direct light shader ###
	ID3D11Buffer* lightCBuffer;
	VertexShader* DeferredShadeVS;
	PixelShader*  DeferredShadePS;
	PixelShader*  DeferredSunShadePS;
	ID3D11SamplerState* shadowSampler;


};

