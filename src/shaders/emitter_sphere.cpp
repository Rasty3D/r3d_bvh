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
	return (void*)params;
}

extern "C" void setParam(void *params, const char *name, void *value)
{
	BVHShader_params *p = (BVHShader_params*)params;
	
	if (strcmp(name, "radius") == 0)
		memcpy(&p->radius, value, sizeof(p->radius));
}

extern "C" void execute(void *params, BVHShader_functions *func, void *env)
{
	BVHShader_emitter *emitter = (BVHShader_emitter*)env;
	BVHShader_params *p = (BVHShader_params*)params;
	float module;
	
	// Generate random position in the sphere
	emitter->rayPos[0] = (*func->rng)(emitter->rng, -1.0f, 1.0f);
	emitter->rayPos[1] = (*func->rng)(emitter->rng, -1.0f, 1.0f);
	emitter->rayPos[2] = (*func->rng)(emitter->rng, -1.0f, 1.0f);
	module = p->radius / sqrtf(
		emitter->rayPos[0] * emitter->rayPos[0] +
		emitter->rayPos[1] * emitter->rayPos[1] +
		emitter->rayPos[2] * emitter->rayPos[2]);
	emitter->rayPos[0] *= module;
	emitter->rayPos[1] *= module;
	emitter->rayPos[2] *= module;
	
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
	
	// Save normal
	emitter->normal[0] = emitter->rayPos[0];
	emitter->normal[1] = emitter->rayPos[1];
	emitter->normal[2] = emitter->rayPos[2];
}

