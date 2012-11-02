#pragma once
#include "Shader.h"
#include <vector>

/******************
	Base Particle System
	 - base functions. 
	 - create particles system by inheriting. 

		- change update class.

	- Since the Particles are computed on the GPU, not much info is needed (no single particles, etc.)
		- need different particle shader for every different system?
		- Particles created and updated on GPU
		- Particles die -> particles reset (or just not drawn)
		- min, max, death rate, lifetime, changerate, speed are defined in the class and sent to the shader.

*/

class ParticleSystem
{
public:
	ParticleSystem(void);
	~ParticleSystem(void);

	// initial device creation
	bool D3DCreateDevice(ID3D11Device* Device, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc);
	void D3DReleaseDevice();

	// resolution / fullscreen changes / initialization
	bool D3DCreateSwapChain(ID3D11Device* Device, IDXGISwapChain* SwapChain, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc);
	void D3DReleaseSwapChain();

	void Update();

	void Render(ID3D11DeviceContext* d3dDeviceContext);

private:

	void CreateParticleBuffers(ID3D11DeviceContext* d3dDeviceContext, ID3D11DepthStencilView* depthBufferView);

	ID3D11InputLayout* mParticleLayout;
	std::vector<ID3D11Buffer*> mParticleBuffers;

	ID3D11ShaderResourceView* particleTexture;

	// shader for computing particles. output directly to buffer
	VertexShader* mParticleSystemVS;
	ID3D11GeometryShader* mParticleSystemGS; // no output stream shader factory yet. 
	

	// shader for rendering particles, render to quads created for every particle in Geometry Shader
	VertexShader* mRenderParticlesVS;
	GeometryShader* mRenderParticlesGS;
	PixelShader* mRenderParticlesPS;

	ID3D11Buffer* mParticleCBuffer;
	ID3D11RasterizerState* mRasterizerState;
	ID3D11SamplerState* mDiffuseSampler;
	ID3D11DepthStencilState* mDepthState;

	ID3D11BlendState* mParticleBlend;
};

