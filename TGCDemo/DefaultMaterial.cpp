#include "DXUT.h"
#include "DefaultMaterial.h"

bool OmniMaterial::isInitialized = false;

DefaultMaterial::DefaultMaterial(const D3DXVECTOR4& diffuse, const D3DXVECTOR4& specular, const D3DXVECTOR4& transmissive)
{
	omniCBuffer = new ConstantBuffer<OmniConstants>();
	omniCBuffer->Data.diffuseColor = diffuse;
	omniCBuffer->Data.specularColor = specular;
	omniCBuffer->Data.transmissive = transmissive;
}


DefaultMaterial::~DefaultMaterial(void)
{
		D3DReleaseDevice();
		SAFE_DELETE(omniCBuffer);
}

void DefaultMaterial::BindParams( ID3D11DeviceContext* ImmediateContext )
{
	ID3D11Buffer* tempCBuffer = omniCBuffer->GetBuffer();
	ImmediateContext->PSSetConstantBuffers(2, 1, &tempCBuffer);
}

bool DefaultMaterial::D3DCreateDevice( ID3D11Device* Device, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc )
{
	materialVS = new VertexShader(d3dDevice, L"Shaders\\OmniMaterial.hlsl", "OmniMaterialVS");
	materialPS = new PixelShader(d3dDevice, L"Shaders\\OmniMaterial.hlsl", "OmniMaterialPS");

	if(!omniCBuffer->D3DCreateDevice(d3dDevice, BackBufferSurfaceDesc)) return false;

	// Create  mesh input layout
	// isn't there an easier and better way to do this???

	// We need the vertex shader bytecode for this... rather than try to wire that all through the
	// shader interface, just recompile the vertex shader.
	UINT shaderFlags = D3D10_SHADER_ENABLE_STRICTNESS | D3D10_SHADER_PACK_MATRIX_ROW_MAJOR;
	ID3D10Blob *bytecode = 0;
	HRESULT hr = D3DX11CompileFromFile(L"Shaders\\OmniMaterial.hlsl", NULL, 0, "OmniMaterialVS", "vs_5_0", shaderFlags, 0, 0, &bytecode, 0, 0);
	if (FAILED(hr)) 
	{
		assert(false);
	}

	const D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	d3dDevice->CreateInputLayout( 
		layout, ARRAYSIZE(layout), 
		bytecode->GetBufferPointer(),
		bytecode->GetBufferSize(), 
		&mInputLayout);

	bytecode->Release();

	DXUT_SetDebugName(mInputLayout, "OmniMaterial_InputLayout"); 
	return hr == S_OK;
}

void DefaultMaterial::D3DReleaseDevice()
{
	SAFE_DELETE(materialVS);
	SAFE_DELETE(materialPS);
	omniCBuffer->D3DReleaseDevice();

	SAFE_RELEASE(mInputLayout);
}
