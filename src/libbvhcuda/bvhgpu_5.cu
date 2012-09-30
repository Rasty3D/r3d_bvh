/*
 * bvhgpu.cu
 *
 *  Created on: 07 Feb 2011
 *      Author: showroom
 */


/*
 * INCLUDES
 */

	/* CUDA */
#include <cuda.h>

	/* BVH */
#include "bvhcudainline.h"
#include "bvhgpu.h"


/*
 * TEXTURES
 */

texture<float4, 1, cudaReadModeElementType> nodeTex_min;
texture<float4, 1, cudaReadModeElementType> nodeTex_max;
texture<uint2, 1, cudaReadModeElementType> nodeTex_skipLocation;


/*
 * NAMESPACES
 */

using namespace std;


/*
 * FUNCTIONS
 */

__device__ int cu_BVHgpu_primitiveIntersection(BVHgpu_ray *ray, unsigned char *primitive)
{
	// Variables
	float t;
	float pi[3];
	BVHPrimitive_triangle *triangle;
	BVHgpu_primitiveHeader *header = (BVHgpu_primitiveHeader*)primitive;
	unsigned char *data = primitive + sizeof(BVHgpu_primitiveHeader);

	if (header->type == BVHPRIMITIVE_TYPE_TRIANGLE)
	{
		// Get triangle data
		triangle = (BVHPrimitive_triangle*)data;

		// Calculate t
		t = ((triangle->v1[0] - ray->pos[0]) * triangle->normal[0] +
			 (triangle->v1[1] - ray->pos[1]) * triangle->normal[1] +
			 (triangle->v1[2] - ray->pos[2]) * triangle->normal[2]) /
			(ray->dir[0] * triangle->normal[0] +
			 ray->dir[1] * triangle->normal[1] +
			 ray->dir[2] * triangle->normal[2]);

		// Check direction
		if (t < 0.001f)
			return 0;

		if (ray->distance != -1.0f && t > ray->distance)
			return 0;

		// Check intersection
		pi[0] = ray->pos[0] + ray->dir[0] * t;
		pi[1] = ray->pos[1] + ray->dir[1] * t;
		pi[2] = ray->pos[2] + ray->dir[2] * t;

		if (((pi[0] - triangle->v1[0]) * triangle->n12[0] +
			 (pi[1] - triangle->v1[1]) * triangle->n12[1] +
			 (pi[2] - triangle->v1[2]) * triangle->n12[2]) < 0.0f)
			return 0;

		if (((pi[0] - triangle->v2[0]) * triangle->n23[0] +
			 (pi[1] - triangle->v2[1]) * triangle->n23[1] +
			 (pi[2] - triangle->v2[2]) * triangle->n23[2]) < 0.0f)
			return 0;

		if (((pi[0] - triangle->v3[0]) * triangle->n31[0] +
			 (pi[1] - triangle->v3[1]) * triangle->n31[1] +
			 (pi[2] - triangle->v3[2]) * triangle->n31[2]) < 0.0f)
			return 0;

		ray->distance = t;
		return 1;
	}

	return 0;
}

__device__ int cu_BVHgpu_aabbIntersection(int nodeIndex, BVHgpu_ray *ray, float *dirInv)
{
	// Calculate intersection
	float tpos[3];
	float tneg[3];
	float tmax[3];
	float tmin[3];
	float lmin;
	float lmax;
	float4 aabbmin = tex1Dfetch(nodeTex_min, nodeIndex);
	float4 aabbmax = tex1Dfetch(nodeTex_max, nodeIndex);

	tpos[0] = (aabbmax.x - ray->pos[0]) * dirInv[0];
	tpos[1] = (aabbmax.y - ray->pos[1]) * dirInv[1];
	tpos[2] = (aabbmax.z - ray->pos[2]) * dirInv[2];
	tneg[0] = (aabbmin.x - ray->pos[0]) * dirInv[0];
	tneg[1] = (aabbmin.y - ray->pos[1]) * dirInv[1];
	tneg[2] = (aabbmin.z - ray->pos[2]) * dirInv[2];

	tmax[0] = max(tpos[0], tneg[0]);
	tmax[1] = max(tpos[1], tneg[1]);
	tmax[2] = max(tpos[2], tneg[2]);
	tmin[0] = min(tpos[0], tneg[0]);
	tmin[1] = min(tpos[1], tneg[1]);
	tmin[2] = min(tpos[2], tneg[2]);

	lmin = max(tmin[0], max(tmin[1], tmin[2]));
	lmax = min(tmax[0], min(tmax[1], tmax[2]));

	if (ray->distance == -1.0f)
		return lmax > lmin && lmax > 0.0f;
	else
		return lmax > lmin && lmax > 0.0f && lmin < ray->distance;
}

