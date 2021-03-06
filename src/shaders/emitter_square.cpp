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
	float width;
	float height;
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
	params->width = 1.0f;
	params->height = 1.0f;
	return (void*)params;
}

extern "C" void setParam(void *params, const char *name, void *value)
{
	BVHShader_params *p = (BVHShader_params*)params;
	
	if (strcmp(name, "width") == 0)
		memcpy(&p->width, value, sizeof(p->width));
	else if (strcmp(name, "height") == 0)
		memcpy(&p->height, value, sizeof(p->height));
}

extern "C" void execute(void *params, BVHShader_functions *func, void *env)
{
	BVHShader_emitter *emitter = (BVHShader_emitter*)env;
	BVHShader_params *p = (BVHShader_params*)params;
	float module;
	
	// Generate random position in the square
	emitter->rayPos[0] = p->width * (*func->rng)(emitter->rng, -0.5f, 0.5f);
	emitter->rayPos[1] = p->height * (*func->rng)(emitter->rng, -0.5f, 0.5f);
	emitter->rayPos[2] = 0.0f;
	
	// Generate random direction
	emitter->rayDir[0] = (*func->rng)(emitter->rng, -1.0f, 1.0f);
	emitter->rayDir[1] = (*func->rng)(emitter->rng, -1.0f, 1.0f);
	emitter->rayDir[2] = (*func->rng)(emitter->rng, -1.0f, 0.0f);
	module = sqrtf(
		emitter->rayDir[0] * emitter->rayDir[0] +
		emitter->rayDir[1] * emitter->rayDir[1] +
		emitter->rayDir[2] * emitter->rayDir[2]);
	emitter->rayDir[0] /= module;
	emitter->rayDir[1] /= module;
	emitter->rayDir[2] /= module;
	
	// Save normal
	emitter->normal[0] = 0.0f;
	emitter->normal[1] = 0.0f;
	emitter->normal[2] = -1.0f;
}

