/*
 * surface_lambert.cpp
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

	float directionInput;
	float directionOutput;

	// Calculate signs
	directionInput = dot(surface->normal, surface->rayInput);
	directionOutput = dot(surface->normal, surface->rayOutput);

	if (directionInput * directionOutput < 0.0f)
		surface->color.init(0.0f);
	else
		surface->color = p->colorDiffuse;

	surface->emission.init(0.0f);
}

extern "C" void getNormal(void *params, BVHShader_functions *func, void *env)
{
	BVHShader_surface *surface = (BVHShader_surface*)env;
	//BVHShader_params *p = (BVHShader_params*)params;

	// Get attributes
	for (int i = 0; i < surface->attribAtlas->number; i++)
	{
		if (strcmp(surface->attribAtlas->attrList[i].name, "N") == 0)
		{
			initVector(surface->normal,
				((float*)&surface->attribBuffer[surface->attribAtlas->attrList[i].offset])[0],
				((float*)&surface->attribBuffer[surface->attribAtlas->attrList[i].offset])[1],
				((float*)&surface->attribBuffer[surface->attribAtlas->attrList[i].offset])[2]);
			normalize(surface->normal);
		}
	}
}
