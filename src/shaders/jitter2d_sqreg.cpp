#include "bvhshader.h"

extern "C" int getType()
{
	return BVHSHADER_TYPE_JITTER_2D;
}

extern "C" void setParam(const char *name, void *value)
{
	return;
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
}
