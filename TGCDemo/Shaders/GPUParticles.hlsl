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

Texture2D randomMap : register(t9);

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
GS_PARTICLE ParticleSystemVS( VS_PARTICLE input )
{
	GS_PARTICLE output = (GS_PARTICLE)0;
	
	output.position		= input.position;
	output.color		= input.color;
	output.direction	= input.direction;
	output.duration		= input.duration;
	output.speed		= input.speed;
	output.rotation		= input.rotation;
	output.size			= input.size;
	output.flags		= input.flags;
	
	return output;
}


//--------------------------------------------------------------------------------------
// Geometry Shader
//--------------------------------------------------------------------------------------
[maxvertexcount(1)]
void ParticleSystemGS( point GS_PARTICLE input[1], inout PointStream<GS_PARTICLE> outputStream )
{
	GS_PARTICLE output;
	output.position		= input[0].position;
	output.color		= input[0].color;
	output.direction	= input[0].direction;
	output.duration		= input[0].duration;
	output.speed		= input[0].speed;
	output.rotation		= input[0].rotation;
	output.size			= input[0].size;
	output.flags		= input[0].flags;

	bool alive = ( fmod(output.flags, 2) == PARTICLEALIVE );

	// if particle empty (duration <= 0 ) check if new ones need to be created (spawn rate), 
	//											else: skip
	float tempVal = (output.position.x+output.position.y+output.position.z)/3 + output.duration + output.speed;

	float randomVal = rand(float2(delta, tempVal));

	if(alive)
	{ 
		if(output.duration <= 0)
		{
			output.flags = 0; //TODO: append any other flags

			output.duration = spawnTime + (randomVal - 0.5) * spawnDeviation; // input random spawn wait with deviation.
			
			//put a new random val in there
			randomVal = rand(float2(randomVal, tempVal));	
		} 
		else
		{
			// move particle
			// change position / color / alpha according to formula / change method.
			// either type or different geometry shaders.
			output.position += float4(0,output.speed * delta,0,0);
		}
	}
	else if(output.duration <= 0 ) // dead and spawn wait over.
	{
		output.flags = PARTICLEALIVE; //TODO: append any other flags

		output.duration = durationTime + (randomVal - 0.5) * durationDeviation;
		//put a new random val in there
		randomVal = rand(float2(randomVal, tempVal));

		output.position = positionStart + (randomVal - 0.5) * positionDeviation;
		//put a new random val in there
		randomVal = rand(float2(randomVal, tempVal));

		output.speed = speed + (randomVal - 0.5) * speedDeviation;

	}
		
	// decrease time stuffs
	output.duration -= delta;
	
	//TODO compute and output a quad from given position and drawSize
	//upper left
	//output.position = input[0].position;
	//output.duration = input[0].duration - delta;

	outputStream.Append( output );

}


// no pixel shader, just computes particles. 
// particles are rendered in a seperate shader ( generating a quad per particle)
#endif // PARTICLESYSTEM_HLSL