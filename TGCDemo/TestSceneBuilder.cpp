#include "DXUT.h"
#include "TestSceneBuilder.h"
#include "MainApp.h"
#include "Scene.h"
#include "BoxGeometry.h"
#include "DefaultMaterial.h"


TestSceneBuilder::TestSceneBuilder(App* givenApp):
BaseSceneBuilder(givenApp)
{

}


TestSceneBuilder::~TestSceneBuilder(void)
{

}

bool TestSceneBuilder::Init()
{
	sceneToBuild = new Scene(GetApp());

	// create geometry
	BoxGeometry* groundPlane= new BoxGeometry(D3DXVECTOR3(0, -10, 0), D3DXVECTOR3(cityWidth, 10, cityDepth));
	sceneToBuild->AddGeometry(groundPlane);

	BoxGeometry* testBox= new BoxGeometry(D3DXVECTOR3(0, -10, 0), D3DXVECTOR3(cityWidth, 10, cityDepth));
	sceneToBuild->AddGeometry(testBox);

	//create material // put in constants
	const D3DXVECTOR4 blue = D3DXVECTOR4(0.15f, 0.15f, 0.5f, 1.0f);
	const D3DXVECTOR4 black = D3DXVECTOR4(0.1f, 0.1f, 0.1f, 1.0f);

	DefaultMaterial* matBlue = new DefaultMaterial(blue, blue);
	DefaultMaterial* matBlack = new DefaultMaterial(black, blue);
	sceneToBuild->AddMaterial(matBlue);
	sceneToBuild->AddMaterial(matBlack);

	//add to scene
	sceneToBuild->AddGeometryInstance(groundPlane, matBlack)
	sceneToBuild->AddGeometryInstance(testBox, matBlack)

	// add and set camera

	// maybe lights, you know.

}
w