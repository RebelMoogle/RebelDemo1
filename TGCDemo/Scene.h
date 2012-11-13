#pragma once

/************************************************************************/
/* Scene Management class
	- used for managing the scene
	- geometry and materials can be added
	- renders itself (to GBuffer)
	- handles geometry, camera, particle systems and lights of this world instance.
		- init, update, render, destroy

*/
/************************************************************************/



#include "DXUT.h"
#include <vector>
#include "Shader.h"
#include "GeometryInstance.h"
#include "DXUTcamera.h"
#include "MainApp.h"
#include "constants.h"
#include "ConstantBuffer.h"

class MainApp;
class BaseGeometry;
class BaseMaterial;
class BaseGeometryInstance;
class ParticleSystem;

class Scene
{
public:
	explicit Scene(MainApp*);
	~Scene(void);

	MainApp* GetApp();

	void AddGeometry(BaseGeometry* Geometry);

	void AddMaterial(BaseMaterial* Material);

	void CreateGeometryInstance(BaseGeometry* Geometry, BaseMaterial* Material);

	void SetCamera(CFirstPersonCamera* givenCamera);

	bool D3DCreateDevice(ID3D11Device* Device, const DXGI_SURFACE_DESC* BackBudderSurfaceDesc);

	void D3DReleaseDevice();

	bool D3DCreateSwapChain(ID3D11Device* d3dDevice, IDXGISwapChain* SwapChain, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc);
	void D3DReleaseSwapChain();

	void Render(ID3D11DeviceContext*, const D3D11_VIEWPORT*, float);

	void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void Move( float elapsedTime );

	void SetSun(D3DXVECTOR3 Position, D3DXVECTOR3 Direction, D3DXVECTOR4 Power);

	void ChangeVale(int change);

private:

	void RenderForward(ID3D11DeviceContext*, const D3D11_VIEWPORT*, LightConstants* lightConstData);

	// !camera constant buffer needs to be set before calling RenderGBuffer!
	void RenderGBuffer(ID3D11DeviceContext* d3dDeviceContext, const D3D11_VIEWPORT* givenViewport);
	void RenderLightGBuffer(ID3D11DeviceContext*, const D3D11_VIEWPORT* lightViewPorts);

	void ToneMapping(ID3D11DeviceContext* d3dDeviceContext, ID3D11ShaderResourceView* inputRender, ID3D11Buffer* quadVertices);
	
	VertexShader* toneMappingVS;
	PixelShader*  toneMappingPS;

	// Parent App.
	MainApp* const parentApp;

	std::vector<BaseGeometry*> mGeometries;
	std::vector<BaseMaterial*> mMaterials;
	std::vector<GeometryInstance*> mGeometryInstances;

	// ### GBuffer stuff ###
	VertexShader* craftMyGBufferVS;
	PixelShader* craftMyGBufferPS;

	ID3D11InputLayout* mMeshVertexLayout;

	std::vector<ID3D11Texture2D* > mGBuffer;
	//cache of list of RT pointers for GBuffer
	std::vector<ID3D11RenderTargetView*> mGBufferRTV;
	//cache of list of SRV pointers for Gbuffer
	std::vector<ID3D11ShaderResourceView*> mGBufferSRV;
	unsigned int mGBufferWidth;
	unsigned int mGBufferHeight;

	// final Splatting render target
	ID3D11Texture2D* splattingTexture;
	ID3D11ShaderResourceView* linesTexture;
	ID3D11RenderTargetView* splattingRTV;
	ID3D11ShaderResourceView* splattingSRV;

	ID3D11RasterizerState* mRasterizerState;
	ID3D11SamplerState* mDiffuseSampler;
	ID3D11SamplerState* mMirrorSampler;
	ID3D11DepthStencilState* mDepthState;
	ID3D11BlendState* mGeometryBlendState;
	ID3D11BlendState* mDeferredShadeBlend;

	//CaMERA
	CFirstPersonCamera* mViewerCamera;
	ConstantBuffer<CameraConstants>* cameraCBuffer;

	// ### normal fullscreen quad (position and texcoord) ###
	ID3D11Buffer* screenQuadVertices;

	// ### direct light shader ###
	ID3D11Buffer* lightCBuffer;
	VertexShader* DeferredShadeVS;
	//PixelShader*  DeferredShadePS;
	PixelShader*  DeferredSunShadePS;
	//ID3D11SamplerState* shadowSampler;

	//Sun
	D3DXVECTOR3* mSunPosition;
	D3DXVECTOR3* mSunDirection;
	D3DXVECTOR4* mSunPower;

	//Particle System
	ParticleSystem* mLineParticles;
};

