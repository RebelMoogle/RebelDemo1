#pragma once

#include "Shader.h"

class Scene;

/************************************************************************/
/* Material Type Definition                                             */
/************************************************************************/


class BaseMaterial
{
public:
	BaseMaterial(void);
	virtual ~BaseMaterial(void);

	void Bind(ID3D11DeviceContext* ImmediateContext, Scene* scene);

	virtual void BindParams(ID3D11DeviceContext* ImmediateContext) = 0;

protected:

	//loading the effect, input layout shaders for material

};

