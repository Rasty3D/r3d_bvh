/*
 * bvhpath.cpp
 *
 *  Created on: 21 Mar 2011
 *      Author: showroom
 */


/*
 * INCLUDES
 */

#include "bvhpath.h"


/*
 * NAMESPACE
 */

using namespace std;


/*
 * CLASS: BVHPath
 */

	/* Default segment lenghts */

int BVHPath::eyeSegmentLength = 0;
int BVHPath::lightSegmentLength = 0;


	/* Constructor and destructor */

BVHPath::BVHPath()
{
	this->nEyeSegments = 0;
	this->eyeSegments = NULL;
	this->nLightSegments = 0;
	this->lightSegments = NULL;

	if (BVHPath::eyeSegmentLength > 0 && BVHPath::lightSegmentLength > 0)
		this->init(BVHPath::eyeSegmentLength, BVHPath::lightSegmentLength);
}

BVHPath::~BVHPath()
{
	this->destroy();
}


	/* Init path */

void BVHPath::init(int eyeSegmentLength, int lightSegmentLength)
{
	// Destroy previous path if existing
	this->destroy();

	// Check path lenghts
	if (eyeSegmentLength <= 0 || lightSegmentLength <= 0)
		return;

	// Init eye segments
	this->nEyeSegments = eyeSegmentLength;
	this->eyeSegments = new BVHPath_segment[this->nEyeSegments];

	this->eyeSegments[0].nColor = NULL;
	this->eyeSegments[0].sColor = NULL;

	for (int i = 1; i < this->nEyeSegments - 1; i++)
	{
		this->eyeSegments[i].nColor = new BVHColor[lightSegmentLength + 1];
		this->eyeSegments[i].sColor = new BVHColor[1];
	}

	this->eyeSegments[this->nEyeSegments - 1].nColor = new BVHColor[lightSegmentLength];
	this->eyeSegments[this->nEyeSegments - 1].sColor = new BVHColor[1];

	// Init light segments
	this->nLightSegments = lightSegmentLength;
	this->lightSegments = new BVHPath_segment[this->nLightSegments];

	for (int i = 0; i < this->nLightSegments - 1; i++)
	{
		this->lightSegments[i].nColor = new BVHColor[eyeSegmentLength];
		this->lightSegments[i].sColor = new BVHColor[eyeSegmentLength];
	}

	this->lightSegments[this->nLightSegments - 1].nColor = new BVHColor[eyeSegmentLength - 1];
	this->lightSegments[this->nLightSegments - 1].sColor = new BVHColor[eyeSegmentLength - 1];

	// Reset
	this->reset();
}

void BVHPath::reset()
{
	// Reset lengths
	this->resetLengths();

	/*
	// Reset eye segment colors
	for (int i = 1; i < this->nEyeSegments - 1; i++)
	{
		for (int j = 0; j < this->nLightSegments + 1; j++)
			this->eyeSegments[i].nColor[j].init(1.0f);

		this->eyeSegments[i].sColor[0].init(1.0f);
	}

	for (int j = 0; j < this->nLightSegments; j++)
		this->eyeSegments[this->nEyeSegments - 1].nColor[j].init(1.0f);

	this->eyeSegments[this->nEyeSegments - 1].sColor[0].init(1.0f);

	// Init light segment colors
	for (int i = 0; i < this->nLightSegments - 1; i++)
	{
		for (int j = 0; j < this->nEyeSegments + 1; j++)
		{
			this->lightSegments[i].nColor[j].init(1.0f);
			this->lightSegments[i].sColor[j].init(1.0f);
		}
	}

	for (int j = 0; j < this->nEyeSegments; j++)
	{
		this->lightSegments[this->nLightSegments - 1].nColor[j].init(1.0f);
		this->lightSegments[this->nLightSegments - 1].sColor[j].init(1.0f);
	}*/
}


	/* Set default segment lenghts */

void BVHPath::setDefaultSegmentLengths(int eyeSegmentLength, int lightSegmentLength)
{
	BVHPath::eyeSegmentLength = eyeSegmentLength;
	BVHPath::lightSegmentLength = lightSegmentLength;
}


	/* Get lenghts */

int BVHPath::getLenght(int type)
{
	if (type == BVHPATH_EYE)
		return this->nEyeSegmentsCut;
	else if (type == BVHPATH_LIGHT)
		return this->nLightSegmentsCut;
	else
		return 0;
}


	/* Set values */

