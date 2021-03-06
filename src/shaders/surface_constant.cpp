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

BVHColor colorConstant;
//void *texture;
//BVHShader texShader;


/*
 * EXTERN FUNCTIONS
 */

extern "C" int getType()
{
	return BVHSHADER_TYPE_SURFACE;
}

extern "C" void setParam(const char *name, void *value)
{
	if (strcmp(name, "colorConstant") == 0)
		colorConstant = *((BVHColor*)value);
	/*
	else if (strcmp(name, "texture") == 0)
		texture = value;
	else if (strcmp(name, "texShader") == 0)
		texShader = *((BVHShader*)value);*/
}

extern "C" void execute(void *env)
{
	BVHShader_surface *surface = (BVHShader_surface*)env;

/*
	surface->color = BVHShader::getColorUV(texture, 0.0f, 0.0f);

	BVHShader_texture texEnv;
	texEnv.coords[0] = 0.0f;
	texEnv.coords[1] = 0.0f;
	texShader.execute(&texEnv);
	surface->color = texEnv.color;*/

	surface->color = colorConstant;
}
