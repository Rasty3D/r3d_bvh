/*
 * emitter_sphere.cpp
 *
 *  Created on: 5 May 2011
 *      Author: showroom
 */

/*
 * INCLUDES
 */

#include <math.h>
#include "bvhshader.h"


/*
 * INTERNAL DATA
 */

	/* Parameters */

typedef struct
{
	float radius;
	float length;
}BVHShader_params;


/*
 * EXTERN FUNCTIONS
 */

extern "C" int getType()
{
	return BVHSHADER_TYPE_EMITTER;
}

extern "C" void *getParams()
{
	BVHShader_params *params = new BVHShader_params;
	params->radius = 1.0f;
	params->length = 1.0f;
	return (void*)params;
}

extern "C" void setParam(void *params, const char *name, void *value)
{
	BVHShader_params *p = (BVHShader_params*)params;
	
	if (strcmp(name, "radius") == 0)
		memcpy(&p->radius, value, sizeof(p->radius));
	else if (strcmp(name, "length") == 0)
		memcpy(&p->length, value, sizeof(p->length));
}

extern "C" void execute(void *params, BVHShader_functions *func, void *env)
{
	BVHShader_emitter *emitter = (BVHShader_emitter*)env;
	BVHShader_params *p = (BVHShader_params*)params;
	float pos;
	float angle;
	float module;
	
	// Select random position and angle;
	pos = p->length * (*func->rng)(emitter->rng, -0.5f, 0.5f);
	angle = (*func->rng)(emitter->rng, 0.0f, 6.28f);
	
	// Save normal
	emitter->normal[0] = 0.0f;
	emitter->normal[1] = cosf(angle);
	emitter->normal[2] = sinf(angle);
	
	// Generate random position in the sphere
	emitter->rayPos[0] = pos;
	emitter->rayPos[1] = p->radius * emitter->normal[1];
	emitter->rayPos[2] = p->radius * emitter->normal[2];
	
	// Generate random direction
	emitter->rayDir[0] = (*func->rng)(emitter->rng, -1.0f, 1.0f);
	emitter->rayDir[1] = (*func->rng)(emitter->rng, -1.0f, 1.0f);
	emitter->rayDir[2] = (*func->rng)(emitter->rng, -1.0f, 1.0f);
	module = 1.0f / sqrtf(
		emitter->rayDir[0] * emitter->rayDir[0] +
		emitter->rayDir[1] * emitter->rayDir[1] +
		emitter->rayDir[2] * emitter->rayDir[2]);
	emitter->rayDir[0] *= module;
	emitter->rayDir[1] *= module;
	emitter->rayDir[2] *= module;
	
	// Check direction
	module =
		emitter->rayPos[0] * emitter->rayDir[0] +
		emitter->rayPos[1] * emitter->rayDir[1] +
		emitter->rayPos[2] * emitter->rayDir[2];
		
	if (module < 0.0f)
	{
		emitter->rayDir[0] = -emitter->rayDir[0];
		emitter->rayDir[1] = -emitter->rayDir[1];
		emitter->rayDir[2] = -emitter->rayDir[2];
	}
}

