#pragma once
/******************
	Base Particle System
	 - base functions. 
	 - create particles system by inheriting. 

		- change update class.

	- Since the Particles are computed on the GPU, not much info is needed (no single particles, etc.)
		- need different particle shader for every different system?
		- Particles created and updated on GPU
		- Particles die -> particles reset (or just not drawn)
		- min, max, death rate, lifetime, changerate, speed are defined in the class and sent to the shader.

*/

class ParticleSystem
{
public:
	ParticleSystem(void);
	~ParticleSystem(void);

	void Update();

	void Render();
};

