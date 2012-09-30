/*
 * bvhprimitive.h
 *
 *  Created on: 19 Jan 2011
 *      Author: showroom
 */

#ifndef BVHPRIMITIVE_H_
#define BVHPRIMITIVE_H_

/*
 * INCLUDES
 */

	/* General */
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <math.h>
#include <list>

	/* Utils */
#include "utils.h"

	/* BVH */
#include "bvhaabb.h"
#include "bvhray.h"
#include "bvhshader.h"


/*
 * DEFINES
 */

	/* Primitive types */
#define BVHPRIMITIVE_TYPE_UNDEFINED	0
#define BVHPRIMITIVE_TYPE_TRIANGLE	1


/*
 * TYPES
 */

	/* Data structures for the primitive types */

typedef struct
{
	float v1[3];
	float v2[3];
	float v3[3];

	float normal[3];
	float n12[3];
	float n23[3];
	float n31[3];
}BVHPrimitive_triangle;


/*
 * CLASS: BVHPrimitive
 */

class BVHPrimitive
{
private:
		/* Primitive variables */
	unsigned char type;				// Primitive type (see defines up)
	unsigned int index;				// Primitive index
	unsigned short geomDataSize;	// Geometric data size
	char *geomData;					// Geometric dada needed for the intersection calculation
	BVHAttribAtlas *attribAtlas;	// Pointer to the attribute atlas
	int attribNumber;				// Number of attributes
	char *attribBuffer;				// Buffer with the attributes

		/* Attrib atlas list */
	static std::list<BVHAttribAtlas> attribAtlasList;

		/* Material */
	BVHShader *material;

public:
		/* Constructor and destructor */
	BVHPrimitive();
	~BVHPrimitive();

		/* Get AABB */
	BVHaabb getAABB();

		/* Get Type */
	int getType();

		/* Add attrib atlas */
	static BVHAttribAtlas *addAttribAtlas(BVHAttribAtlas atlas);

		/* Init */
	int init(int type, BVHAttribAtlas *attribAtlas, int attribNumber, char *attribBuffer);

		/* Init geom data */
	int initGeom(int type, char *geomData, int geomDataSize = 0);
	int initGeom(BVHPrimitive &primitive);

		/* Get geometric data */
	int getGeomDataSize();
	char *getGeomData();

		/* Index */
	void setIndex(int index);
	int getIndex();

		/* Clean the data */
	void clean();

		/* Get primitive area */
	float getArea();

		/* Get random ray */
	void getRandomRay(gsl_rng *rng, float *rayPos, float *rayDir, float *normal, float &pdf);

		/* Material */
	void setMaterial(BVHShader *material);
	BVHShader *getMaterial();

		/* Get attrib information */
	BVHAttribAtlas *getAttribAtlas();
	int getAttribNumber();
	char *getAttribBuffer();

private:
		/* Calculate internal data */
	void calculateInternal();
};

#endif /* BVHPRIMITIVE_H_ */