int BVHPath::setNodePositionNormal(int type, int id, float *position, float *normal)
{
	if (type == BVHPATH_EYE)
	{
		if (id >= this->nEyeSegmentsCut || id < 0)
			return 0;

		this->eyeSegments[id].position[0] = position[0];
		this->eyeSegments[id].position[1] = position[1];
		this->eyeSegments[id].position[2] = position[2];

		this->eyeSegments[id].normal[0] = normal[0];
		this->eyeSegments[id].normal[1] = normal[1];
		this->eyeSegments[id].normal[2] = normal[2];
	}
	else if (type == BVHPATH_LIGHT)
	{
		if (id >= this->nLightSegmentsCut || id < 0)
			return 0 ;

		this->lightSegments[id].position[0] = position[0];
		this->lightSegments[id].position[1] = position[1];
		this->lightSegments[id].position[2] = position[2];

		this->lightSegments[id].normal[0] = normal[0];
		this->lightSegments[id].normal[1] = normal[1];
		this->lightSegments[id].normal[2] = normal[2];
	}

	return 1;
}


	/* Manage eye segment length */

// Reset eye and light lengths
void BVHPath::resetLengths()
{
	this->nEyeSegmentsCut = this->nEyeSegments;
	this->nLightSegmentsCut = this->nLightSegments;
}

// Cut eye length
void BVHPath::cutEyeLength(int id)
{
	if (id > this->nEyeSegments)
		this->nEyeSegmentsCut = this->nEyeSegments;
	else
		this->nEyeSegmentsCut = id;
}

// Cut light length
void BVHPath::cutLightLength(int id)
{
	if (id > this->nLightSegments)
		this->nLightSegmentsCut = this->nLightSegments;
	else
		this->nLightSegmentsCut = id;
}


	/* Subpath operations */

// Get the number of subpaths
int BVHPath::getSubpathNumber()
{
	return (this->nEyeSegments - 1) * this->nLightSegments;
}

// Get the subpath step
int BVHPath::getSubpathStep()
{
	return this->nLightSegments;
}

// Get a subpath in a sequence
int BVHPath::getSubpathPositions(int id, float *posEyePrev, float *posEye, float *posLight, float *posLightNext)
{
	if (id < 0 || id >= this->getSubpathNumber())
		return 0;

	int idEye = (id / this->nLightSegments) + 1;
	int idLight = id % this->nLightSegments;

	if (idEye >= this->nEyeSegmentsCut ||
		idLight >= this->nLightSegmentsCut)
		return 0;

	posEyePrev[0] = this->eyeSegments[idEye - 1].position[0];
	posEyePrev[1] = this->eyeSegments[idEye - 1].position[1];
	posEyePrev[2] = this->eyeSegments[idEye - 1].position[2];

	posEye[0] = this->eyeSegments[idEye].position[0];
	posEye[1] = this->eyeSegments[idEye].position[1];
	posEye[2] = this->eyeSegments[idEye].position[2];

	posLight[0] = this->lightSegments[idLight].position[0];
	posLight[1] = this->lightSegments[idLight].position[1];
	posLight[2] = this->lightSegments[idLight].position[2];

	if (idLight > 0)
	{
		posLightNext[0] = this->lightSegments[idLight - 1].position[0];
		posLightNext[1] = this->lightSegments[idLight - 1].position[1];
		posLightNext[2] = this->lightSegments[idLight - 1].position[2];
	}

	return 1;
}

// Set subpath colors
int BVHPath::setSubpathColors(int id, BVHColor &nodeEye, BVHColor &nodeLight, BVHColor &segment)
{
	if (id < 0 || id >= this->getSubpathNumber())
		return 0;

	int idEye = (id / this->nLightSegments) + 1;
	int idLight = id % this->nLightSegments;

	if (idEye >= this->nEyeSegmentsCut ||
		idLight >= this->nLightSegmentsCut)
		return 0;

	this->eyeSegments[idEye].nColor[idLight] = nodeEye;
	this->lightSegments[idLight].nColor[idEye - 1] = nodeLight;
	this->lightSegments[idLight].sColor[idEye - 1] = segment;

	return 1;
}

