/*
 * jitter2d_circular.cpp
 *
 *  Created on: 25 Feb 2011
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
	float amplitude;
}BVHShader_params;


/*
 * EXTERN FUNCTIONS
 */

extern "C" int getType()
{
	return BVHSHADER_TYPE_JITTER_2D;
}

extern "C" void *getParams()
{
	BVHShader_params *params = new BVHShader_params;
	params->amplitude = 1.0f;
	return (void*)params;
}

extern "C" void setParam(void *params, const char *name, void *value)
{
	BVHShader_params *p = (BVHShader_params*)params;
	
	if (strcmp(name, "amplitude") == 0)
		memcpy(&p->amplitude, value, sizeof(p->amplitude));
}

extern "C" void execute(void *params, BVHShader_functions *func, void *env)
{
	BVHShader_jitter2d *jitter2d = (BVHShader_jitter2d*)env;
	BVHShader_params *p = (BVHShader_params*)params;

	float radius;

	do
	{
		jitter2d->dx = (*func->rng)(jitter2d->rng, -1.0f, 1.0f);
		jitter2d->dy = (*func->rng)(jitter2d->rng, -1.0f, 1.0f);
		radius = jitter2d->dx * jitter2d->dx + jitter2d->dy * jitter2d->dy;
	}while (radius > 1.0f);

	jitter2d->dx *= p->amplitude;
	jitter2d->dy *= p->amplitude;
}
