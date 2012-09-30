/*
 * bvhcamera.h
 *
 *  Created on: 25 Feb 2011
 *      Author: showroom
 */

#ifndef BVHCAMERA_H_
#define BVHCAMERA_H_

/*
 * INCLUDES
 */

#include <math.h>

#include "bvhshader.h"


/*
 * CLASS: BVHCamera
 */

class BVHCamera
{
private:
		/* Shaders */
	BVHShader shader;
	BVHShader jitter;

public:
		/* Constructor and destructor */
	BVHCamera();
	~BVHCamera();

		/* Init */
	int init(const char *shader, const char *jitter);
	int initShader(const char *shader);
	int initJitter(const char *jitter);

		/* Get shaders */
	BVHShader *getShader();
	BVHShader *getJitter();

		/* Set params */
	void setJitterParam(const char *name, void *value);
	void setShaderParam(const char *name, void *value);

		/* Get ray */
	void getRay(
		float x, float y,
		int *kernelDim, int *kernelIdx,
		float *rayPos, float *rayDir,
		gsl_rng *rng);
};


#endif /* BVHCAMERA_H_ */
