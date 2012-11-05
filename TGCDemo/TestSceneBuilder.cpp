#include "DXUT.h"
#include "TestSceneBuilder.h"
#include "MainApp.h"
#include "Scene.h"
#include "BoxGeometry.h"
#include "DefaultMaterial.h"


TestSceneBuilder::TestSceneBuilder(MainApp* givenApp):
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
	BoxGeometry* groundPlane= new BoxGeometry(D3DXVECTOR3(0, -10, 0), D3DXVECTOR3(1000, 10, 1000));
	sceneToBuild->AddGeometry(groundPlane);

	BoxGeometry* testBox= new BoxGeometry(D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(1, 1, 1));
	sceneToBuild->AddGeometry(testBox);

	//create material // put in constants
	const D3DXVECTOR4 blue = D3DXVECTOR4(0.15f, 0.15f, 0.5f, 1.0f);
	const D3DXVECTOR4 black = D3DXVECTOR4(0.1f, 0.1f, 0.1f, 1.0f);

	DefaultMaterial* matBlue = new DefaultMaterial(blue, blue);
	DefaultMaterial* matBlack = new DefaultMaterial(black, blue);
	sceneToBuild->AddMaterial(matBlue);
	sceneToBuild->AddMaterial(matBlack);

	//add to scene
	sceneToBuild->CreateGeometryInstance(groundPlane, matBlack);
	sceneToBuild->CreateGeometryInstance(testBox, matBlack);

	// add and set camera
	CFirstPersonCamera* sceneCamera = new CFirstPersonCamera();
	sceneCamera->SetRotateButtons(true, false, false);
	sceneCamera->SetDrag(true);
	sceneCamera->SetEnableYAxisMovement(true);
	sceneCamera->SetEnablePositionMovement(true);
	sceneCamera->SetScalers(0.01f, 200.0f);

	sceneCamera->SetViewParams(&D3DXVECTOR3(500.0f, 160.0f, 0), &D3DXVECTOR3(500.0f, 0.0f, 500.0f));

	sceneToBuild->SetCamera(sceneCamera);

	// set sun constants ??
	sceneToBuild->SetSun(&D3DXVECTOR3(600.0f, 500.0f, 500.0f), &D3DXVECTOR3(0.0f, -0.9f, 0.1f), &D3DXVECTOR4(0.0f,0.0f,0.0f,1.0f));

	// maybe lights, you know, to see stuff?

	return true;
}
