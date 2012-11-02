/*****************************
* GPU Based Particle System using Swap Buffers.
*	- Input:	Initial Particle Buffer / Previous
				Particle settings / ranges (life, spawn rate, max count( = buffer size ), Force, Size			
*
*
******************************/

#ifndef PARTICLESYSTEM_HLSL
#define PARTICLESYSTEM_HLSL

#include "ShaderConstants.hlsl"
//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//-------------------------------------------------------------------------------------

struct VS_PARTICLE
{
	float3 position          : POSITION;
	float duration			 : DURATION;   
};

struct GS_PARTICLE
{
	float3 position	: SV_POSITION;
	float  duration	: DURATION;
};

// Need Depth and Blend States?

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
GS_PARTICLE ParticleSystemVS( VS_PARTICLE input )
{
	GS_PARTICLE output = (GS_PARTICLE)0;
	
	output.position = input.position; // *WorldViewProjectionMatrix!
	output.duration = input.duration;
	
	return output;
}


//--------------------------------------------------------------------------------------
// Geometry Shader
//--------------------------------------------------------------------------------------
[maxvertexcount(1)]
void ParticleSystemGS( point GS_PARTICLE input[1], inout PointStream<GS_PARTICLE> outputStream )
{
	GS_PARTICLE output;

	// if particle empty (duration <= 0 ) check if new ones need to be created (spawn rate), 
	//											else: skip
	
	//TODO compute and output a quad from given position and drawSize
	//upper left
	output.position = input[0].position;
	output.duration = input[0].duration - delta;

	outputStream.Append( output );

}


// no pixel shader, just computes particles. 
// particles are rendered in a seperate shader ( generating a quad per particle)
#endif // PARTICLESYSTEM_HLSL