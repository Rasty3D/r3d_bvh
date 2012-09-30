/*
 * bvhgpu.h
 *
 *  Created on: 04 Feb 2011
 *      Author: showroom
 */

#ifndef BVHGPU_H_
#define BVHGPU_H_

/*
 * INCLUDES
 */

	/* General */
#include <iostream>
#include <math.h>
#include <pthread.h>

	/* Utils */
#include "utils.h"

	/* BVH */
#include "bvhbidi.h"
#include "bvhcamera.h"
#include "bvhprimitive.h"
#include "bvhshader.h"


/*
 * DEFINES
 */

	/* No primitive flag */
#define BVHGPU_NOPRIMITIVE	0xFFFFFFFF

	/* Phases */
#define PHASE_EYE_RAY		0
#define PHASE_LIGHT_RAY		1
#define PHASE_SUBPATH		2
#define PHASE_INTERSECTION	3
#define PHASE_FILL_BUFFER	4

	/* Thread states */
#define BVHGPU_THREAD_WAIT	0
#define BVHGPU_THREAD_RUN	1
#define BVHGPU_THREAD_STOP	2


/*
 * TYPES
 */

typedef struct
{
	unsigned int type;
	void *primitive;
}BVHgpu_primitiveHeader;

typedef struct
{
	float pos[3];
	float dir[3];
	unsigned int closestNode;
	float distance;
}BVHgpu_ray;

typedef struct
{
	float rgb[3];
	float alpha;
}BVH_gpuRenderBuffer;

typedef struct
{
	int size[2];	// Size of the bucket
	int pos[2];		// Position (corner) of the actual bucket
}BVHgpu_bucket;

typedef struct
{
	int phase;
	int eyeDepth;
	int lightDepth;
	int eyeSegmentLength;
	int lightSegmentLength;
	int nRaysMax;
	int subpathId;
	int subpathMax;
	int subpathStep;

	BVHgpu_bucket bucket;
	int bucketSize;

	int rayBuffer;
	int nRays[2];
	BVHgpu_ray *rays[2];

	//BVHPath *paths;
	BVHBidi *paths;

	unsigned int nNodes;
	float *nodesMin;
	float *nodesMax;
	unsigned char *primitives;
	unsigned int *nodesSkipLocation;

	unsigned int width;
	unsigned int height;

	void *scene;

	BVH_gpuRenderBuffer *renderBuffer;
	int samplesPerPixel;
	float blurRadius;
}BVHgpu_renderState;

typedef struct
{
	BVHgpu_renderState *renderState;
	int threadState;
	int threadNumber;
	int threadId;
	int ret;
	gsl_rng *rng;
}BVHgpu_threadData;

typedef struct
{
	double timeTotal;

    double timeCPU;
    double timeInit;
    double timeGenRays;
    double timeIntersection;
    double timeFillBuffer;
    double timeOthers;

    double timeMemory;
    double timeAllocation;
    double timeHostToDevice;
    double timeBindTextures;
    double timeRaysToDevice;
    double timeRaysToHost;

    double timeKernel;
}BVHgpu_stats;


/*
 * FUNCTIONS
 */

	/* CPU version */
void BVHgpu_render(
	// Scene definition
	unsigned int gpu_nPrimitives, unsigned int *gpu_primitiveLocations,
	unsigned int gpu_primitiveSize, unsigned char *gpu_primitives,
	unsigned int gpu_nNodes, float *gpu_nodesMin, float *gpu_nodesMax, unsigned int *gpu_nodesSkipLocation,
	// Render buffer
	unsigned int width, unsigned int height, BVH_gpuRenderBuffer *renderBuffer, void *scene, int samplesPerPixel,
	int lightDepth, int eyeDepth, float blurRadius,
	// Kernel control
	int nThreadsCPU, BVHgpu_bucket *bucket,
	// Stats
	BVHgpu_stats *stats);

	/* CUDA version */
void cu_BVHgpu_render(
	// Scene definition
	unsigned int gpu_nPrimitives, unsigned int *gpu_primitiveLocations,
	unsigned int gpu_primitiveSize, unsigned char *gpu_primitives,
	unsigned int gpu_nNodes, float *gpu_nodesMin, float *gpu_nodesMax, unsigned int *gpu_nodesSkipLocation,
	// Render buffer
	unsigned int width, unsigned int height, BVH_gpuRenderBuffer *renderBuffer, void *scene, int samplesPerPixel,
	int lightDepth, int eyeDepth, float blurRadius,
	// Kernel control
	int nThreadsGPU, int nThreadsCPU, BVHgpu_bucket *bucket,
	// Stats
	BVHgpu_stats *stats);

	/* Path tracing (Always CPU) */
void BVHgpu_pathtracing_launchThreads(
	BVHgpu_renderState *state, int nThreads,
	BVHgpu_threadData *threadData, pthread_t *threads);
void BVHgpu_pathtracing_stopThreads(
	BVHgpu_renderState *state, int nThreads,
	BVHgpu_threadData *threadData, pthread_t *threads);
int BVHgpu_pathtracing(
	BVHgpu_renderState *state, int nThreads,
	BVHgpu_threadData *threadData, pthread_t *threads);

#endif /* BVHGPU_H_ */
