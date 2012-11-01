#include "DXUT.h"
#include "ParticleSystem.h"


ParticleSystem::ParticleSystem(void)
{
}


ParticleSystem::~ParticleSystem(void)
{
}

bool ParticleSystem::D3DCreateDevice( ID3D11Device* Device, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc )
{

	// load shaders
	mParticleSystemVS = new VertexShader(Device, L"Shaders\\ParticleSystem.hlsl", "ParticleSystemVS");
	mParticleSystemGS = new GeometryShader(Device, L"Shaders\\ParticleSystem.hlsl", "ParticleSystemGS");
	mParticleSystemPS = new PixelShader(Device, L"Shaders\\ParticleSystem.hlsl", "ParticleSystemPS");

	// create mesh input layout
	{
		ID3D10Blob *bytecode = mParticleSystemVS->GetByteCode();

		const D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{"position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"normal", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"texCoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};

		Device->CreateInputLayout(
			layout, ARRAYSIZE(layout),
			bytecode->GetBufferPointer(),
			bytecode->GetBufferSize(),
			&mMeshVertexLayout);
	}

	// create Rasterizer state
	{
		CD3D11_RASTERIZER_DESC desc(D3D11_DEFAULT);
		desc.CullMode = D3D11_CULL_BACK;
		desc.FrontCounterClockwise = FALSE;
		Device->CreateRasterizerState(&desc, &mRasterizerState);
	}

	// Create sampler state
	{
		CD3D11_SAMPLER_DESC desc(D3D11_DEFAULT);
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		Device->CreateSamplerState(&desc, &mDiffuseSampler);
	}

	// Create Depth State
	{
		CD3D11_DEPTH_STENCIL_DESC desc(D3D11_DEFAULT);
		desc.StencilEnable = false;
		desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		desc.DepthEnable = true;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		Device->CreateDepthStencilState(&desc, &mDepthState);
	}

	// Create particle blend state
	{
		// splat particles, blend onto backbuffer or rendertarget
		CD3D11_BLEND_DESC* blendDesc = new CD3D11_BLEND_DESC();
		blendDesc->AlphaToCoverageEnable = false;
		blendDesc->IndependentBlendEnable = false;

		D3D11_RENDER_TARGET_BLEND_DESC RTBlendDesc = D3D11_RENDER_TARGET_BLEND_DESC();
		RTBlendDesc.BlendEnable = true;
		RTBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
		RTBlendDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
		RTBlendDesc.DestBlend = D3D11_BLEND_ONE;
		RTBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		RTBlendDesc.SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
		RTBlendDesc.DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
		RTBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		blendDesc->RenderTarget[0] = RTBlendDesc;


		HRESULT hr = Device->CreateBlendState(blendDesc, &mParticleBlend);
		if (FAILED(hr)) {
			DXUT_ERR_MSGBOX(L"failed to create particle blend state", hr);
		}
	}

	// create vertex, index and constant Buffers

	// create swap buffers (2, size of max photons

	// create constant buffers

	// load particle texture
}

bool ParticleSystem::D3DCreateSwapChain( ID3D11Device* Device, IDXGISwapChain* SwapChain, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc )
{

}

void ParticleSystem::Update()
{
	// update shaders/particle system with new time and changed values.
	// access constant buffer
}

void ParticleSystem::Render()
{
	// load shaders
	// sync constant buffer

	// set buffers and shaders for rendering
	// render. (to screen)
}

void ParticleSystem::D3DReleaseDevice()
{

}

void ParticleSystem::D3DReleaseSwapChain()
{

}

