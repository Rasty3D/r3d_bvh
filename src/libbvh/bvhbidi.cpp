/*
 * bvhbidi.cpp
 *
 *  Created on: 17 May 2011
 *      Author: showroom
 */


/*
 * INCLUDES
 */

#include "bvhbidi.h"


/*
 * NAMESPACE
 */

using namespace std;


/*
 * CLASS: BVHBidi
 */

	/* Default segment lenghts */

int BVHBidi::eyePathLengthMax = BVHBIDI_EYE_LENGTH_DEFAULT;
int BVHBidi::lightPathLengthMax = BVHBIDI_LIGHT_LENGTH_DEFAULT;


	/* Constructor and destructor */

BVHBidi::BVHBidi()
{
	this->eyePathLength = 0;
	this->eyePathLengthCut = 0;
	this->eyePath = NULL;

	this->lightPathLength = 0;
	this->lightPathLengthCut = 0;
	this->lightPath = NULL;

	this->subpaths = NULL;

	this->init(BVHBidi::eyePathLengthMax, BVHBidi::lightPathLengthMax);
}

BVHBidi::~BVHBidi()
{
	this->destroy();
}


	/* Set path maximum lenghts */

void BVHBidi::setPathLengthMax(int eyePathLength, int lightPathLength)
{
	BVHBidi::eyePathLengthMax = eyePathLength;
	BVHBidi::lightPathLengthMax = lightPathLength;
}


	/* Init and destroy */

void BVHBidi::init(int eyePathLength, int lightPathLength)
{
	this->destroy();

	if (eyePathLength == 0 || lightPathLength == 0)
		return;

	this->eyePathLength = eyePathLength;
	this->eyePathLengthCut = 0;
	this->eyePath = new BVHBidi_node[this->eyePathLength];

	this->lightPathLength = lightPathLength;
	this->lightPathLengthCut = 0;
	this->lightPath = new BVHBidi_node[this->lightPathLength];

	this->subpaths = new BVHBidi_subpath[this->eyePathLength * this->lightPathLength];

	this->eyePath[0].attenuation.init(1.0f);
	this->eyePath[0].primitiveLocation = BVHBIDI_NOPRIMITIVE;
	this->lightPath[0].attenuation.init(1.0f);
	this->lightPath[0].primitiveLocation = BVHBIDI_NOPRIMITIVE;
}

void BVHBidi::destroy()
{
	if (this->eyePathLength != 0)
		delete [] this->eyePath;

	this->eyePathLength = 0;
	this->eyePathLengthCut = 0;
	this->eyePath = NULL;

	if (this->lightPathLength != 0)
		delete [] this->lightPath;

	this->lightPathLength = 0;
	this->lightPathLengthCut = 0;
	this->lightPath = NULL;

	if (this->eyePathLength != 0 && this->lightPathLength != 0)
		delete [] this->subpaths;

	this->subpaths = NULL;
}


	/* Reset path lengths */

void BVHBidi::resetPathLengths()
{
	this->eyePathLengthCut = 0;
	this->lightPathLengthCut = 0;
}

void BVHBidi::reset()
{
	this->resetPathLengths();
	this->directLight.init(0.0f);
}


	/* Get path lengths */

int BVHBidi::getPathLength(int type)
{
	if (type == BVHBIDI_EYE)
		return this->eyePathLength;
	else if (type == BVHBIDI_LIGHT)
		return this->lightPathLength;
	else
		return 0;
}

int BVHBidi::getPathLengthCut(int type)
{
	if (type == BVHBIDI_EYE)
		return this->eyePathLengthCut;
	else if (type == BVHBIDI_LIGHT)
		return this->lightPathLengthCut;
	else
		return 0;
}


	/* Set node info */

