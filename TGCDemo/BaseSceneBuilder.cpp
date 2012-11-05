#include "DXUT.h"
#include "BaseSceneBuilder.h"
#include "Scene.h"

// ================================================================================
BaseSceneBuilder::BaseSceneBuilder(MainApp* givenApp) :
sceneToBuild(NULL),
parentApp(givenApp)
{

}

// ================================================================================
BaseSceneBuilder::~BaseSceneBuilder()
{
	SAFE_DELETE(sceneToBuild);
}

// ================================================================================
Scene* BaseSceneBuilder::GetScene()
{
	return sceneToBuild;
}

// ================================================================================
MainApp* BaseSceneBuilder::GetApp()
{
	return parentApp;
}
