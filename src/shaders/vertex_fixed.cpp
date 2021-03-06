/*
 * vertex_fixed.cpp
 *
 *  Created on: 5 May 2011
 *      Author: showroom
 */

/*
 * INCLUDES
 */

#include <math.h>
#include "bvhshader.h"
#include "bvhtransform.h"


/*
 * INTERNAL DATA
 */

	/* Parameters */


/*
 * EXTERN FUNCTIONS
 */

extern "C" int getType()
{
	return BVHSHADER_TYPE_VERTEX;
}

extern "C" void *getParams()
{
	return NULL;
}

extern "C" void setParam(void *params, const char *name, void *value)
{
	return;
}

extern "C" void execute(void *params, BVHShader_functions *func, void *env)
{
	BVHShader_vertex *vertex = (BVHShader_vertex*)env;
	BVHAttribAtlas *attribAtlas = vertex->attribAtlas;
	
	for (int i = 0; i < attribAtlas->number; i++)
	{
		if (strcmp(attribAtlas->attrList[i].name, "P") == 0)
		{
			vertex->transform.transformVector(
				(float*)&vertex->attribBuffer[attribAtlas->attrList[i].offset],
				(float*)&vertex->newAttribBuffer[attribAtlas->attrList[i].offset]);			
		}
		else if (strcmp(attribAtlas->attrList[i].name, "N") == 0)
		{
			vertex->transform.rotateVector(
				(float*)&vertex->attribBuffer[attribAtlas->attrList[i].offset],
				(float*)&vertex->newAttribBuffer[attribAtlas->attrList[i].offset]);
		}
		else
		{
			memcpy(
				&vertex->newAttribBuffer[attribAtlas->attrList[i].offset],
				&vertex->attribBuffer[attribAtlas->attrList[i].offset],
				attribAtlas->attrList[i].size);
		}
	}
}

