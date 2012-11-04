#pragma once
// here are some constants

#define CAMERA_NEAR 1.0f
#define CAMERA_FAR 2000.0f

#define RefractIdx_Vacuum 1.0f
#define RefractIdx_Air 1.000292f
#define RefractIdx_Ice 1.31f
#define RefractIdx_Water 1.33f
#define RefractIdx_FusedQuartz 1.46f
#define RefractIdx_Glass 2.0f
#define RefractIdx_Diamond 2.42f

struct PositionTexCoordVertex
{
	D3DXVECTOR3 position;
	D3DXVECTOR2 TexCoord;
};

struct PositionNormalTexCoordVertex
{
	D3DXVECTOR3 position;
	D3DXVECTOR3 normal;
	D3DXVECTOR2 TexCoord;
};

__declspec(align(16))
struct CameraConstants
{
	D3DXMATRIX World; // object to world (of light?)
	D3DXMATRIX View;
	D3DXMATRIX WorldViewProjection;
	D3DXVECTOR3 cameraPosition;
	float refractiveIndexETA;
};
