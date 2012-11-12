#pragma once
#include "Shader.h"
#include <vector>
#include "ConstantBuffer.h"

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

/*
float3 position     : POSITION;
float4 color		: COLOR;
float3 direction	: DIRECTION;
float2 spawnRange	: SPAWNRANGE;
float2 durationRange: DURATIONRANGE;
float duration		: DURATION;
float speed			: SPEED;
float rotation		: ROTATION;
float size			: SIZE;
bool alive			: ALIVE;
*/
struct PARTICLEDATA
{
	D3DXVECTOR4 position;
	D3DXVECTOR4 color;
	D3DXVECTOR3 direction;
	float duration;
	float speed;
	float rotation;
	float size;
	UINT flags; // first flag: alive | more flags (max 23)
};

struct PARTICLESYSTEMDATA
{
	D3DXVECTOR4 colorStart;
	D3DXVECTOR4 colorEnd;
	D3DXVECTOR4 colorDeviation;
	D3DXVECTOR4 positionStart;
	D3DXVECTOR4 positionDeviation;
	float spawnTime;
	float spawnDeviation;
	float durationTime;
	float durationDeviation;
	float speed;
	float speedDeviation;
	float rotation;
	float rotationDeviation;
	float sizeStart;
	float sizeDeviation;
};

// Data that changes each frame for the particles
struct PARTICLEFRAMEDATA
{
	float delta;
};

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

	void Update(float elapsedTime);

	void Render(ID3D11DeviceContext* d3dDeviceContext, ID3D11RenderTargetView* particleRTV, ID3D11Buffer* cameraBuffer);

	void SetSystemData(PARTICLESYSTEMDATA* newSysData);

private:

	// PARTICLE SYSTEM SETTINGS
	UINT mMaxParticleCount;

	bool mInitParticle;

	ID3D11InputLayout* mParticleLayout;
	ID3D11Buffer* mParticleBufferStream;
	ID3D11Buffer* mParticleBufferDraw;

	ID3D11ShaderResourceView* particleTexture;

	// shader for computing particles. output directly to buffer
	VertexShader* mParticleSystemVS;
	ID3D11GeometryShader* mParticleSystemGS; // no output stream shader factory yet. 
	

	// shader for rendering particles, render to quads created for every particle in Geometry Shader
	VertexShader* mRenderParticlesVS;
	GeometryShader* mRenderParticlesGS;
	PixelShader* mRenderParticlesPS;

	ID3D11ShaderResourceView* mParticleTexture;
	ID3D11ShaderResourceView* mNoiseTexture;
	ID3D11RasterizerState* mRasterizerState;
	ID3D11SamplerState* mDiffuseSampler;
	ID3D11DepthStencilState* mDepthState;

	ID3D11BlendState* mParticleBlend;

	bool mSystemChange;
	ConstantBuffer<PARTICLESYSTEMDATA>* mSystemCB;
	ConstantBuffer<PARTICLEFRAMEDATA>* mFrameCB;
};

