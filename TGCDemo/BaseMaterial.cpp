#include "DXUT.h"
#include "BaseMaterial.h"


BaseMaterial::BaseMaterial(void)
{
}


BaseMaterial::~BaseMaterial(void)
{
}

void BaseMaterial::Bind( ID3D11DeviceContext* ImmediateContext, Scene* scene )
{
	// bind material shader
	ImmediateContext->VSSetShader(materialVS->GetShader(), NULL, 0);
	ImmediateContext->GSSetShader(NULL, NULL, 0);
	ImmediateContext->PSSetShader(materialPS->GetShader(), NULL, 0);

	BindParams(ImmediateContext);

	ImmediateContext->IASetInputLayout(mInputLayout);
}
