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
	BVHColor color;
	float indexRel;
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
	params->indexRel = 1.0f;

	return (void*)params;
}

extern "C" void setParam(void *params, const char *name, void *value)
{
	BVHShader_params *p = (BVHShader_params*)params;

	if (strcmp(name, "color") == 0)
		p->color = *((BVHColor*)value);
	else if (strcmp(name, "indexRel") == 0)
		p->indexRel = *((float*)value);
}

extern "C" void execute(void *params, BVHShader_functions *func, void *env)
{
	BVHShader_surface *surface = (BVHShader_surface*)env;
	BVHShader_params *p = (BVHShader_params*)params;

	float rayRefract[3];
	BVHColor colorRefraction;

	// Calculate refraction ray
	refract(surface->normal, surface->rayInput, rayRefract, p->indexRel);
	invert(rayRefract);
	normalize(rayRefract);
	normalize(surface->rayOutput);
	normalize(surface->normal);

	if (dotAbs(surface->rayOutput, rayRefract) > 0.9999f)
		colorRefraction.init(1.0f / dotAbs(surface->rayOutput, surface->normal));
	else
		colorRefraction.init(0.0f);

	surface->color = p->color * colorRefraction;
	surface->emission.init(0.0f);
}

extern "C" void getNormal(void *params, BVHShader_functions *func, void *env)
{
	BVHShader_surface *surface = (BVHShader_surface*)env;

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

extern "C" void getSampling(void *params, BVHShader_samplingList *samplingList)
{
	BVHShader_params *p = (BVHShader_params*)params;

	samplingList->samplingNumber = 1;
	samplingList->samplings[0].type = BVHSHADER_SAMPLING_REFRACTION;
	samplingList->samplings[0].pdf = 1.0f;
	samplingList->samplings[0].indexRel = p->indexRel;
}
