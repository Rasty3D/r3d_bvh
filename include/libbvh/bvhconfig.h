/*
 * bvhconfig.h
 *
 *  Created on: 03 May 2011
 *      Author: showroom
 */

#ifndef BVHCONFIG_H_
#define BVHCONFIG_H_


/*
 * INCLUDES
 */

#include <iostream>
#include <vector>

#include "nmlparser.h"


/*
 * DEFINES
 */

// Default values
#define BVHCONFIG_WIDTH_IMAGE	512
#define BVHCONFIG_HEIGHT_IMAGE	512
#define BVHCONFIG_WIDTH_BUCKET	128
#define BVHCONFIG_HEIGHT_BUCKET	128
#define BVHCONFIG_SPP			10
#define BVHCONFIG_LIGHT_DEPTH	4
#define BVHCONFIG_EYE_DEPTH		4
#define BVHCONFIG_BLUR_RADIUS	0.5f

#define BVHCONFIG_THREADS_TREE	1
#define BVHCONFIG_DEVICEID		0
#define BVHCONFIG_THREADS_GPU	256
#define BVHCONFIG_THREADS_CPU	1

// MODES
#define BVH_MODE_CPU	0
#define BVH_MODE_GPU	1


/*
 * TYPES
 */

typedef struct
{
	int mode;
	int id;

	// For GPU mode only
	int deviceId;
	int nThreadsGPU;

	// For CPU & GPU mode
	int nThreadsCPU;
}BVHRenderUnit;


/*
 * Class: BVHConfig
 */

class BVHConfig
{
private:
	int widthImage;
	int heightImage;

	int widthBucket;
	int heightBucket;

	int spp;

	int lightDepth;
	int eyeDepth;
	float blurRadius;

	int nThreadsTree;

	std::vector<BVHRenderUnit> renderUnits;

public:
	BVHConfig();
	~BVHConfig();

	int load(const char *filename);
private:
	int loadRenderUnit(NMLParser &parser, int mode, BVHRenderUnit &renderUnit);
public:
	int save(const char *filename);

	int getWidthImage();
	int getHeightImage();
	int getWidthBucket();
	int getHeightBucket();
	int getSPP();
	int getLightDepth();
	int getEyeDepth();
	float getBlurRadius();
	int getNThreadsTree();
	unsigned int getNRenderUnits();
	BVHRenderUnit *getRenderUnit(unsigned int id);
	std::vector<BVHRenderUnit> *getRenderUnits();
};

#endif /* BVHCONFIG_H_ */