// Get subpath node id
int BVHPath::getSubpathNodeId(int id, int type)
{
	if (type == BVHPATH_EYE)
	{
		int idEye = (id / this->nLightSegments) + 1;

		if (idEye >= this->nEyeSegmentsCut)
			return -1;
		else
			return idEye;
	}
	else if (type == BVHPATH_LIGHT)
	{

		int idLight = id % this->nLightSegments;

		if (idLight >= this->nLightSegmentsCut)
			return -1;
		else
			return idLight;
	}

	return -1;
}

// Get the number of subpaths with the same length
int BVHPath::getSubpathNumberSameLength(int id)
{
	if (id < 0 || id >= this->getSubpathNumber())
		return 0;

	int idEye = (id / this->nLightSegments) + 1;
	int idLight = id % this->nLightSegments;

	if (idEye >= this->nEyeSegmentsCut ||
		idLight >= this->nLightSegmentsCut)
		return 0;

	int idEyeInv = this->nEyeSegmentsCut - idEye;
	int idLightInv = this->nLightSegmentsCut - idLight;

	return min(idEye + idLight, idEyeInv + idLightInv - 1);
}


	/* Path operations */

// Get path positions
int BVHPath::getPathPositions(int type, int id, float *posPrev, float *pos, float *posNext)
{
	if (type == BVHPATH_EYE)
	{
		if (id >= this->nEyeSegmentsCut || id < 0)
			return 0;

		pos[0] = this->eyeSegments[id].position[0];
		pos[1] = this->eyeSegments[id].position[1];
		pos[2] = this->eyeSegments[id].position[2];

		if (id > 0)
		{
			posPrev[0] = this->eyeSegments[id - 1].position[0];
			posPrev[1] = this->eyeSegments[id - 1].position[1];
			posPrev[2] = this->eyeSegments[id - 1].position[2];
		}

		if (id < this->nEyeSegmentsCut - 1)
		{
			posNext[0] = this->eyeSegments[id + 1].position[0];
			posNext[1] = this->eyeSegments[id + 1].position[1];
			posNext[2] = this->eyeSegments[id + 1].position[2];
		}
	}
	else if (type == BVHPATH_LIGHT)
	{
		if (id >= this->nLightSegmentsCut || id < 0)
			return 0;

		pos[0] = this->lightSegments[id].position[0];
		pos[1] = this->lightSegments[id].position[1];
		pos[2] = this->lightSegments[id].position[2];

		if (id > 0)
		{
			posPrev[0] = this->lightSegments[id - 1].position[0];
			posPrev[1] = this->lightSegments[id - 1].position[1];
			posPrev[2] = this->lightSegments[id - 1].position[2];
		}

		if (id < this->nLightSegmentsCut - 1)
		{
			posNext[0] = this->lightSegments[id + 1].position[0];
			posNext[1] = this->lightSegments[id + 1].position[1];
			posNext[2] = this->lightSegments[id + 1].position[2];
		}
	}

	return 1;
}

// Get path direction in (from eye to light in the eye rays and from light to eye in the light rays)
int BVHPath::getPathDirectionIn(int type, int id, float *dir)
{
	if (type == BVHPATH_EYE)
	{
		if (id >= this->nEyeSegmentsCut || id <= 0)
			return 0;

		dir[0] = this->eyeSegments[id - 1].position[0] - this->eyeSegments[id].position[0];
		dir[1] = this->eyeSegments[id - 1].position[1] - this->eyeSegments[id].position[1];
		dir[2] = this->eyeSegments[id - 1].position[2] - this->eyeSegments[id].position[2];
	}
	else if (type == BVHPATH_LIGHT)
	{
		if (id >= this->nLightSegmentsCut || id <= 0)
			return 0;

		dir[0] = this->lightSegments[id - 1].position[0] - this->lightSegments[id].position[0];
		dir[1] = this->lightSegments[id - 1].position[1] - this->lightSegments[id].position[1];
		dir[2] = this->lightSegments[id - 1].position[2] - this->lightSegments[id].position[2];
	}

	return 1;
}

