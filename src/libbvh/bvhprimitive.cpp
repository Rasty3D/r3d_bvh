/*
 * bvhprimitive.cpp
 *
 *  Created on: 19 Jan 2011
 *      Author: showroom
 */


/*
 * INCLUDES
 */

#include "bvhprimitive.h"


/*
 * NAMESPACES
 */

using namespace std;


/*
 * CLASS: BVHPrimitive
 */

	/* Attrib atlas list */

std::list<BVHAttribAtlas> BVHPrimitive::attribAtlasList;


	/* Constructor and destructor */

BVHPrimitive::BVHPrimitive()
{
	this->geomData = NULL;
	this->type = BVHPRIMITIVE_TYPE_UNDEFINED;
	this->index = 0;
	this->attribAtlas = NULL;
	this->attribBuffer = NULL;
	this->material = NULL;
}

BVHPrimitive::~BVHPrimitive()
{
	this->clean();
}


	/* Get AABB */

BVHaabb BVHPrimitive::getAABB()
{
	BVHaabb aabb;

	if (this->geomData == NULL)
		return aabb;

	if (this->type == BVHPRIMITIVE_TYPE_TRIANGLE)
	{
		BVHPrimitive_triangle *triangle = (BVHPrimitive_triangle*)this->geomData;
		aabb.update(triangle->v1);
		aabb.update(triangle->v2);
		aabb.update(triangle->v3);
		aabb.enlarge(0.001f);
	}

	return aabb;
}


	/* Get Type */

int BVHPrimitive::getType()
{
	return this->type;
}


	/* Add attrib atlas */

BVHAttribAtlas *BVHPrimitive::addAttribAtlas(BVHAttribAtlas atlas)
{
	BVHPrimitive::attribAtlasList.push_back(atlas);
	return &BVHPrimitive::attribAtlasList.back();
}


	/* Init */

int BVHPrimitive::init(int type, BVHAttribAtlas *attribAtlas, int attribNumber, char *attribBuffer)
{
	// Delete previous data
	this->clean();

	// Check atlas
	if (attribAtlas == NULL)
		return 0;

	// Save attrib information
	this->attribAtlas = attribAtlas;
	this->attribNumber = attribNumber;
	this->attribBuffer = new char[this->attribAtlas->size * this->attribNumber];
	memcpy(this->attribBuffer, attribBuffer, this->attribAtlas->size * this->attribNumber);

	// Generate geometry data depending on the type
	if (type == BVHPRIMITIVE_TYPE_TRIANGLE)
	{
		this->geomDataSize = sizeof(BVHPrimitive_triangle);
		this->geomData = new char[this->geomDataSize];
		this->type = type;
		BVHPrimitive_triangle *triangle = (BVHPrimitive_triangle*)this->geomData;

		// The number of attributes should be three
		if (this->attribNumber != 3)
		{
			this->clean();
			return 0;
		}

		// Look for the position attribute
		int attribPosition = -1;

		for (int i = 0; i < this->attribAtlas->number; i++)
		{
			if (this->attribAtlas->attrList[i].name[0] == 'P' &&
				this->attribAtlas->attrList[i].name[1] == '\0')
			{
				attribPosition = i;
				break;
			}
		}

		if (attribPosition == -1)
		{
			this->clean();
			return 0;
		}

		// Check properties of the attribute
		if (this->attribAtlas->attrList[attribPosition].type != BVHSHADER_TYPE_FLOAT ||
			this->attribAtlas->attrList[attribPosition].length != 3 ||
			this->attribAtlas->attrList[attribPosition].size != sizeof(float) * 3)
		{
			this->clean();
			return 0;
		}

		// Calculate geom data from position attribute
		memcpy(
			triangle->v1,
			&this->attribBuffer[this->attribAtlas->attrList[attribPosition].offset],
			this->attribAtlas->attrList[attribPosition].size);
		memcpy(
			triangle->v2,
			&this->attribBuffer[this->attribAtlas->attrList[attribPosition].offset + this->attribAtlas->size],
			this->attribAtlas->attrList[attribPosition].size);
		memcpy(
			triangle->v3,
			&this->attribBuffer[this->attribAtlas->attrList[attribPosition].offset + this->attribAtlas->size * 2],
			this->attribAtlas->attrList[attribPosition].size);
	}
	else
	{
		// Unknown primitive type, return error
		return 0;
	}

	// Calculate internal data
	this->calculateInternal();

	// Return ok
	return 1;
}


	/* Init geom data */

