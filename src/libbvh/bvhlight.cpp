/*
 * bvhlight.cpp
 *
 *  Created on: 21 Apr 2011
 *      Author: showroom
 */

/*
 * INCLUDES
 */

#include "bvhlight.h"


/*
 * NAMESPACES
 */

using namespace std;


/*
 * CLASS: BVHLight
 */

	/* Constructor and destructor */

BVHLight::BVHLight()
{
	this->totalArea = 0.0f;
}

BVHLight::~BVHLight()
{
	return;
}


	/* Init */

int BVHLight::init(const char *shader, const char *emitter)
{
	if (!this->shader.compileAndLoad(shader))
		return 0;

	if (this->shader.getType() != BVHSHADER_TYPE_LIGHT)
	{
		this->shader.unload();
		return 0;
	}

	if (emitter != NULL)
	{
		if (!this->emitter.compileAndLoad(emitter))
			return 0;

		if (this->emitter.getType() != BVHSHADER_TYPE_EMITTER)
		{
			this->shader.unload();
			this->emitter.unload();
			return 0;
		}
	}

	return 1;
}

int BVHLight::initShader(const char *shader)
{
	if (!this->shader.compileAndLoad(shader))
		return 0;

	if (this->shader.getType() != BVHSHADER_TYPE_LIGHT)
	{
		this->shader.unload();
		return 0;
	}

	return 1;
}

int BVHLight::initEmitter(const char *emitter)
{
	if (!this->emitter.compileAndLoad(emitter))
		return 0;

	if (this->emitter.getType() != BVHSHADER_TYPE_EMITTER)
	{
		this->emitter.unload();
		return 0;
	}

	return 1;
}


	/* Get shaders */

BVHShader *BVHLight::getShader()
{
	return &this->shader;
}

BVHShader *BVHLight::getEmitter()
{
	return &this->emitter;
}


	/* Add primitive */

void BVHLight::addPrimitive(BVHPrimitive *primitive)
{
	BVHLight_primitive primNew;

	primNew.primitive = primitive;
	primNew.area = primitive->getArea();
	primNew.accumArea = this->totalArea;
	this->totalArea += primNew.area;

	this->primitives.push_back(primNew);
}

void BVHLight::addPrimitives(std::list<BVHPrimitive> *primitives)
{
	std::list<BVHPrimitive>::iterator primIt;
	BVHLight_primitive primNew;

	for (primIt = primitives->begin(); primIt != primitives->end(); primIt++)
	{
		primNew.primitive = &(*primIt);
		primNew.area = (*primIt).getArea();
		primNew.accumArea = this->totalArea;
		this->totalArea += primNew.area;

		this->primitives.push_back(primNew);
	}
}


	/* Get ray & color */

void BVHLight::setShaderParam(const char *name, void *value)
{
	this->shader.setParam(name, value);
}

void BVHLight::setEmitterParam(const char *name, void *value)
{
	this->emitter.setParam(name, value);
}

BVHShader *BVHLight::getRay(gsl_rng *rng, BVHShader_emitter *env)
{
	if (this->emitter.getType() == BVHSHADER_TYPE_UNDEFINED)	// Based on an object
	{
		// Check if there are primitives
		if (this->primitives.size() == 0)
			return NULL;

		// Choose a random primitive
		float primitiveArea = this->totalArea * (float)rand() / (float)RAND_MAX;

		// Find the primitive
		int primitiveIndex = 0;

		// Binary search
		int minIndex = 0;
		int maxIndex = this->primitives.size() - 1;
		int auxIndex;

		while ((maxIndex - minIndex) > 1)
		{
			auxIndex = (maxIndex + minIndex) / 2;

			if (primitiveArea > this->primitives[auxIndex].accumArea)
				minIndex = auxIndex;
			else
				maxIndex = auxIndex;
		}

		primitiveIndex = minIndex;

		// Get a random ray in the primitive
		this->primitives[primitiveIndex].primitive->getRandomRay(rng, env->rayPos, env->rayDir, env->normal, env->pdf);

		env->rayPos[0] += env->normal[0] * 0.001f;
		env->rayPos[1] += env->normal[1] * 0.001f;
		env->rayPos[2] += env->normal[2] * 0.001f;

		// Modify the pdf with the total area
		env->pdf /= this->totalArea;

		return this->primitives[primitiveIndex].primitive->getMaterial();
	}
	else	// Based on a shader
	{
		this->emitter.execute(env);
		return NULL;
	}
}

void BVHLight::getColor(BVHShader_light *env)
{
	this->shader.execute(env);
}