// Get path direction out (from eye to light in the eye rays and from light to eye in the light rays)
int BVHPath::getPathDirectionOut(int type, int id, float *dir)
{
	if (type == BVHPATH_EYE)
	{
		if (id >= this->nEyeSegmentsCut - 1 || id < 0)
			return 0;

		dir[0] = this->eyeSegments[id + 1].position[0] - this->eyeSegments[id].position[0];
		dir[1] = this->eyeSegments[id + 1].position[1] - this->eyeSegments[id].position[1];
		dir[2] = this->eyeSegments[id + 1].position[2] - this->eyeSegments[id].position[2];
	}
	else if (type == BVHPATH_LIGHT)
	{
		if (id >= this->nLightSegmentsCut - 1 || id < 0)
			return 0;

		dir[0] = this->lightSegments[id + 1].position[0] - this->lightSegments[id].position[0];
		dir[1] = this->lightSegments[id + 1].position[1] - this->lightSegments[id].position[1];
		dir[2] = this->lightSegments[id + 1].position[2] - this->lightSegments[id].position[2];
	}

	return 1;
}

// Set path colors
int BVHPath::setPathColors(int type, int id, BVHColor *node, BVHColor *segment)
{
	if (type == BVHPATH_EYE)
	{
		if (id >= this->nEyeSegmentsCut || id <= 0)
			return 0;

		if (id < this->nEyeSegmentsCut - 1)
			this->eyeSegments[id].nColor[this->nLightSegments] = *node;

		this->eyeSegments[id].sColor[0] = *segment;
	}
	else if (type == BVHPATH_LIGHT)
	{
		if (id >= this->nLightSegmentsCut - 1 || id < 0)
			return 0;

		this->lightSegments[id].nColor[this->nEyeSegments - 1] = *node;
		this->lightSegments[id].sColor[this->nEyeSegments - 1] = *segment;
	}

	return 1;
}


	/* Primitive operations */

// Set node primitive location
int BVHPath::setPrimitiveLocation(int type, int id, unsigned int primitiveLocation)
{
	if (type == BVHPATH_EYE && id >= 0 && id < this->nEyeSegmentsCut)
	{
		this->eyeSegments[id].primitiveLocation = primitiveLocation;
		return 1;
	}
	else if (type == BVHPATH_LIGHT && id >= 0 && id < this->nLightSegmentsCut)
	{
		this->lightSegments[id].primitiveLocation = primitiveLocation;
		return 1;
	}

	return 0;
}

// Get node primitive location
unsigned int BVHPath::getPrimitiveLocation(int type, int id)
{
	if (type == BVHPATH_EYE && id >= 0 && id < this->nEyeSegmentsCut)
	{
		return this->eyeSegments[id].primitiveLocation;
	}
	else if (type == BVHPATH_LIGHT && id >= 0 && id < this->nLightSegmentsCut)
	{
		return this->lightSegments[id].primitiveLocation;
	}
	else
	{
		return BVHPATH_NOPRIMITIVE;
	}
}


	/* Get total contribution of the path */

BVHColor BVHPath::getColor()
{
	/*
	BVHColor color;
	int nPaths = 0;
	color.init(0.0f);

	for (int i = 0; i < this->getSubpathNumber(); i++)
	{
		nPaths = this->getSubpathNumberSameLength(i);

		if (nPaths > 0)
			color += this->getColorSubpath(i) / nPaths;
	}

	return color;*/

	BVHColor color;
	color.init(0.0f);

	for (int idLight = 0; idLight < this->nLightSegmentsCut; idLight++)
	{
		for (int idEye = 1; idEye < this->nEyeSegmentsCut; idEye++)
		{
			color += this->getColorSubpath(idEye, idLight) / (idLight + idEye);
		}
	}

	return color;
}

BVHColor BVHPath::getColorSubpath(int idEye, int idLight)
//BVHColor BVHPath::getColorSubpath(int id)
{
	BVHColor color;
	color.init(0.0f);

	/*
	if (id < 0 || id >= this->getSubpathNumber())
		return color;

	int idEye = (id / this->nLightSegments) + 1;
	int idLight = id % this->nLightSegments;

	if (idEye >= this->nEyeSegmentsCut ||
		idLight >= this->nLightSegmentsCut)
		return color;*/

	color.init(1.0f);

	for (int i = 1; i < idEye; i++)
	{
		color *= this->eyeSegments[i].sColor[0];
		color *= this->eyeSegments[i].nColor[this->nLightSegments];
		//color *= Gfunc(&this->eyeSegments[i], &this->eyeSegments[i - 1]);
	}

	color *= this->eyeSegments[idEye].sColor[0];
	color *= this->eyeSegments[idEye].nColor[idLight];
	//color *= Gfunc(&this->eyeSegments[idEye], &this->eyeSegments[idEye - 1]);

	for (int i = 0; i < idLight; i++)
	{
		color *= this->lightSegments[i].sColor[this->nEyeSegments - 1];
		color *= this->lightSegments[i].nColor[this->nEyeSegments - 1];
		//color *= Gfunc(&this->lightSegments[i + 1], &this->lightSegments[i]);
	}

	color *= this->lightSegments[idLight].sColor[idEye - 1];
	color *= this->lightSegments[idLight].nColor[idEye - 1];
	//color *= Gfunc(&this->lightSegments[idLight], &this->eyeSegments[idEye]);

	return color;
}


	/* Aux functions */

