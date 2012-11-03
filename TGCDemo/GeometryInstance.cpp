#include "DXUT.h"
#include "GeometryInstance.h"


GeometryInstance::GeometryInstance(BaseGeometry* Geometry, BaseMaterial* Material, Scene* Scene)
{
}


GeometryInstance::~GeometryInstance(void)
{
}

void GeometryInstance::Render( ID3D11DeviceContext* ImmediateContext )
{

}

void GeometryInstance::RenderToGBuffer( ID3D11DeviceContext* d3dDeviceContext )
{

}

const BaseGeometry* GeometryInstance::GetGeometry() const
{

	return _Geometry;
}

const D3DXMATRIX& GeometryInstance::GetTransform() const
{

	return mTransform;
}

void GeometryInstance::SetTransform( D3DXMATRIX* newTransform )
{
	mTransform = *newTransform;
}
