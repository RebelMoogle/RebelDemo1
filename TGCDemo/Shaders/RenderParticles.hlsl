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
PS_PARTICLE RenderParticlesVS( VS_PARTICLE input )
{
	PS_PARTICLE output = (PS_PARTICLE)0;

	output.position		= mul(float4(0,0,0,1), WorldViewProj);//mul(float4(input.position.xyz, 1.0), WorldViewProj);
	// move to screen space
	
	//output.position -= float4(size/2.0, size/2.0, 0, 0); // move vertex to upper left of quad.
	
	output.color		= input.color;
	output.direction	= input.direction;
	output.duration		= input.duration;
	output.speed		= input.speed;
	output.rotation		= input.rotation;
	output.size			= 50.0; //input.size;
	output.flags		= input.flags;
	
	return output;
}


//--------------------------------------------------------------------------------------
// Geometry Shader
//--------------------------------------------------------------------------------------
[maxvertexcount(1)]
void RenderParticlesGS( point PS_PARTICLE input[1], inout TriangleStream<PS_PARTICLE> TriStream )
{
	////check if particle is alive, if dead: do not append; return.
	//if(fmod(input[0].flags, 2) != PARTICLEALIVE)
	//	return;

	//fill the output, so we have everything.
	PS_PARTICLE output;
	output.color		= input[0].color;
	output.direction	= input[0].direction;
	output.duration		= input[0].duration;
	output.speed		= input[0].speed;
	output.rotation		= input[0].rotation;
	output.size			= input[0].size;
	output.flags		= input[0].flags;


	// we should already be in ScreenSpace and position should be upper left of quad.
	//TODO compute and output a quad from given position and drawSize
	//upper left
	output.position = input[0].position;
	output.Tex = float2(0,0);
	TriStream.Append( output );

	//upper right
	output.position = input[0].position + float4(output.size, 0, 0, 0); // x + size 
	output.Tex = float2(1.0, 0);
	TriStream.Append( output );

	//lower left
	output.position = input[0].position + float4(0, output.size, 0, 0); // y + size
	output.Tex = float2(0.0, 1.0);
	TriStream.Append( output );

	//lower right
	output.position = input[0].position + float4(output.size, output.size, 0, 0); // (x,y) + (size,size)
	output.Tex = float2(1.0, 1.0);
	TriStream.Append( output );

	TriStream.RestartStrip();

}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 RenderParticlesPS( PS_PARTICLE input) : SV_Target
{

	return float4(1, 0, 0, 1);
	//TODO distribute values equally (histogram)
	float3 color = particleTex.Sample(defaultSampler, input.Tex).rgb * input.color.rgb;
	if(length(color) > 3.0f)
		color /= 1000.0f; 
	return float4(color, input.color.a);
}

#endif // RENDERPARTICLES_HLSL