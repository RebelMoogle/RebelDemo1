#include "DXUT.h"
#include "Scene.h"


Scene::Scene(App* givenApp) :
parentApp(givenApp)
{
		_Geometries.clear();
		_Materials.clear();
		_GeometryInstances.clear();
}



Scene::~Scene(void)
{
}