// Print node data
void BVHPath::printNodeData()
{
	for (int i = 0; i < this->nEyeSegmentsCut; i++)
		cout << "Eye segment [" << i << "]: " << this->eyeSegments[i].primitiveLocation << endl;

	for (int i = 0; i < this->nLightSegmentsCut; i++)
		cout << "Light segment [" << i << "]: " << this->lightSegments[i].primitiveLocation << endl;
}

// Print node colors
void BVHPath::printNodeColors()
{
	cout << "Eye segments: " << this->nEyeSegmentsCut << endl;

	for (int i = 1; i < this->nEyeSegmentsCut; i++)
	{
		cout << " Eye segment [" << i << "]:" << endl;

		if (i < this->nEyeSegmentsCut - 1)
		{
			cout << "  EE: ";
			this->eyeSegments[i].nColor[this->nLightSegments].print();
		}

		for (int j = 0; j < this->nLightSegmentsCut; j++)
		{
			cout << "  EL: ";
			this->eyeSegments[i].nColor[j].print();
		}
	}

	cout << "Light segments: " << this->nLightSegmentsCut << endl;

	for (int i = 0; i < this->nLightSegmentsCut; i++)
	{
		cout << " Light segment [" << i << "]:" << endl;

		if (i < this->nLightSegmentsCut - 1)
		{
			cout << "  LL: ";
			this->lightSegments[i].nColor[this->nEyeSegments - 1].print();
		}

		for (int j = 0; j < this->nEyeSegmentsCut - 1; j++)
		{
			cout << "  LE: ";
			this->lightSegments[i].nColor[j].print();
		}
	}
}


	/* Destroy path */

void BVHPath::destroy()
{
	if (this->eyeSegments != NULL)
	{
		for (int i = 0; i < this->nEyeSegments; i++)
		{
			if (this->eyeSegments[i].nColor != NULL)
				delete [] this->eyeSegments[i].nColor;

			if (this->eyeSegments[i].sColor != NULL)
				delete [] this->eyeSegments[i].sColor;
		}

		delete [] this->eyeSegments;
		this->eyeSegments = NULL;
		this->nEyeSegments = 0;
	}

	if (this->lightSegments != NULL)
	{
		for (int i = 0; i < this->nLightSegments; i++)
		{
			if (this->lightSegments[i].nColor != NULL)
				delete [] this->lightSegments[i].nColor;

			if (this->lightSegments[i].sColor != NULL)
				delete [] this->lightSegments[i].sColor;
		}

		delete [] this->lightSegments;
		this->lightSegments = NULL;
		this->nLightSegments = 0;
	}
}


	/* G function */

float BVHPath::Gfunc(BVHPath_segment *n1, BVHPath_segment *n2)
{
	return 1.0f;

	/*
	float dir[3];
	float distance2;
	float distance;
	float Dx, Dy;

	dir[0] = n1->position[0] - n2->position[0];
	dir[1] = n1->position[1] - n2->position[1];
	dir[2] = n1->position[2] - n2->position[2];

	distance2 = dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2];

	distance = sqrtf(distance2);
	dir[0] /= distance;
	dir[1] /= distance;
	dir[2] /= distance;

	Dx = fabsf(n1->normal[0] * dir[0] + n1->normal[1] * dir[1] + n1->normal[2] * dir[2]);
	Dy = fabsf(n2->normal[0] * dir[0] + n2->normal[1] * dir[1] + n2->normal[2] * dir[2]);

	return Dx * Dy / distance2;*/
}
