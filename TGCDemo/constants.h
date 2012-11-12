#pragma once
// here are some constants

#define CAMERA_NEAR 0.1f
#define CAMERA_FAR 5000.0f

#define RefractIdx_Vacuum 1.0f
#define RefractIdx_Air 1.000292f
#define RefractIdx_Ice 1.31f
#define RefractIdx_Water 1.33f
#define RefractIdx_FusedQuartz 1.46f
#define RefractIdx_Glass 2.0f
#define RefractIdx_Diamond 2.42f

#define SPECULAR_EXP 1.0f

// PARTICLE FLAGS
#define PARTICLE_ALIVE 1


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

struct CameraConstants
{
	D3DXMATRIX World; // object to world (of light?)
	D3DXMATRIX View;
	D3DXMATRIX WorldViewProjection;
	D3DXVECTOR3 cameraPosition;
	float refractiveIndexETA;
};

struct LightConstants
{
	//D3DXMATRIX	lightViewProj;
	//D3DXMATRIX  shadowToScreen;
	D3DXVECTOR4	lightPower;
	D3DXVECTOR4 lightPositionAngle; //contains distance in w // if w is < 0.0f it is the sun!
	D3DXVECTOR4 lightDirectionDistance; // normalized; contains Distance in w
};

__declspec(align(16))
struct TextureToScreenConstants
{
	D3DXVECTOR4 mDestRect; 
};

__declspec(align(16))
struct CraftMyGBufferConstants
{
	D3DXVECTOR4 diffuseConstant;
	D3DXVECTOR4 specularConstant;
	D3DXVECTOR3 transmissiveConstant;
};

struct geometryConstants
{
	D3DXMATRIX objToWorld;
};