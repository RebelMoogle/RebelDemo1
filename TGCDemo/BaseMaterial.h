#pragma once

#include "Shader.h"
#include "ID3DResource.h"

class Scene;

/************************************************************************/
/* Material Type Definition                                             */
/************************************************************************/


class BaseMaterial : public ID3DDeviceResource
{
public:
	BaseMaterial(void);
	virtual ~BaseMaterial(void);

	void Bind(ID3D11DeviceContext* ImmediateContext, Scene* scene);

	virtual void BindParams(ID3D11DeviceContext* ImmediateContext) = 0;

protected:
	// D3D input layout (assumes pos/normal layout...)
	ID3D11InputLayout* mInputLayout;

	// TODO: Shader	
	VertexShader* materialVS;
	PixelShader* materialPS;

};

