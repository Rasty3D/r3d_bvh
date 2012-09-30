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
	BVHColor colorDiffuse;
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
	return (void*)params;
}

extern "C" void setParam(void *params, const char *name, void *value)
{
	BVHShader_params *p = (BVHShader_params*)params;
	
	if (strcmp(name, "colorDiffuse") == 0)
		p->colorDiffuse = *((BVHColor*)value);
}

extern "C" void execute(void *params, BVHShader_functions *func, void *env)
{
	BVHShader_surface *surface = (BVHShader_surface*)env;
	BVHShader_params *p = (BVHShader_params*)params;

	float module;
	float directionInput;
	float directionOutput;
	float normal[3] = {surface->normal[0], surface->normal[1], surface->normal[2]};
	
	// Get attributes
	for (int i = 0; i < surface->attribAtlas->number; i++)
	{
		if (strcmp(surface->attribAtlas->attrList[i].name, "N") == 0)
		{
			normal[0] = ((float*)&surface->attribBuffer[surface->attribAtlas->attrList[i].offset])[0];
			normal[1] = ((float*)&surface->attribBuffer[surface->attribAtlas->attrList[i].offset])[1];
			normal[2] = ((float*)&surface->attribBuffer[surface->attribAtlas->attrList[i].offset])[2];
			module = sqrtf(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
			normal[0] /= module;
			normal[1] /= module;
			normal[2] /= module;
		}
	}
	
	// Normalize output direction
	module = sqrtf(
		surface->rayOutput[0] * surface->rayOutput[0] +
		surface->rayOutput[1] * surface->rayOutput[1] +
		surface->rayOutput[2] * surface->rayOutput[2]);
	surface->rayOutput[0] /= module;
	surface->rayOutput[1] /= module;
	surface->rayOutput[2] /= module;

	// Calculate signs	
	directionInput =
		normal[0] * surface->rayInput[0] +
		normal[1] * surface->rayInput[1] +
		normal[2] * surface->rayInput[2];

	directionOutput =
		normal[0] * surface->rayOutput[0] +
		normal[1] * surface->rayOutput[1] +
		normal[2] * surface->rayOutput[2];

	if (directionInput * directionOutput < 0.0f)
		surface->color.init(0.0f);
	else
		surface->color.init(fabsf(directionOutput));

	surface->color *= p->colorDiffuse;
}