int BVHBidi::setNodePosition(int type, int index, float *position)
{
	if (type == BVHBIDI_EYE)
	{
		if (index < 0 || index >= this->eyePathLength)
			return 0;

		this->eyePath[index].position[0] = position[0];
		this->eyePath[index].position[1] = position[1];
		this->eyePath[index].position[2] = position[2];
		this->eyePathLengthCut = index + 1;
		return 1;
	}
	else if (type == BVHBIDI_LIGHT)
	{
		if (index < 0 || index >= this->lightPathLength)
			return 0;

		this->lightPath[index].position[0] = position[0];
		this->lightPath[index].position[1] = position[1];
		this->lightPath[index].position[2] = position[2];
		this->lightPathLengthCut = index + 1;
		return 1;
	}

	return 0;
}

int BVHBidi::setNodePrimitiveLocation(int type, int index, unsigned int primitiveLocation)
{
	if (type == BVHBIDI_EYE)
	{
		if (index < 0 || index >= this->eyePathLength)
			return 0;

		this->eyePath[index].primitiveLocation = primitiveLocation;
		return 1;
	}
	else if (type == BVHBIDI_LIGHT)
	{
		if (index < 0 || index >= this->lightPathLength)
			return 0;

		this->lightPath[index].primitiveLocation = primitiveLocation;
		return 1;
	}

	return 0;
}

int BVHBidi::setNodeNormal(int type, int index, float *normal)
{
	float module = sqrtf(
		normal[0] * normal[0] +
		normal[1] * normal[1] +
		normal[2] * normal[2]);

	if (type == BVHBIDI_EYE)
	{
		if (index < 0 || index >= this->eyePathLength)
			return 0;

		this->eyePath[index].normal[0] = normal[0] / module;
		this->eyePath[index].normal[1] = normal[1] / module;
		this->eyePath[index].normal[2] = normal[2] / module;
		return 1;
	}
	else if (type == BVHBIDI_LIGHT)
	{
		if (index < 0 || index >= this->lightPathLength)
			return 0;

		this->lightPath[index].normal[0] = normal[0] / module;
		this->lightPath[index].normal[1] = normal[1] / module;
		this->lightPath[index].normal[2] = normal[2] / module;
		return 1;
	}

	return 0;
}

int BVHBidi::setNodeAttenuation(
	int type, int index,
	BVHColor &media, BVHColor &bsdf, BVHColor &emission,
	float pdf, float *outDirection)
{
	normalize(outDirection);

	if (type == BVHBIDI_EYE)
	{
		if (index > 0 || index < this->eyePathLength)
		{
			this->eyePath[index - 1].attenuation *= media;
			this->eyePath[index].attenuation =
				this->eyePath[index - 1].attenuation *
				(bsdf / pdf) * dotAbs(outDirection, this->eyePath[index].normal) +
				(emission / pdf) * dotAbs(outDirection, this->eyePath[index].normal);

			if (index == 1)
				this->directLight = emission;

			return 1;
		}
	}
	else if (type == BVHBIDI_LIGHT)
	{
		if (index > 0 || index < this->lightPathLength)
		{
			this->lightPath[index - 1].attenuation *= media;
			this->lightPath[index].attenuation =
				this->lightPath[index - 1].attenuation *
				(bsdf / pdf) * dotAbs(outDirection, this->lightPath[index].normal) +
				(emission / pdf) * dotAbs(outDirection, this->lightPath[index].normal);
			return 1;
		}
	}

	return 0;
}

int BVHBidi::setNodeMedia(int type, int index, BVHColor &media)
{
	if (type == BVHBIDI_EYE)
	{
		if (index > 0 || index < this->eyePathLength)
		{
			this->eyePath[index - 1].attenuation *= media;
			return 1;
		}
	}
	else if (type == BVHBIDI_LIGHT)
	{
		if (index > 0 || index < this->lightPathLength)
		{
			this->lightPath[index - 1].attenuation *= media;
			return 1;
		}
	}

	return 0;
}


	/* Get node info */

