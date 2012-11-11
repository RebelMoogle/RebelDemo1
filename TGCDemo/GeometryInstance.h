#pragma once


/************************************************************************/
/* Geometry Instance manangement
	- handles one Geometry instance.

*/
/************************************************************************/

class BaseGeometry;
class BaseMaterial;
class Scene;

class GeometryInstance
{
public:
	GeometryInstance(BaseGeometry* Geometry, BaseMaterial* Material, Scene* Scene);
	virtual ~GeometryInstance(void);

	void Render(ID3D11DeviceContext* ImmediateContext);
	void RenderToGBuffer(ID3D11DeviceContext* d3dDeviceContext);

	const BaseGeometry* GetGeometry() const;
	const D3DXMATRIX& GetTransform() const;
	
	void SetTransform( D3DXMATRIX* newTransform );
	float GetAngle();
	void SetAngle(float);

private:

	BaseGeometry* mGeometry;
	BaseMaterial* mMaterial;
	Scene* mScene;

	D3DXMATRIX mTransform;
	float mAngle;
};

