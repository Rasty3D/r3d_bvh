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
#include <iostream>
#include "bvhshader.h"


/*
 * INTERNAL DATA
 */

	/* Parameters */

typedef struct
{
	void *texDiffuse;
	float specularExponent;
	BVHColor specularColor;
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
	params->texDiffuse = NULL;
	params->specularExponent = 1.0f;
	params->specularColor.init(1.0f);

	return (void*)params;
}

extern "C" void setParam(void *params, const char *name, void *value)
{
	BVHShader_params *p = (BVHShader_params*)params;
	
	if (strcmp(name, "texDiffuse") == 0)
	{
		p->texDiffuse = value;
	}
	else if (strcmp(name, "specularExponent") == 0)
	{
		p->specularExponent = *((float*)value);
	}
	else if (strcmp(name, "specularColor") == 0)
	{
		p->specularColor = *((BVHColor*)value);
	}
}

extern "C" void execute(void *params, BVHShader_functions *func, void *env)
{
	BVHShader_surface *surface = (BVHShader_surface*)env;
	BVHShader_params *p = (BVHShader_params*)params;

	float directionInput;
	float directionOutput;
	BVHColor colorDiffuse;
	float texCoords[2] = {0.0f, 0.0f};
	float rayReflect[3];
	float specular;
	BVHColor colorSpecular;
	
	// Get attributes
	for (int i = 0; i < surface->attribAtlas->number; i++)
	{
		if (strcmp(surface->attribAtlas->attrList[i].name, "T") == 0)
		{
			texCoords[0] = ((float*)&surface->attribBuffer[surface->attribAtlas->attrList[i].offset])[0];
			texCoords[1] = ((float*)&surface->attribBuffer[surface->attribAtlas->attrList[i].offset])[1];
		}
	}
	
	// Calculate signs	
	directionInput = dot(surface->normal, surface->rayInput);
	directionOutput = dot(surface->normal, surface->rayOutput);

	if (directionInput * directionOutput < 0.0f)
	{
		surface->color.init(0.0f);
	}
	else
	{
		// Calculate reflection ray
		reflect(surface->normal, surface->rayInput, rayReflect);
		invert(rayReflect);
		normalize(rayReflect);
		normalize(surface->rayOutput);

		// Calculate specular contribution
		specular = powf(dotAbs(rayReflect, surface->rayOutput), p->specularExponent) * (1.0f + p->specularExponent) / (2.0f * PI);

		colorSpecular = p->specularColor * specular;
		//colorDiffuse = func->getColorUV(p->texDiffuse, texCoords[0], texCoords[1]);
		colorDiffuse = func->getColorUV(p->texDiffuse, texCoords[0] * 0.1f, texCoords[1] * 0.1f);

		//surface->color = colorDiffuse + colorSpecular;
		surface->color = colorDiffuse;
	}
	
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

extern "C" void getSampling(void *params, BVHShader_samplingList *samplingList)
{
	BVHShader_params *p = (BVHShader_params*)params;

	samplingList->samplingNumber = 1;
	samplingList->samplings[0].type = BVHSHADER_SAMPLING_DIFFUSE;
	samplingList->samplings[0].pdf = 1.0f;
	samplingList->samplings[1].type = BVHSHADER_SAMPLING_SPECULAR;
	samplingList->samplings[1].pdf = 1.0f;
	samplingList->samplings[1].exponent = p->specularExponent;
}
