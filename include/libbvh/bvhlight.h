/*
 * bvhcamera.h
 *
 *  Created on: 25 Feb 2011
 *      Author: showroom
 */

#ifndef BVHLIGHT_H_
#define BVHLIGHT_H_

/*
 * INCLUDES
 */

#include <math.h>
#include <vector>

#include "bvhprimitive.h"
#include "bvhshader.h"


/*
 * TYPES
 */

typedef struct
{
	BVHPrimitive *primitive;
	float area;
	float accumArea;
}BVHLight_primitive;


/*
 * CLASS: BVHLight
 */

class BVHLight
{
private:
		/* Shader */
	BVHShader shader;
	BVHShader emitter;

		/* List of primitives */
	float totalArea;
	std::vector<BVHLight_primitive> primitives;

public:
		/* Constructor and destructor */
	BVHLight();
	~BVHLight();

		/* Init */
	int init(const char *shader, const char *emitter);
	int initShader(const char *shader);
	int initEmitter(const char *emitter);

		/* Get shaders */
	BVHShader *getShader();
	BVHShader *getEmitter();

		/* Add primitive */
	void addPrimitive(BVHPrimitive *primitive);
	void addPrimitives(std::list<BVHPrimitive> *primitives);

		/* Get ray & color */
	void setShaderParam(const char *name, void *value);
	void setEmitterParam(const char *name, void *value);
	BVHShader *getRay(gsl_rng *rng, BVHShader_emitter *env);
	void getColor(BVHShader_light *env);
};

#endif /* BVHLIGHT_H_ */