int BVHBidi::getNodePosition(int type, int index, float *position)
{
	if (type == BVHBIDI_EYE)
	{
		if (index < 0 || index >= this->eyePathLengthCut)
			return 0;

		position[0] = this->eyePath[index].position[0];
		position[1] = this->eyePath[index].position[1];
		position[2] = this->eyePath[index].position[2];
		return 1;
	}
	else if (type == BVHBIDI_LIGHT)
	{
		if (index < 0 || index >= this->lightPathLengthCut)
			return 0;

		position[0] = this->lightPath[index].position[0];
		position[1] = this->lightPath[index].position[1];
		position[2] = this->lightPath[index].position[2];
		return 1;
	}

	return 0;
}

unsigned int BVHBidi::getNodePrimitiveLocation(int type, int index)
{
	if (type == BVHBIDI_EYE)
	{
		if (index < 0 || index >= this->eyePathLengthCut)
			return BVHBIDI_NOPRIMITIVE;

		return this->eyePath[index].primitiveLocation;
	}
	else if (type == BVHBIDI_LIGHT)
	{
		if (index < 0 || index >= this->lightPathLengthCut)
			return BVHBIDI_NOPRIMITIVE;

		return this->lightPath[index].primitiveLocation;
	}

	return BVHBIDI_NOPRIMITIVE;
}


	/* Subpath management */

int BVHBidi::getSubpathNumber()
{
	return this->eyePathLength * this->lightPathLength;
}

int BVHBidi::getSubpathStep()
{
	return this->eyePathLength;
}

int BVHBidi::getSubpathVertices(int index, float *pe, float *pl)
{
	int eyeId = index % this->eyePathLength;
	int lightId = index / this->eyePathLength;

	if (eyeId < 0 || eyeId >= this->eyePathLengthCut ||
		lightId < 0 || lightId >= this->lightPathLengthCut)
		return 0;

	pe[0] = this->eyePath[eyeId].position[0];
	pe[1] = this->eyePath[eyeId].position[1];
	pe[2] = this->eyePath[eyeId].position[2];

	pl[0] = this->lightPath[lightId].position[0];
	pl[1] = this->lightPath[lightId].position[1];
	pl[2] = this->lightPath[lightId].position[2];

	return 1;
}

int BVHBidi::isSubpathEye(int index)
{
	return (index % this->eyePathLength) == 0;
}

int BVHBidi::isSubpathLight(int index)
{
	return (index / this->eyePathLength) == 0;
}

int BVHBidi::getSubpathInfo(int index, float *ne, float *nl, float *de, float *dl, unsigned int &pe, unsigned int &pl)
{
	int eyeId = index % this->eyePathLength;
	int lightId = index / this->eyePathLength;

	if (eyeId < 0 || eyeId >= this->eyePathLengthCut ||
		lightId < 0 || lightId >= this->lightPathLengthCut)
		return 0;

	ne[0] = this->eyePath[eyeId].normal[0];
	ne[1] = this->eyePath[eyeId].normal[1];
	ne[2] = this->eyePath[eyeId].normal[2];

	pe = this->eyePath[eyeId].primitiveLocation;

	if (eyeId > 0)
	{
		de[0] = this->eyePath[eyeId - 1].position[0] - this->eyePath[eyeId].position[0];
		de[1] = this->eyePath[eyeId - 1].position[1] - this->eyePath[eyeId].position[1];
		de[2] = this->eyePath[eyeId - 1].position[2] - this->eyePath[eyeId].position[2];
	}

	nl[0] = this->lightPath[lightId].normal[0];
	nl[1] = this->lightPath[lightId].normal[1];
	nl[2] = this->lightPath[lightId].normal[2];

	pl = this->lightPath[lightId].primitiveLocation;

	if (lightId > 0)
	{
		dl[0] = this->lightPath[lightId - 1].position[0] - this->lightPath[lightId].position[0];
		dl[1] = this->lightPath[lightId - 1].position[1] - this->lightPath[lightId].position[1];
		dl[2] = this->lightPath[lightId - 1].position[2] - this->lightPath[lightId].position[2];
	}

	return 1;
}

