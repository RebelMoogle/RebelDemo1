#include "DXUT.h"
#include "TestSceneBuilder.h"
#include "MainApp.h"
#include "Scene.h"
#include "BoxGeometry.h"
#include "DefaultMaterial.h"
#include <random>


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
	BoxGeometry* groundPlane= new BoxGeometry(D3DXVECTOR3(-500, -10, -500), D3DXVECTOR3(1000, 10, 1000));
	sceneToBuild->AddGeometry(groundPlane);

	BoxGeometry* testBox= new BoxGeometry(D3DXVECTOR3(-0.5, 0, -0.5), D3DXVECTOR3(1, 1, 1));
	sceneToBuild->AddGeometry(testBox);

	//create material // put in constants
	const D3DXVECTOR4 blue = D3DXVECTOR4(0.15f, 0.15f, 0.5f, 0.5f);
	const D3DXVECTOR4 black = D3DXVECTOR4(0.001f, 0.001f, 0.001f, 25.0f);
	const D3DXVECTOR4 zeroes = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 1.0f);

	DefaultMaterial* matBlue = new DefaultMaterial(black, zeroes);
	DefaultMaterial* matBlack = new DefaultMaterial(black, black);
	sceneToBuild->AddMaterial(matBlue);
	sceneToBuild->AddMaterial(matBlack);

	//add to scene
	sceneToBuild->CreateGeometryInstance(groundPlane, matBlack);
	sceneToBuild->CreateGeometryInstance(testBox, matBlue);

	// add and set camera
	CFirstPersonCamera* sceneCamera = new CFirstPersonCamera();
	sceneCamera->SetRotateButtons(true, false, false);
	sceneCamera->SetDrag(true);
	sceneCamera->SetEnableYAxisMovement(true);
	sceneCamera->SetEnablePositionMovement(true);
	sceneCamera->SetScalers(0.01f, 10.0f);

	sceneCamera->SetViewParams(&D3DXVECTOR3(-10.0f, 2.0f, -10.0f), &D3DXVECTOR3(0.0f, 4.0f, 0.0f));

	sceneToBuild->SetCamera(sceneCamera);

	// set sun constants ??
	sceneToBuild->SetSun(D3DXVECTOR3(400.0f, 800.0f, 400.0f), D3DXVECTOR3(0.25f, -0.5f, 0.25f), D3DXVECTOR4(1.0f,1.0f,1.0f,1.0f));

	// maybe lights, you know, to see stuff?


	return true;
}
