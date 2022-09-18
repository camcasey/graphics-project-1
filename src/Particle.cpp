//
// sueda - geometry edits Z. Wood
// 3/16
//

#include <iostream>
#include "Particle.h"
#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Texture.h"


float randFloat(float l, float h)
{
	float r = rand() / (float) RAND_MAX;
	return (1.0f - r) * l + r * h;
}

Particle::Particle(vec3 start) :
	charge(1.0f),
	m(1.0f),
	d(0.0f),
	x(start),
	v(0.0f, 0.0f, 0.0f),
	lifespan(1.0f),
	tEnd(0.0f),
	scale(100.0f),
	color(1.0f, 1.0f, 1.0f, 1.0f)
{
}

Particle::~Particle()
{
}

void Particle::load(vec3 start)
{
	// Random initialization
	rebirth(0.0f, start);
}

/* all particles born at the origin */
void Particle::rebirth(float t, vec3 start)
{
	charge = randFloat(0.0f, 1.0f) < 0.5 ? -1.0f : 1.0f;	
	m = 1.0f;
  	d = randFloat(0.0f, 10.02f);
	x = start + vec3(randFloat(-60.5f, -40.5f),randFloat(135.5f, 150.5f),randFloat(-210.5f, -200.5f));
	// x = start;

	v.x = randFloat(-0.27f, 0.3f);
	v.y = randFloat(-0.1f, 0.9f);
	v.z = randFloat(-0.3f, 0.27f);
	lifespan = randFloat(1.0f, 10.0f); 
	tEnd = t + lifespan;
	scale = randFloat(100.0f, 2000.0f);
   	color.r = randFloat(0.85f, 0.95f);
   	color.g = randFloat(0.30f, 0.40f);
   	color.b = randFloat(0.10f, 0.15f);
	color.a = 0.8f;
}

void Particle::update(float t, float h, const vec3 &g, const vec3 start)
{
	if(t > tEnd) {
		rebirth(t, start);
	}

	//very simple updatema
	x += h*((g*(7 - (tEnd - t)))+v);
	
	//To do - how do you want to update the forces?
	color.a = (tEnd-t)/lifespan - 0.8;
	//std::cout << "FFF: " << color.a << "\n";
}
