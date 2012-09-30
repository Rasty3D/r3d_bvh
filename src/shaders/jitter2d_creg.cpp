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

	float rmin = sqrtf((float)jitter2d->kernelIdx[0] / (float)jitter2d->kernelDim[0]);
	float rmax = sqrtf((float)(jitter2d->kernelIdx[0] + 1) / (float)jitter2d->kernelDim[0]);
	float rrand = rmin + (rmax - rmin) * (0.5f + 0.5f * randFloat(-amplitude, amplitude));
	float amin = 6.283185308f * (float)jitter2d->kernelIdx[1] / (float)jitter2d->kernelDim[1];
	float amax = amin + 6.283185308f / (float)jitter2d->kernelDim[1];
	float arand = amin + (amax - amin) * (0.5f + 0.5f * randFloat(-amplitude, amplitude));

	jitter2d->dx = rrand * sinf(arand);
	jitter2d->dy = rrand * cosf(arand);
}
