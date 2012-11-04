#pragma once
#include "BaseMaterial.h"
#include "ConstantBuffer.h"

struct DefaultConstants
{
	D3DXVECTOR4 diffuseColor;
	D3DXVECTOR4 specularColor;
	D3DXVECTOR4 transmissive;
};

class DefaultMaterial :
	public BaseMaterial
{
public:
	DefaultMaterial(const D3DXVECTOR4& diffuse = D3DXVECTOR4(1.0f,1.0f,1.0f, 1.0f), const D3DXVECTOR4& specular = D3DXVECTOR4(1.0f,1.0f,1.0f, 1.0f), const D3DXVECTOR4& transmissive = D3DXVECTOR4(0.0f,0.0f,0.0f, RefractIdx_Vacuum));
	~DefaultMaterial(void);

	// Binds effect parameters.
	void BindParams(ID3D11DeviceContext* ImmediateContext);

	bool D3DCreateDevice(ID3D11Device* Device, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc);
	void D3DReleaseDevice();

private:		

	ConstantBuffer<OmniConstants>* omniCBuffer;

	static bool isInitialized;
};

