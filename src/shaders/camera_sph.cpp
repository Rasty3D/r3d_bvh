/*
 * camera_rect.cpp
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
	float filmSize;			// In mm
	float focalLength;		// In mm
	float aperture;			// In F stops
	float focalDistance;	// In m
}BVHShader_params;


/*
 * EXTERN FUNCTIONS
 */

extern "C" int getType()
{
	return BVHSHADER_TYPE_CAMERA;
}

extern "C" void *getParams()
{
	BVHShader_params *params = new BVHShader_params;
	params->filmSize = 36.0f;
	params->focalLength = 50.0f;
	params->aperture = 1.4f;
	params->focalDistance = 10.0f;
	return (void*)params;
}

extern "C" void setParam(void *params, const char *name, void *value)
{
	BVHShader_params *p = (BVHShader_params*)params;
	
	if (strcmp(name, "filmSize") == 0)
		memcpy(&p->filmSize, value, sizeof(p->filmSize));
	else if (strcmp(name, "focalLength") == 0)
		memcpy(&p->focalLength, value, sizeof(p->focalLength));
	else if (strcmp(name, "aperture") == 0)
		memcpy(&p->aperture, value, sizeof(p->aperture));
	else if (strcmp(name, "focalDistance") == 0)
		memcpy(&p->focalDistance, value, sizeof(p->focalDistance));
}

extern "C" void execute(void *params, BVHShader_functions *func, void *env)
{
	BVHShader_camera *camera = (BVHShader_camera*)env;
	BVHShader_params *p = (BVHShader_params*)params;

	float rFocalLength = p->focalLength / p->filmSize;
	float rAperture = rFocalLength * p->filmSize * 0.001f / p->aperture;
	float rayObj[3];
	
	camera->rayPos[0] = 0.0;
	camera->rayPos[1] = rAperture * camera->dx;
	camera->rayPos[2] = rAperture * camera->dy;
	
	rayObj[1] = 0.5f * camera->x / rFocalLength;
	rayObj[2] = 0.5f * camera->y / rFocalLength;
	rayObj[0] = sqrtf(1.0f - rayObj[1] * rayObj[1] - rayObj[2] * rayObj[2]);
	
	rayObj[0] *= p->focalDistance;
	rayObj[1] *= p->focalDistance;
	rayObj[2] *= p->focalDistance;

	camera->rayDir[0] = rayObj[0] - camera->rayPos[0];
	camera->rayDir[1] = rayObj[1] - camera->rayPos[1];
	camera->rayDir[2] = rayObj[2] - camera->rayPos[2];
}
