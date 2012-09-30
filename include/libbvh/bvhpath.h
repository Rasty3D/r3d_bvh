/*
 * bvhpath.h
 *
 *  Created on: 21 Mar 2011
 *      Author: showroom
 */

#ifndef BVHPATH_H_
#define BVHPATH_H_

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

#define BVHPATH_EYE		0
#define BVHPATH_LIGHT	1

#define BVHPATH_NOPRIMITIVE	0xFFFFFFFF	// It should be the same value as BVHGPU_NOPRIMITIVE defined in "bvhgpu.h"


/*
 * TYPES
 */

typedef struct
{
	float position[3];
	float normal[3];
	BVHColor *nColor;	// Transfer color in the node (BDRF, BDTF)
	BVHColor *sColor;	// Transfer color in the segment (occlussion or media)
	unsigned int primitiveLocation;
}BVHPath_segment;


/*
 * CLASS: BVHPath
 */

class BVHPath
{
public:
		/* Default segment lenghts */
	static int eyeSegmentLength;
	static int lightSegmentLength;

private:
		/* Segments */
	int nEyeSegments;
	int nEyeSegmentsCut;
	BVHPath_segment *eyeSegments;
	int nLightSegments;
	int nLightSegmentsCut;
	BVHPath_segment *lightSegments;

public:
		/* Constructor and destructor */
	BVHPath();
	~BVHPath();

		/* Init path */
	void init(int eyeSegmentLength, int lightSegmentLength);
	void reset();

		/* Set default segment lenghts */
	static void setDefaultSegmentLengths(int eyeSegmentLength, int lightSegmentLength);

		/* Get lenghts */
	int getLenght(int type);

		/* Set values */
	int setNodePositionNormal(int type, int id, float *position, float *normal);

		/* Manage eye segment length */
	// Reset eye and light lengths
	void resetLengths();

	// Cut eye length
	void cutEyeLength(int id);

	// Cut light length
	void cutLightLength(int id);

		/* Subpath operations */
	// Get the number of subpaths
	int getSubpathNumber();

	// Get the subpath step
	int getSubpathStep();

	// Get a subpath in a sequence
	int getSubpathPositions(int id, float *posEyePrev, float *posEye, float *posLight, float *posLightNext);

	// Set subpath colors
	int setSubpathColors(int id, BVHColor &nodeEye, BVHColor &nodeLight, BVHColor &segment);

	// Get subpath node id
	int getSubpathNodeId(int id, int type);

	// Get the number of subpaths with the same length
	int getSubpathNumberSameLength(int id);

		/* Path operations */
	// Get path positions
	int getPathPositions(int type, int id, float *posPrev, float *pos, float *posNext);

	// Get path direction in (from eye to light in the eye rays and from light to eye in the light rays)
	int getPathDirectionIn(int type, int id, float *dir);

	// Get path direction out (from eye to light in the eye rays and from light to eye in the light rays)
	int getPathDirectionOut(int type, int id, float *dir);

	// Set path colors
	int setPathColors(int type, int id, BVHColor *node, BVHColor *segment);

		/* Primitive operations */
	// Set node primitive location
	int setPrimitiveLocation(int type, int id, unsigned int primitiveLocation);

	// Get node primitive location
	unsigned int getPrimitiveLocation(int type, int id);

		/* Get total contribution of the path */
	BVHColor getColor();
	BVHColor getColorSubpath(int idEye, int idLight);
	//BVHColor getColorSubpath(int id);

		/* Aux functions */
	// Print node data
	void printNodeData();

	// Print node colors
	void printNodeColors();

private:
		/* Destroy path */
	void destroy();

		/* G function */
	float Gfunc(BVHPath_segment *n1, BVHPath_segment *n2);
};

#endif /* BVHPATH_H_ */
