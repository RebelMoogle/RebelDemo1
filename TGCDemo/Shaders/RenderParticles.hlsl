/*****************************
* GPU Based Particle System using Swap Buffers.
*	- Input:	Initial Particle Buffer / Previous
				Particle settings / ranges (life, spawn rate, max count( = buffer size ), Force, Size			
*
*
******************************/

#ifndef RENDERPARTICLES_HLSL
#define RENDERPARTICLES_HLSL

#include "ShaderConstants.hlsl"
//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
Texture2D<float4> particleTex : register(t1);


// Need Depth and Blend States?

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
GS_PARTICLE RenderParticlesVS( VS_PARTICLE input )
{
	GS_PARTICLE output = (GS_PARTICLE)0;

	output.position		= mul(float4(input.position.xyz, 1.0), WorldViewProj);
	// move to screen space
	
	//output.position -= float4(size/2.0, size/2.0, 0, 0); // move vertex to upper left of quad.
	
	output.color		= input.color;
	output.direction	= input.direction;
	output.duration		= input.duration;
	output.speed		= input.speed;
	output.rotation		= input.rotation;
	output.size			= input.size; //input.size;
	output.flags		= input.flags;
	
	return output;
}


//--------------------------------------------------------------------------------------
// Geometry Shader
//--------------------------------------------------------------------------------------
[maxvertexcount(4)]
void RenderParticlesGS( point GS_PARTICLE input[1], inout TriangleStream<PS_PARTICLE> TriStream )
{
	//check if particle is alive, if dead: do not append; return.
	if(input[0].flags <= 0) //fmod(input[0].flags, 2) != PARTICLEALIVE)
		return;

	//fill the output, so we have everything.
	PS_PARTICLE output;
	output.color		= input[0].color;
	output.direction	= input[0].direction;
	output.duration		= input[0].duration;
	output.speed		= input[0].speed;
	output.rotation		= input[0].rotation;
	output.size			= input[0].size;
	output.flags		= input[0].flags;


	output.position = input[0].position + float4(output.size, output.size, 0, 0); // (x,y) + (size,size)
	output.Tex = float2(1, 0);
	TriStream.Append( output );

	output.position = input[0].position + float4(output.size, 0, 0, 0); // x + size 
	output.Tex = float2(1, 1);
	TriStream.Append( output );

	output.position = input[0].position + float4(0, output.size, 0, 0); // y + size
	output.Tex = float2(0.0, 0.0);
	TriStream.Append( output );

	output.position = input[0].position;
	output.Tex = float2(0,1);
	TriStream.Append( output );

	TriStream.RestartStrip();

}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 RenderParticlesPS( PS_PARTICLE input) : SV_Target
{

	//return float4(input.Tex, 0, 1);
	//TODO distribute values equally (histogram)
	float4 color = particleTex.Sample(defaultSampler, input.Tex) * input.color;

	return color; //input.color.a);
}

#endif // RENDERPARTICLES_HLSL