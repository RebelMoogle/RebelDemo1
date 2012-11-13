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
	output.initialRandom = input.initialRandom;
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
	GS_PARTICLE output	= (GS_PARTICLE)0;
	output.position		= input[0].position;
	output.initialRandom = input[0].initialRandom;
	output.color		= input[0].color;
	output.direction	= input[0].direction;
	output.duration		= input[0].duration;
	output.speed		= input[0].speed;
	output.rotation		= input[0].rotation;
	output.size			= input[0].size;
	output.flags		= input[0].flags;

	// don't have any other flags yet, so this is faster.
	bool alive = ( output.flags > 0 ); //fmod(input[0].flags, 2) == PARTICLEALIVE) 

	// if particle empty (duration <= 0 ) check if new ones need to be created (spawn rate), 
	//											else: skip

	float tempVal = (output.position.x + output.position.y + output.position.z)/3 + output.speed + Delta;// + (output.color.x + output.color.y + output.color.z)/3;

	float randomVal = rand(float2(Delta, tempVal));

	if(alive)
	{ 
		if(output.duration <= 0)
		{
			output.flags = 0; //TODO: append any other flags

			output.duration = SpawnTime + (randomVal - 0.5) * SpawnDeviation; // input random spawn wait with deviation.
			
			//put a new random val in there
			randomVal = rand(float2(randomVal,  output.initialRandom.x));	
		} 

		float lifeLeft = output.duration/DurationTime;

		// use smoothstep if not smooth enough.

		if(lifeLeft > 0.5)
			output.color = lifeLeft * float4(0.001,0.001,0.001,0.001) + (1.0 - lifeLeft) * ColorStart;
		else
			output.color = lifeLeft * ColorStart + (1.0 - lifeLeft) * ColorEnd;
		
		// move particle
		// change position / color / alpha according to formula / change method.
		//output.color.rgb = ;
		// either type or different geometry shaders.
		
		// if completed about 1 square, turn
		 
		float3 towardsCenter = (DirectionDeviation.xyz - output.position.xyz);
		output.position.xyz = output.position.xyz + output.direction.xyz * (lifeLeft * output.speed * Delta) * max((1.0 - CenterGravity), 0);
		output.position.xyz = output.position.xyz + towardsCenter * (output.speed * Delta) * CenterGravity;

		if(CenterGravity > 0)
		{
			output.direction += (1.0 - lifeLeft) * cross(towardsCenter, float3(0,1,0) * Delta);
			//output.position.xyz += (1.0 - lifeLeft) * float3(0, towardsCenter.y, 0) * Delta;
			//output.color = lifeLeft * ColorStart + (1.0 - lifeLeft) * float4(0.8, 0, 0.3, 1.0);
		}	
		//output.position.xyz = output.position.xyz + lifeLeft * float3(-towardsCenter.x, 0, -towardsCenter.z) * (output.speed * Delta) * CenterGravity;

	}
	else if(output.duration <= 0 ) // dead and spawn wait over.
	{
		// SPAWN NEW PARTICLE!
		output.flags = PARTICLEALIVE; //TODO: append any other flags

		output.duration = DurationTime + (randomVal - 0.5) * DurationDeviation;
		//put a new random val in there
		randomVal = rand(float2(randomVal, output.initialRandom.y));

		output.color = float4(0.001,0.001,0.001,1);
		//put a new random val in there
		randomVal = rand(float2(randomVal, Delta));

		
		output.speed = SpeedStart + (randomVal - 0.5) * SpeedDeviation;

		//position
		output.position.x = PositionStart.x + floor((randomVal - 0.5) * PositionDeviation.x) + 0.5;
		//put a new random val in there
		randomVal = rand(float2(randomVal, Delta));

		output.position.y = PositionStart.y + floor((randomVal - 0.5) * PositionDeviation.y) + 0.5;
		//put a new random val in there
		randomVal = rand(float2(randomVal, Delta));

		output.position.z = PositionStart.z + floor((randomVal - 0.5) * PositionDeviation.z) + 0.5;
		//put a new random val in there
		randomVal = rand(float2(randomVal, tempVal));

		//direction
		output.direction = DirectionStart;
		//randomVal = rand(float2(randomVal, Delta));
		output.direction.y = 0; //directionStart.y + (randomVal -0.5) * directionDeviation.y;
		//randomVal = rand(float2(randomVal, delta));
		output.direction.z = 0; //directionStart.z + (randomVal -0.5) * directionDeviation.z;
		//randomVal = rand(float2(randomVal, delta));

		output.size = SizeStart + (randomVal -0.5) * SizeDeviation;

	}
		
	// decrease time stuffs
	output.duration = output.duration - Delta;
	
	//TODO compute and output a quad from given position and drawSize
	//upper left
	//output.position = input[0].position;
	//output.duration = input[0].duration - delta;

	outputStream.Append( output );

}


// no pixel shader, just computes particles. 
// particles are rendered in a seperate shader ( generating a quad per particle)
#endif // PARTICLESYSTEM_HLSL