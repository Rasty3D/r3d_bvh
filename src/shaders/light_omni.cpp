/*
 * light_omni.cpp
 *
 *  Created on: 10 May 2011
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
	return BVHSHADER_TYPE_LIGHT;
}

extern "C" void *getParams()
{
	BVHShader_params *params = new BVHShader_params;
	params->color.init(1.0);
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
	BVHShader_light *light = (BVHShader_light*)env;
	BVHShader_params *p = (BVHShader_params*)params;
			
	light->color = p->color * p->intensity;
}

