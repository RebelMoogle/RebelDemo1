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
Texture2D particleTex : register(t1);


// Need Depth and Blend States?

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_PARTICLE ParticleSystemVS( VS_PARTICLE input )
{
	PS_PARTICLE output = (PS_PARTICLE)0;
	
	output.position = input.position; // make sure we're in screenspace after this
	
	//output.position -= float4(size/2.0, size/2.0, 0, 0); // move vertex to upper left of quad.
	
	output.duration = input.duration;
	
	return output;
}


//--------------------------------------------------------------------------------------
// Geometry Shader
//--------------------------------------------------------------------------------------
[maxvertexcount(1)]
void ParticleSystemGS( point PS_PARTICLE input[1], inout TriangleStream<PS_PARTICLE> TriStream )
{
	PS_PARTICLE output;

	// we should already be in ScreenSpace and position should be upper left of quad.
	
	//TODO compute and output a quad from given position and drawSize
	//upper left
	output.position = input[0].Pos;
	output.Tex = input[0].Tex;
	output.duration = input.duration;
	TriStream.Append( output );

	//upper right
	output.position = input[0].Pos + float4(1.0, 0, 0, 0); // x + size 
	output.Tex = input[0].Tex + float2(1.0, 0);
	output.duration = input.duration;
	TriStream.Append( output );

	//lower left
	output.position = input[0].Pos + float4(0, 1.0, 0, 0); // y + size
	output.Tex = input[0].Tex + float2(0.0, 1.0);
	output.duration = input.duration;
	TriStream.Append( output );

	//lower right
	output.position = input[0].Pos + float4(1.0, 1.0, 0, 0); // (x,y) + (size,size)
	output.Tex = input[0].Tex + float2(1.0, 1.0);
	output.duration = input.duration;
	TriStream.Append( output );

	TriStream.RestartStrip();

}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 TextureToScreenPS( PS_PARTICLE input) : SV_Target
{

	//return float4(1, 0, 0, 1);
	//TODO distribute values equally (histogram)
	float3 color = particleTex.Sample(defaultSampler, input.Tex).rgb;
	if(length(color) > 3.0f)
		color /= 1000.0f; 
	return float4(color, clamp(input.duration/maxDuration, 0.0, 1.0));
}

#endif // RENDERPARTICLES_HLSL