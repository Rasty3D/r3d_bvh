/*
 * jitter2d_sqrand.cpp
 *
 *  Created on: 25 Feb 2011
 *      Author: showroom
 */

/*
 * INCLUDES
 */

#include "bvhshader.h"


/*
 * INTERNAL DATA
 */

	/* Parameters */

float amplitude = 1.0f;


	/* Internal functions */

float randFloat(float min, float max)
{
	return min + (max - min) * (float)rand() / (float)RAND_MAX;
}


/*
 * EXTERN FUNCTIONS
 */

extern "C" int getType()
{
	return BVHSHADER_TYPE_JITTER_2D;
}

extern "C" void setParam(const char *name, void *value)
{
	if (strcmp(name, "amplitude") == 0)
		memcpy(&amplitude, value, sizeof(amplitude));
}

extern "C" void execute(void *env)
{
	BVHShader_jitter2d *jitter2d = (BVHShader_jitter2d*)env;

	jitter2d->dx =
		(float)(jitter2d->kernelIdx[0] * 2 + 1 - jitter2d->kernelDim[0]) /
		(float)jitter2d->kernelDim[0];
	jitter2d->dy =
		(float)(jitter2d->kernelIdx[1] * 2 + 1 - jitter2d->kernelDim[1]) /
		(float)jitter2d->kernelDim[1];

	jitter2d->dx += amplitude * randFloat(-1.0f, 1.0f) / (float)jitter2d->kernelDim[0];
	jitter2d->dy += amplitude * randFloat(-1.0f, 1.0f) / (float)jitter2d->kernelDim[1];
}