int BVHPrimitive::initGeom(int type, char *geomData, int geomDataSize)
{
	// Delete previous data
	this->clean();

	// Check data size
	if (type == BVHPRIMITIVE_TYPE_TRIANGLE)
	{
		if (geomDataSize > 0 && geomDataSize != sizeof(BVHPrimitive_triangle))
			return 0;
	}
	else
	{
		return 0;
	}

	// Copy data
	this->geomDataSize = geomDataSize;
	this->geomData = new char[geomDataSize];
	memcpy(this->geomData, geomData, geomDataSize);
	this->type = type;

	// Calculate internal data
	this->calculateInternal();

	// Return ok
	return 1;
}

int BVHPrimitive::initGeom(BVHPrimitive &primitive)
{
	// Delete previous data
	this->clean();

	// Check type and copy data
	if (primitive.type == BVHPRIMITIVE_TYPE_TRIANGLE)
	{
		this->type = primitive.type;
		this->geomDataSize = sizeof(BVHPrimitive_triangle);
		this->geomData = new char[this->geomDataSize];
		memcpy(this->geomData, primitive.geomData, this->geomDataSize);
	}
	else
	{
		return 0;
	}

	// Calculate internal data
	this->calculateInternal();

	// Return ok
	return 1;
}


	/* Get geometric data */

int BVHPrimitive::getGeomDataSize()
{
	return this->geomDataSize;
}

char *BVHPrimitive::getGeomData()
{
	return this->geomData;
}


	/* Index */

void BVHPrimitive::setIndex(int index)
{
	this->index = index;
}

int BVHPrimitive::getIndex()
{
	return this->index;
}


	/* Clean the data */

void BVHPrimitive::clean()
{
	if (this->geomData != NULL)
		delete [] this->geomData;
	this->geomData = NULL;

	this->type = BVHPRIMITIVE_TYPE_UNDEFINED;
	this->geomDataSize = 0;

	if (this->attribBuffer != NULL)
		delete [] this->attribBuffer;
	this->attribBuffer = NULL;
}


	/* Get primitive area */

float BVHPrimitive::getArea()
{
	if (this->geomData == NULL)
		return 0.0f;

	if (this->type == BVHPRIMITIVE_TYPE_TRIANGLE)
	{
		float vecAux[3];
		BVHPrimitive_triangle *triangle;

		// Get triangle data
		triangle = (BVHPrimitive_triangle*)this->geomData;

		// Calculate area
		vecAux[0] =
			(triangle->v3[1] - triangle->v1[1]) * (triangle->v2[2] - triangle->v1[2]) -
			(triangle->v3[2] - triangle->v1[2]) * (triangle->v2[1] - triangle->v1[1]);
		vecAux[1] =
			(triangle->v3[2] - triangle->v1[2]) * (triangle->v2[0] - triangle->v1[0]) -
			(triangle->v3[0] - triangle->v1[0]) * (triangle->v2[2] - triangle->v1[2]);
		vecAux[2] =
			(triangle->v3[0] - triangle->v1[0]) * (triangle->v2[1] - triangle->v1[1]) -
			(triangle->v3[1] - triangle->v1[1]) * (triangle->v2[0] - triangle->v1[0]);

		return sqrtf(vecAux[0] * vecAux[0] + vecAux[1] * vecAux[1] + vecAux[2] * vecAux[2]) * 0.5f;
	}
	else
	{
		return 0.0f;
	}
}


	/* Get random ray */

void BVHPrimitive::getRandomRay(gsl_rng *rng, float *rayPos, float *rayDir, float *normal, float &pdf)
{
	if (this->geomData == NULL)
		return;

	if (this->type == BVHPRIMITIVE_TYPE_TRIANGLE)
	{
		// Get triangle data
		BVHPrimitive_triangle *triangle;
		triangle = (BVHPrimitive_triangle*)this->geomData;

		// Select a random point on the triangle
		float coefs[3];

		do
		{
			coefs[0] = randFloat(rng, 0.0f, 1.0f);
			coefs[1] = randFloat(rng, 0.0f, 1.0f);
			coefs[2] = 1.0f - coefs[0] - coefs[1];
		}while (coefs[2] > 1.0f || coefs[2] < 0.0f);

		rayPos[0] =
			triangle->v1[0] * coefs[0] +
			triangle->v2[0] * coefs[1] +
			triangle->v3[0] * coefs[2];
		rayPos[1] =
			triangle->v1[1] * coefs[0] +
			triangle->v2[1] * coefs[1] +
			triangle->v3[1] * coefs[2];
		rayPos[2] =
			triangle->v1[2] * coefs[0] +
			triangle->v2[2] * coefs[1] +
			triangle->v3[2] * coefs[2];

		// Generate a random direction
		rayDir[0] = randFloat(rng, -1.0f, 1.0f);
		rayDir[1] = randFloat(rng, -1.0f, 1.0f);
		rayDir[2] = randFloat(rng, -1.0f, 1.0f);
		normalize(rayDir);

		if (dot(rayDir, triangle->normal) < 0.0f)
			invert(rayDir);

		normal[0] = triangle->normal[0];
		normal[1] = triangle->normal[1];
		normal[2] = triangle->normal[2];

		pdf = 1.0f / PI;
	}
}


	/* Material */

