/*
 * surface_constant.cpp
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
	BVHColor color;
	float intensity;
}BVHShader_params;


/*
 * EXTERN FUNCTIONS
 */

extern "C" int getType()
{
	return BVHSHADER_TYPE_SURFACE;
}

extern "C" void *getParams()
{
	BVHShader_params *params = new BVHShader_params;

	// Init params
	params->color.init(1.0f);
	params->intensity = 1.0f;

	return (void*)params;
}

extern "C" void setParam(void *params, const char *name, void *value)
{
	BVHShader_params *p = (BVHShader_params*)params;
	
	if (strcmp(name, "color") == 0)
		p->color = *((BVHColor*)value);
	else if (strcmp(name, "intensity") == 0)
		p->intensity = *((float*)value);
}

extern "C" void execute(void *params, BVHShader_functions *func, void *env)
{
	BVHShader_surface *surface = (BVHShader_surface*)env;
	BVHShader_params *p = (BVHShader_params*)params;
	surface->color.init(0.0f);
	surface->emission = p->color * p->intensity;
}