int BVHBidi::setSubpathColors(int index, int visible, BVHColor &media, BVHColor &bsdfEye, BVHColor &bsdfLight)
{
	int eyeId = index % this->eyePathLength;
	int lightId = index / this->eyePathLength;

	if (eyeId < 0 || eyeId >= this->eyePathLengthCut ||
		lightId < 0 || lightId >= this->lightPathLengthCut)
		return 0;

	this->subpaths[index].visible = visible;

	if (visible)
	{
		float ple[3] = {
			this->eyePath[eyeId].position[0] - this->lightPath[lightId].position[0],
			this->eyePath[eyeId].position[1] - this->lightPath[lightId].position[1],
			this->eyePath[eyeId].position[2] - this->lightPath[lightId].position[2]};
		float pleNeg[3];
		float distance2 = dot(ple, ple);
		float distance = sqrtf(distance2);
		float g;

		ple[0] /= distance;
		ple[1] /= distance;
		ple[2] /= distance;

		pleNeg[0] = -ple[0];
		pleNeg[1] = -ple[1];
		pleNeg[2] = -ple[2];

		if (distance2 < 0.001f)
		{
			this->subpaths[index].attenuation.init(0.0f);
		}
		else
		{
			if (eyeId == 0 && lightId == 0)
			{
				g = 1.0f / (distance2 * distance2);
			}
			else if (eyeId == 0)
			{
				g = dotPos(this->lightPath[lightId].normal, ple);
			}
			else if (lightId == 0)
			{
				g = dotPos(this->eyePath[eyeId].normal, pleNeg) *
					dotPos(this->lightPath[lightId].normal, ple) /
					distance2;
			}
			else
			{
				g = dotPos(this->eyePath[eyeId].normal, pleNeg) *
					dotPos(this->lightPath[lightId].normal, ple) /
					distance2;
			}

			this->subpaths[index].attenuation = media * bsdfEye * bsdfLight * g;
		}
	}

	return 1;
}


	/* Get path color */

BVHColor BVHBidi::getColor()
{
	// Variables
	int index;
	int eyeId;
	int lightId;
	BVHColor color;

	// Init color
	color.init(0.0f);

	for (eyeId = 1; eyeId < this->eyePathLengthCut; eyeId++)
	{
		for (lightId = 0; lightId < this->lightPathLengthCut; lightId++)
		{
			index = eyeId + lightId * this->eyePathLength;
			this->subpaths[index].factor = 1.0f / ((float)(lightId + eyeId));
		}
	}

	for (eyeId = 1; eyeId < this->eyePathLengthCut; eyeId++)
	{
		for (lightId = 0; lightId < this->lightPathLengthCut; lightId++)
		{
			index = eyeId + lightId * this->eyePathLength;
			color += this->getColor(eyeId, lightId) * this->subpaths[index].factor;
		}
	}

	// TODO: Fix it
	return color + this->directLight * 100.0f;
}

BVHColor BVHBidi::getColor(int eyeId, int lightId)
{
	BVHColor color;
	int index = eyeId + lightId * this->eyePathLength;

	if (!this->subpaths[index].visible)
	{
		color.init(0.0f);
	}
	else
	{
		if (eyeId == 0 && lightId == 0)
		{
			//color = this->subpaths[0].attenuation;
			color.init(0.0f);
		}
		else if (eyeId == 0)
		{
			/*
			color =
				this->lightPath[lightId - 1].attenuation *
				this->subpaths[index].attenuation;*/
			color.init(0.0f);
		}
		else if (lightId == 0)
		{
			color =
				this->eyePath[eyeId - 1].attenuation *
				this->subpaths[index].attenuation;
		}
		else
		{
			color =
				this->eyePath[eyeId - 1].attenuation *
				this->lightPath[lightId - 1].attenuation *
				this->subpaths[index].attenuation;
		}
	}

	return color;
}
