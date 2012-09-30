/*
 * bvhbidi.h
 *
 *  Created on: 17 May 2011
 *      Author: showroom
 */

#ifndef BVHBIDI_H_
#define BVHBIDI_H_

/*
 * INCLUDES
 */

#include <stdlib.h>
#include <iostream>

#include "mathaux.h"

#include "bvhcolor.h"


/*
 * DEFINES
 */

#define BVHBIDI_EYE		0
#define BVHBIDI_LIGHT	1

#define BVHBIDI_NOPRIMITIVE	0xFFFFFFFF	// It should be the same value as BVHGPU_NOPRIMITIVE defined in "bvhgpu.h"

#define BVHBIDI_EYE_LENGTH_DEFAULT		4
#define BVHBIDI_LIGHT_LENGTH_DEFAULT	4


/*
 * TYPES
 */

typedef struct
{
	float position[3];
	float normal[3];
	unsigned int primitiveLocation;
	BVHColor attenuation;
}BVHBidi_node;

typedef struct
{
	int visible;
	BVHColor attenuation;
	float factor;
}BVHBidi_subpath;


/*
 * CLASS: BVHBidi
 */

class BVHBidi
{
private:
		/* Maximum path lenghts */
	static int eyePathLengthMax;
	static int lightPathLengthMax;

		/* Paths */
	int eyePathLength;
	int eyePathLengthCut;
	BVHBidi_node *eyePath;

	int lightPathLength;
	int lightPathLengthCut;
	BVHBidi_node *lightPath;

		/* Sub-paths */
	BVHBidi_subpath *subpaths;

		/* Light directly to the camera */
	BVHColor directLight;

public:
		/* Constructor and destructor */
	BVHBidi();
	~BVHBidi();

		/* Set path maximum lenghts */
	static void setPathLengthMax(int eyePathLength, int lightPathLength);

		/* Init and destroy */
	void init(int eyePathLength, int lightPathLength);
	void destroy();

		/* Reset path lengths */
	void resetPathLengths();
	void reset();

		/* Get path lengths */
	int getPathLength(int type);
	int getPathLengthCut(int type);

		/* Set node info */
	int setNodePosition(int type, int index, float *position);
	int setNodePrimitiveLocation(int type, int index, unsigned int primitiveLocation);
	int setNodeNormal(int type, int index, float *normal);
	int setNodeAttenuation(
		int type, int index,
		BVHColor &media, BVHColor &bsdf, BVHColor &emission,
		float pdf, float *outDirection);
	int setNodeMedia(int type, int index, BVHColor &media);

		/* Get node info */
	int getNodePosition(int type, int index, float *position);
	unsigned int getNodePrimitiveLocation(int type, int index);

		/* Subpath management */
	int getSubpathNumber();
	int getSubpathStep();
	int getSubpathVertices(int index, float *pe, float *pl);
	int isSubpathEye(int index);
	int isSubpathLight(int index);
	int getSubpathInfo(int index, float *ne, float *nl, float *de, float *dl, unsigned int &pe, unsigned int &pl);
	int setSubpathColors(int index, int visible, BVHColor &media, BVHColor &bsdfEye, BVHColor &bsdfLight);

		/* Get path color */
	BVHColor getColor();
	BVHColor getColor(int eyeId, int lightId);
};

#endif /* BVHBIDI_H_ */
