/*
 * bvhcamera.cpp
 *
 *  Created on: 25 Feb 2011
 *      Author: showroom
 */

/*
 * INCLUDES
 */

#include "bvhcamera.h"


/*
 * NAMESPACES
 */

using namespace std;


/*
 * CLASS: BVHCamera
 */

	/* Constructor and destructor */

BVHCamera::BVHCamera()
{
	return;
}

BVHCamera::~BVHCamera()
{
	return;
}


	/* Init */

int BVHCamera::init(const char *shader, const char *jitter)
{
	if (!this->shader.compileAndLoad(shader))
		return 0;

	if (this->shader.getType() != BVHSHADER_TYPE_CAMERA)
	{
		this->shader.unload();
		return 0;
	}

	if (!this->jitter.compileAndLoad(jitter))
		return 0;

	if (this->jitter.getType() != BVHSHADER_TYPE_JITTER_2D)
	{
		this->shader.unload();
		this->jitter.unload();
		return 0;
	}

	return 1;
}

int BVHCamera::initShader(const char *shader)
{
	if (!this->shader.compileAndLoad(shader))
		return 0;

	if (this->shader.getType() != BVHSHADER_TYPE_CAMERA)
	{
		this->shader.unload();
		return 0;
	}

	return 1;
}

int BVHCamera::initJitter(const char *jitter)
{
	if (!this->jitter.compileAndLoad(jitter))
		return 0;

	if (this->jitter.getType() != BVHSHADER_TYPE_JITTER_2D)
	{
		this->jitter.unload();
		return 0;
	}

	return 1;
}


	/* Get shaders */

BVHShader *BVHCamera::getShader()
{
	return &this->shader;
}

BVHShader *BVHCamera::getJitter()
{
	return &this->jitter;
}



	/* Set params */

void BVHCamera::setJitterParam(const char *name, void *value)
{
	this->jitter.setParam(name, value);
}

void BVHCamera::setShaderParam(const char *name, void *value)
{
	this->shader.setParam(name, value);
}


	/* Get ray */

void BVHCamera::getRay(
	float x, float y,
	int *kernelDim, int *kernelIdx,
	float *rayPos, float *rayDir,
	gsl_rng *rng)
{
		/* Variables */

	BVHShader_jitter2d jitter;
	BVHShader_camera camera;


		/* Run jitter shader */

	jitter.kernelDim[0] = kernelDim[0];
	jitter.kernelDim[1] = kernelDim[1];
	jitter.kernelIdx[0] = kernelIdx[0];
	jitter.kernelIdx[1] = kernelIdx[1];
	jitter.rng = (void*)rng;

	this->jitter.execute(&jitter);


		/* Run camera shader */

	camera.x = x;
	camera.y = y;
	camera.dx = jitter.dx;
	camera.dy = jitter.dy;
	camera.wavelength = 450.0e-9;

	this->shader.execute(&camera);


		/* Return data */

	rayPos[0] = camera.rayPos[0];
	rayPos[1] = camera.rayPos[1];
	rayPos[2] = camera.rayPos[2];

	rayDir[0] = camera.rayDir[0];
	rayDir[1] = camera.rayDir[1];
	rayDir[2] = camera.rayDir[2];
}
