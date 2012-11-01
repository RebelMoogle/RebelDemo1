#pragma once
/************************************************************************/
/* Geometry type Definiton                                              */
/************************************************************************/


class BaseGeometry
{
public:
	explicit BaseGeometry(void);
	virtual ~BaseGeometry(void);

	void Bind(ID3D11DeviceContext* ImmediateContext);
	void Render(ID3D11DeviceContext* ImmediateContext);

protected:

	// Vertex Buffer and stuffs
};

