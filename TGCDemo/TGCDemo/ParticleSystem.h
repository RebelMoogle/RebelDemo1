#pragma once
#include "Shader.h"

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

	void Render();

private:

	ID3D11InputLayout* mMeshVertexLayout;

	VertexShader* mParticleSystemVS;
	GeometryShader* mParticleSystemGS;
	PixelShader* mParticleSystemPS;

	ID3D11Buffer* mParticleCBuffer;
	ID3D11RasterizerState* mRasterizerState;
	ID3D11SamplerState* mDiffuseSampler;
	ID3D11DepthStencilState* mDepthState;

	ID3D11BlendState* mParticleBlend;
};