void BVHPrimitive::setMaterial(BVHShader *material)
{
	if (material != NULL &&
		material->getType() == BVHSHADER_TYPE_SURFACE)
		this->material = material;
}

BVHShader *BVHPrimitive::getMaterial()
{
	return this->material;
}


	/* Get attrib information */

BVHAttribAtlas *BVHPrimitive::getAttribAtlas()
{
	return this->attribAtlas;
}

int BVHPrimitive::getAttribNumber()
{
	return this->attribNumber;
}

char *BVHPrimitive::getAttribBuffer()
{
	return this->attribBuffer;
}


	/* Calculate internal data */

void BVHPrimitive::calculateInternal()
{
	if (this->geomData == NULL)
		return;

	if (this->type == BVHPRIMITIVE_TYPE_TRIANGLE)
	{
		// Variables
		BVHPrimitive_triangle *triangle;
		float v12[3];
		float v23[3];
		float v31[3];
		float module;

		// Get triangle data
		triangle = (BVHPrimitive_triangle*)this->geomData;

		// Calculate side vectors
		v12[0] = triangle->v2[0] - triangle->v1[0];
		v12[1] = triangle->v2[1] - triangle->v1[1];
		v12[2] = triangle->v2[2] - triangle->v1[2];
		v23[0] = triangle->v3[0] - triangle->v2[0];
		v23[1] = triangle->v3[1] - triangle->v2[1];
		v23[2] = triangle->v3[2] - triangle->v2[2];
		v31[0] = triangle->v1[0] - triangle->v3[0];
		v31[1] = triangle->v1[1] - triangle->v3[1];
		v31[2] = triangle->v1[2] - triangle->v3[2];

		// Calculate normal
		triangle->normal[0] = v12[2] * v31[1] - v12[1] * v31[2];
		triangle->normal[1] = v12[0] * v31[2] - v12[2] * v31[0];
		triangle->normal[2] = v12[1] * v31[0] - v12[0] * v31[1];

		if (triangle->normal[0] == 0.0f && triangle->normal[1] == 0.0f && triangle->normal[2] == 0.0f)
		{
			this->type = BVHPRIMITIVE_TYPE_UNDEFINED;
			delete [] this->geomData;
			this->geomData = NULL;
			return;
		}

		module = sqrtf(
			triangle->normal[0] * triangle->normal[0] +
			triangle->normal[1] * triangle->normal[1] +
			triangle->normal[2] * triangle->normal[2]);

		triangle->normal[0] /= module;
		triangle->normal[1] /= module;
		triangle->normal[2] /= module;

		// Calculate internal normals
		triangle->n12[0] = triangle->normal[1] * v12[2] - triangle->normal[2] * v12[1];
		triangle->n12[1] = triangle->normal[2] * v12[0] - triangle->normal[0] * v12[2];
		triangle->n12[2] = triangle->normal[0] * v12[1] - triangle->normal[1] * v12[0];
		triangle->n23[0] = triangle->normal[1] * v23[2] - triangle->normal[2] * v23[1];
		triangle->n23[1] = triangle->normal[2] * v23[0] - triangle->normal[0] * v23[2];
		triangle->n23[2] = triangle->normal[0] * v23[1] - triangle->normal[1] * v23[0];
		triangle->n31[0] = triangle->normal[1] * v31[2] - triangle->normal[2] * v31[1];
		triangle->n31[1] = triangle->normal[2] * v31[0] - triangle->normal[0] * v31[2];
		triangle->n31[2] = triangle->normal[0] * v31[1] - triangle->normal[1] * v31[0];
	}
	else
	{
		return;
	}
}
