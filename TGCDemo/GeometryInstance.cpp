#include "DXUT.h"
#include "GeometryInstance.h"
#include "BaseMaterial.h"
#include "BaseGeometry.h"
#include "Scene.h"



GeometryInstance::GeometryInstance(BaseGeometry* Geometry, BaseMaterial* Material, Scene* Scene) :
mGeometry(Geometry),
mMaterial(Material),
mScene(Scene)
{
	mAngle = 0.0f;
}


GeometryInstance::~GeometryInstance(void)
{
}

void GeometryInstance::Render( ID3D11DeviceContext* ImmediateContext )
{
	mMaterial->Bind(ImmediateContext, mScene);

	mGeometry->Bind(ImmediateContext);

	mGeometry->Render(ImmediateContext);
}

void GeometryInstance::RenderToGBuffer( ID3D11DeviceContext* d3dDeviceContext )
{
	mGeometry->Bind(d3dDeviceContext);
	mMaterial->BindParams(d3dDeviceContext);
	mGeometry->Render(d3dDeviceContext);
}

const BaseGeometry* GeometryInstance::GetGeometry() const
{

	return mGeometry;
}

const D3DXMATRIX& GeometryInstance::GetTransform() const
{

	return mTransform;
}

void GeometryInstance::SetTransform( D3DXMATRIX* newTransform )
{
	mTransform = *newTransform;
}

float GeometryInstance::GetAngle()
{
	return mAngle;
}

void GeometryInstance::SetAngle( float angle)
{
	mAngle = angle;
}