__global__ void cu_BVHgpu_trace(const unsigned int gpu_nNodes, BVHgpu_ray *rays, const int nRays, unsigned char *primitives)
{
	// Normalize ray
	float rayModule;
	float dirInv[3];
	unsigned int nodeIndex = 0;
	int idx = threadIdx.x + blockIdx.x * blockDim.x;
	uint2 skipLocation;

	if (idx >= nRays)
		return;

	if (rays[idx].distance == -2.0f)
		return;

	rayModule = sqrtf(
		rays[idx].dir[0] * rays[idx].dir[0] +
		rays[idx].dir[1] * rays[idx].dir[1] +
		rays[idx].dir[2] * rays[idx].dir[2]);
	rays[idx].dir[0] /= rayModule;
	rays[idx].dir[1] /= rayModule;
	rays[idx].dir[2] /= rayModule;

	// Inverse ray
	dirInv[0] = 1.0f / rays[idx].dir[0];
	dirInv[1] = 1.0f / rays[idx].dir[1];
	dirInv[2] = 1.0f / rays[idx].dir[2];

	// Tree traversal
	while (nodeIndex < gpu_nNodes)
	{
		skipLocation = tex1Dfetch(nodeTex_skipLocation, nodeIndex);

		if (cu_BVHgpu_aabbIntersection(nodeIndex, &rays[idx], dirInv))
		{
			if (skipLocation.y != BVHGPU_NOPRIMITIVE)
			{
				if (cu_BVHgpu_primitiveIntersection(&rays[idx], &primitives[skipLocation.y]))
					rays[idx].closestNode = nodeIndex;
			}

			nodeIndex++;
		}
		else
		{
			nodeIndex = skipLocation.x;
		}
	}
}

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
	BVHgpu_stats *stats)
{
		/* Variables */

	BVHgpu_renderState state;

	int pos;

	dim3 threads(1, 1);
	dim3 grid(1, 1);

	unsigned char *cu_gpu_primitives;
	BVHgpu_ray *cu_rays;
	float *cu_gpu_nodesMin;
	float *cu_gpu_nodesMax;
	unsigned int *cu_gpu_nodesSkipLocation;

	struct timespec time0, time1;

	BVHgpu_threadData *threadDataCPU = new BVHgpu_threadData[nThreadsCPU];
	pthread_t *threadsCPU = new pthread_t[nThreadsCPU];


		/* Configure textures */

	// Min
	nodeTex_min.normalized = 0;
	nodeTex_min.filterMode = cudaFilterModePoint;
	nodeTex_min.addressMode[0] = cudaAddressModeClamp;

	// Max
	nodeTex_max.normalized = 0;
	nodeTex_max.filterMode = cudaFilterModePoint;
	nodeTex_max.addressMode[0] = cudaAddressModeClamp;

	// Skip index and primitive location
	nodeTex_skipLocation.normalized = 0;
	nodeTex_skipLocation.filterMode = cudaFilterModePoint;
	nodeTex_skipLocation.addressMode[0] = cudaAddressModeClamp;


		/* Init stats */

	stats->timeInit = 0.0;
	stats->timeAllocation = 0.0;
	stats->timeHostToDevice = 0.0;
	stats->timeRaysToDevice = 0.0;
	stats->timeRaysToHost = 0.0;
	stats->timeKernel = 0.0;
	stats->timeOthers = 0.0;
	stats->timeBindTextures = 0.0;
	stats->timeGenRays = 0.0;
	stats->timeFillBuffer = 0.0;


		/* Init things */

	clock_gettime(CLOCK_REALTIME, &time0);

	// Bucket
	state.bucket = *bucket;
	state.bucketSize = state.bucket.size[0] * state.bucket.size[1];

	// Paths
	state.eyeSegmentLength = eyeDepth;
	state.lightSegmentLength = lightDepth;
	state.nRaysMax = state.bucketSize * state.eyeSegmentLength;
	state.paths = new BVHBidi[state.bucketSize];

	// Rays
	state.rays[0] = new BVHgpu_ray[state.nRaysMax];
	state.rays[1] = new BVHgpu_ray[state.nRaysMax];
	state.nRays[0] = 0;
	state.nRays[1] = 0;

	// Primitives
	state.primitives = gpu_primitives;
	state.nodesSkipLocation = gpu_nodesSkipLocation;

	// Dimensions
	state.width = width;
	state.height = height;

	// Scene
	state.scene = scene;

	// Render buffer
	state.renderBuffer = renderBuffer;

	// Samples per pixel
	state.samplesPerPixel = samplesPerPixel;

	// Blur radius
	state.blurRadius = blurRadius;

	// Render buffer
	for (int x = state.bucket.pos[0]; x < state.bucket.pos[0] + state.bucket.size[0]; x++)
	{
		for (int y = state.bucket.pos[1]; y < state.bucket.pos[1] + state.bucket.size[1]; y++)
		{
			pos = x + y * width;

			// Reset buffer
			renderBuffer[pos].rgb[0] = 0.0f;
			renderBuffer[pos].rgb[1] = 0.0f;
			renderBuffer[pos].rgb[2] = 0.0f;
			renderBuffer[pos].alpha = 0.0f;
		}
	}

	clock_gettime(CLOCK_REALTIME, &time1);
	stats->timeInit += (double)(time1.tv_sec - time0.tv_sec) + 1E-9 * (double)(time1.tv_nsec - time0.tv_nsec);


		/* Copy data to GPU memory */

	// Memory allocation
	clock_gettime(CLOCK_REALTIME, &time0);
	cuda_safeCall(cudaMalloc((void**)&cu_gpu_primitives, gpu_primitiveSize));
	cuda_safeCall(cudaMalloc((void**)&cu_gpu_nodesMin, gpu_nNodes * 4 * sizeof(float)));
	cuda_safeCall(cudaMalloc((void**)&cu_gpu_nodesMax, gpu_nNodes * 4 * sizeof(float)));
	cuda_safeCall(cudaMalloc((void**)&cu_gpu_nodesSkipLocation, gpu_nNodes * 2 * sizeof(unsigned int)));
	cuda_safeCall(cudaMalloc((void**)&cu_rays, sizeof(BVHgpu_ray) * state.nRaysMax));
	clock_gettime(CLOCK_REALTIME, &time1);
	stats->timeAllocation += (double)(time1.tv_sec - time0.tv_sec) + 1E-9 * (double)(time1.tv_nsec - time0.tv_nsec);

	// Copy data to graphic card
	clock_gettime(CLOCK_REALTIME, &time0);
	cuda_safeCall(cudaMemcpy(cu_gpu_primitives, gpu_primitives, gpu_primitiveSize, cudaMemcpyHostToDevice));
	cuda_safeCall(cudaMemcpy(cu_gpu_nodesMin, gpu_nodesMin, gpu_nNodes * 4 * sizeof(float), cudaMemcpyHostToDevice));
	cuda_safeCall(cudaMemcpy(cu_gpu_nodesMax, gpu_nodesMax, gpu_nNodes * 4 * sizeof(float), cudaMemcpyHostToDevice));
	cuda_safeCall(cudaMemcpy(cu_gpu_nodesSkipLocation, gpu_nodesSkipLocation, gpu_nNodes * 2 * sizeof(unsigned int), cudaMemcpyHostToDevice));
	clock_gettime(CLOCK_REALTIME, &time1);
	stats->timeHostToDevice += (double)(time1.tv_sec - time0.tv_sec) + 1E-9 * (double)(time1.tv_nsec - time0.tv_nsec);

	// Bind data to textures
	clock_gettime(CLOCK_REALTIME, &time0);
	cudaChannelFormatDesc channelDescMin = cudaCreateChannelDesc<float4>();
	cudaBindTexture(NULL, nodeTex_min, cu_gpu_nodesMin, channelDescMin, gpu_nNodes * 4 * sizeof(float));
	cudaChannelFormatDesc channelDescMax = cudaCreateChannelDesc<float4>();
	cudaBindTexture(NULL, nodeTex_max, cu_gpu_nodesMax, channelDescMax, gpu_nNodes * 4 * sizeof(float));
	cudaChannelFormatDesc channelDescSkipLocation = cudaCreateChannelDesc<uint2>();
	cudaBindTexture(NULL, nodeTex_skipLocation, cu_gpu_nodesSkipLocation, channelDescSkipLocation, gpu_nNodes * 2 * sizeof(unsigned int));
	clock_gettime(CLOCK_REALTIME, &time1);
	stats->timeBindTextures += (double)(time1.tv_sec - time0.tv_sec) + 1E-9 * (double)(time1.tv_nsec - time0.tv_nsec);


		/* Launch threads */

	BVHgpu_pathtracing_launchThreads(&state, nThreadsCPU, threadDataCPU, threadsCPU);


		/* Render bucket */

	for (int s = 0; s < samplesPerPixel; s++)
	{
		// Init phase
		state.phase = PHASE_EYE_RAY;
		state.eyeDepth = 0;
		state.lightDepth = 0;
		state.subpathId = 0;
		state.subpathMax = state.paths[0].getSubpathNumber();
		state.subpathStep = state.paths[0].getSubpathStep();
		state.rayBuffer = 0;

		// Reset paths
		for (pos = 0; pos < state.bucketSize; pos++)
			state.paths[pos].reset();

		while (1)
		{
			// No in the first step (there is no rays in the buffer)
			if (state.nRays[!state.rayBuffer] > 0)
			{
				// Configure the thread grid
				threads.x = nThreadsGPU;
				grid.x = iDivUp(state.nRays[!state.rayBuffer], nThreadsGPU);

				// Copy rays to GPU
				clock_gettime(CLOCK_REALTIME, &time0);
				cuda_safeCall(cudaMemcpy(cu_rays, state.rays[!state.rayBuffer], sizeof(BVHgpu_ray) * state.nRays[!state.rayBuffer], cudaMemcpyHostToDevice));
				clock_gettime(CLOCK_REALTIME, &time1);
				stats->timeRaysToDevice += (double)(time1.tv_sec - time0.tv_sec) + 1E-9 * (double)(time1.tv_nsec - time0.tv_nsec);

				// Run kernel
				clock_gettime(CLOCK_REALTIME, &time0);
				cu_BVHgpu_trace<<<grid, threads>>>(gpu_nNodes, cu_rays, state.nRays[!state.rayBuffer], cu_gpu_primitives);
				clock_gettime(CLOCK_REALTIME, &time1);
				stats->timeKernel += (double)(time1.tv_sec - time0.tv_sec) + 1E-9 * (double)(time1.tv_nsec - time0.tv_nsec);
			}

			clock_gettime(CLOCK_REALTIME, &time0);
			if (!BVHgpu_pathtracing(&state, nThreadsCPU, threadDataCPU, threadsCPU))
				break;
			clock_gettime(CLOCK_REALTIME, &time1);
			stats->timeGenRays += (double)(time1.tv_sec - time0.tv_sec) + 1E-9 * (double)(time1.tv_nsec - time0.tv_nsec);

			// No in the first step (there is no rays in the buffer)
			if (state.nRays[!state.rayBuffer] > 0)
			{
				// Synchronize to wait for the kernel to finish
				clock_gettime(CLOCK_REALTIME, &time0);
				cudaThreadSynchronize();
				clock_gettime(CLOCK_REALTIME, &time1);
				stats->timeKernel += (double)(time1.tv_sec - time0.tv_sec) + 1E-9 * (double)(time1.tv_nsec - time0.tv_nsec);

				// Copy rays from GPU
				clock_gettime(CLOCK_REALTIME, &time0);
				cuda_safeCall(cudaMemcpy(state.rays[!state.rayBuffer], cu_rays, sizeof(BVHgpu_ray) * state.nRays[!state.rayBuffer], cudaMemcpyDeviceToHost));
				clock_gettime(CLOCK_REALTIME, &time1);
				stats->timeRaysToHost += (double)(time1.tv_sec - time0.tv_sec) + 1E-9 * (double)(time1.tv_nsec - time0.tv_nsec);
			}

			// Swap buffers
			state.rayBuffer = !state.rayBuffer;

			// Next phase
			clock_gettime(CLOCK_REALTIME, &time0);
			if (state.phase == PHASE_EYE_RAY)
			{
				state.phase = PHASE_LIGHT_RAY;
				state.eyeDepth++;

				if (state.eyeDepth >= state.eyeSegmentLength && state.lightDepth >= state.lightSegmentLength)
					state.phase = PHASE_SUBPATH;
			}
			else if (state.phase == PHASE_LIGHT_RAY)
			{
				state.phase = PHASE_EYE_RAY;
				state.lightDepth++;

				if (state.eyeDepth >= state.eyeSegmentLength && state.lightDepth >= state.lightSegmentLength)
					state.phase = PHASE_SUBPATH;
			}
			else if (state.phase == PHASE_SUBPATH)
			{
				state.subpathId += state.subpathStep;

				if (state.subpathId > state.subpathMax + state.subpathStep)
				{
					clock_gettime(CLOCK_REALTIME, &time1);
					stats->timeOthers += (double)(time1.tv_sec - time0.tv_sec) + 1E-9 * (double)(time1.tv_nsec - time0.tv_nsec);
					break;
				}
			}
			else
			{
				break;
			}
			clock_gettime(CLOCK_REALTIME, &time1);
			stats->timeOthers += (double)(time1.tv_sec - time0.tv_sec) + 1E-9 * (double)(time1.tv_nsec - time0.tv_nsec);
		}

		// Fill buffer
		clock_gettime(CLOCK_REALTIME, &time0);
		state.phase = PHASE_FILL_BUFFER;
		if (!BVHgpu_pathtracing(&state, nThreadsCPU, threadDataCPU, threadsCPU))
			break;
		clock_gettime(CLOCK_REALTIME, &time1);
		stats->timeFillBuffer += (double)(time1.tv_sec - time0.tv_sec) + 1E-9 * (double)(time1.tv_nsec - time0.tv_nsec);
	}


		/* Stop threads */

	BVHgpu_pathtracing_stopThreads(&state, nThreadsCPU, threadDataCPU, threadsCPU);


		/* Calculate stats */

    stats->timeMemory = stats->timeAllocation + stats->timeHostToDevice + stats->timeBindTextures + stats->timeRaysToDevice + stats->timeRaysToHost;
    stats->timeCPU = stats->timeInit + stats->timeGenRays + stats->timeFillBuffer + stats->timeOthers;
    stats->timeTotal = stats->timeKernel + stats->timeCPU + stats->timeMemory;


    	/* Free memory */

    cuda_safeCall(cudaFree(cu_gpu_nodesMin));
    cuda_safeCall(cudaFree(cu_gpu_nodesMax));
    cuda_safeCall(cudaFree(cu_gpu_nodesSkipLocation));
	cuda_safeCall(cudaFree(cu_gpu_primitives));
	cuda_safeCall(cudaFree(cu_rays));
	delete [] state.paths;
	delete [] state.rays[0];
	delete [] state.rays[1];
	delete [] threadDataCPU;
	delete [] threadsCPU;
}
