/*
 * bvh.h
 *
 *  Created on: 19 Jan 2011
 *      Author: showroom
 */

#ifndef BVH_H_
#define BVH_H_

/*
 * INCLUDES
 */

#include <stdlib.h>
#include <math.h>
#include <list>
#include <iostream>

#include "bvhaabb.h"
#include "bvhcamera.h"
#include "bvhconfig.h"
#include "bvhcuda.h"
#include "bvhgpu.h"
#include "bvhnode.h"
#include "bvhprimitive.h"
#include "bvhray.h"
#include "bvhscene.h"
#include "bvhtexture.h"


/*
 * DEFINES
 */




/*
 * TYPES
 */

typedef BVHPrimitive *BVHPrimitivep;

typedef struct
{
	int bucketSize;
	BVHgpu_bucket *buckets;
	int bucketCurrent;
	pthread_mutex_t bucketMutex;
}BVHBucketList;

typedef struct
{
	// Render unit
	BVHRenderUnit *renderUnit;

	// Bucket list
	BVHBucketList *bucketList;

	// Scene info
	unsigned int nPrimitives;
	unsigned int *primitiveLocations;

	unsigned int primitiveSize;
	unsigned char *primitives;

	unsigned int nNodes;
	float *nodesMin;
	float *nodesMax;
	unsigned int *nodesSkipLocation;

	int width;
	int height;
	BVH_gpuRenderBuffer *renderBuffer;
	BVHScene *scene;
	int samplesPerPixel;
	int lightDepth;
	int eyeDepth;
	float blurRadius;

	struct timespec *timeInitRendering;
}BVHRenderUnitInfo;


/*
 * CLASS: BVH
 */

class BVH
{
private:
		/* Scene */
	BVHScene scene;

		/* GPU frienly lists */
	unsigned int gpu_nPrimitives;
	unsigned int *gpu_primitiveLocations;

	unsigned int gpu_primitiveSize;
	unsigned char *gpu_primitives;

	unsigned int gpu_nNodes;
	float *gpu_nodesMin;
	float *gpu_nodesMax;
	unsigned int *gpu_nodesSkipLocation;

		/* Bucket list */
	int nBuckets;
	BVHgpu_bucket *buckets;

		/* Time init rendering */
	struct timespec timeInitRendering;

		/* Verbose */
	static int verbose;


public:
		/* Constructor and destructor */
	BVH();
	~BVH();

		/* Init function */
	static void init(const char *colorSystem = NULL);

		/* Load scene */
	int load(const char *filename);

		/* Build tree */
	void buildTree(int nThreads = 1);

private:
		/* Generate GPU friendly lists */
	void generateGPUlists(int nodeStaticNumber, BVHNodeStatic *nodeStaticList);

public:
		/* Render */
	void render(
		unsigned int width, unsigned int height,
		BVH_gpuRenderBuffer *renderBuffer, int samplesPerPixel,
		int lightDepth, int eyeDepth, float blurRadius,
		int widthBucket, int heightBucket,
		int nRenderUnits, BVHRenderUnit *renderUnits);
private:
	static void *renderThread(void *renderUnitInfo);

public:
		/* Print time */
	static void printTime(double time, int miliseconds = 0);

		/* Set verbose */
	static void setVerbose(int verbose);
};

#endif /* BVH_H_ */
