#pragma once
#include "ID3DResource.h"
/************************************************************************/
/* Geometry type Definiton                                              */
/************************************************************************/


class BaseGeometry : public ID3DDeviceResource
{
public:
	explicit BaseGeometry(bool isItStatic = true);
	virtual ~BaseGeometry(void);

	void Bind(ID3D11DeviceContext* ImmediateContext);
	void Render(ID3D11DeviceContext* ImmediateContext);

	bool IsStatic() const;
	void SetStatic(bool isItStatic);

protected:

	// Vertex Buffer and stuffs


	//loading the effect, input layout shaders for material
	// Vertex buffer used for rendering with D3D.
	ID3D11Buffer* mVertexBuffer;
	// Index buffer used for rendering with D3D.
	ID3D11Buffer* mIndexBuffer;
	// Number of elements in the D3D buffers. (if IndexBuffer=null then NumVertices else NumIndices)
	UINT mNumElements;
	// Stride in the vertex buffer.
	UINT mStride;
	// Topology of the primitive.
	D3D11_PRIMITIVE_TOPOLOGY mTopology;
	// Format of the elements in the index buffer.
	DXGI_FORMAT mIndexFormat;

	bool mIsStatic;
};

