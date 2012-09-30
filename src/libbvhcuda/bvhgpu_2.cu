/*
 * bvhgpu.cu
 *
 *  Created on: 07 Feb 2011
 *      Author: showroom
 */


/*
 * INCLUDES
 */

#include <cutil_inline.h>

#include "bvhgpu.h"


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
		if (t < 0.0f)
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

__device__ int cu_BVHgpu_aabbIntersection(BVHgpu_node *gpu_node, BVHgpu_ray *ray, float *dirInv)
{
	// Calculate intersection
	float tpos[3];
	float tneg[3];
	float tmax[3];
	float tmin[3];
	float lmin;
	float lmax;

	tpos[0] = (gpu_node->max[0] - ray->pos[0]) * dirInv[0];
	tpos[1] = (gpu_node->max[1] - ray->pos[1]) * dirInv[1];
	tpos[2] = (gpu_node->max[2] - ray->pos[2]) * dirInv[2];
	tneg[0] = (gpu_node->min[0] - ray->pos[0]) * dirInv[0];
	tneg[1] = (gpu_node->min[1] - ray->pos[1]) * dirInv[1];
	tneg[2] = (gpu_node->min[2] - ray->pos[2]) * dirInv[2];

	tmax[0] = (tpos[0] > tneg[0]) ? tpos[0] : tneg[0];
	tmax[1] = (tpos[1] > tneg[1]) ? tpos[1] : tneg[1];
	tmax[2] = (tpos[2] > tneg[2]) ? tpos[2] : tneg[2];
	tmin[0] = (tpos[0] < tneg[0]) ? tpos[0] : tneg[0];
	tmin[1] = (tpos[1] < tneg[1]) ? tpos[1] : tneg[1];
	tmin[2] = (tpos[2] < tneg[2]) ? tpos[2] : tneg[2];

	if (tmin[0] >= tmin[1] && tmin[0] >= tmin[2])
		lmin = tmin[0];
	else if (tmin[1] > tmin[0] && tmin[1] >= tmin[2])
		lmin = tmin[1];
	else
		lmin = tmin[2];

	if (tmax[0] <= tmax[1] && tmax[0] <= tmax[2])
		lmax = tmax[0];
	else if (tmax[1] < tmax[0] && tmax[1] <= tmax[2])
		lmax = tmax[1];
	else
		lmax = tmax[2];

	if (ray->distance == -1.0f)
		return lmax > lmin && lmax > 0.0f;
	else
		return lmax > lmin && lmax > 0.0f && lmin < ray->distance;
}

__global__ void cu_BVHgpu_trace(const unsigned int gpu_nNodes, BVHgpu_node *gpu_nodes, BVHgpu_ray *rays, unsigned char *primitives)
{
	// Normalize ray
	float rayModule;
	float dirInv[3];
	unsigned int nodeIndex = 0;
	int idx = threadIdx.x + blockIdx.x * blockDim.x;

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
		if (cu_BVHgpu_aabbIntersection(&gpu_nodes[nodeIndex], &rays[idx], dirInv))
		{
			if (gpu_nodes[nodeIndex].primitiveLocation != BVHGPU_NOPRIMITIVE)
			{
				if (cu_BVHgpu_primitiveIntersection(&rays[idx], &primitives[gpu_nodes[nodeIndex].primitiveLocation]))
					rays[idx].closestNode = nodeIndex;
			}

			nodeIndex++;
		}
		else
		{
			nodeIndex = gpu_nodes[nodeIndex].skipIndex;
		}
	}
}

inline int iDivUp(int a, int b)
{
    return ((a % b) != 0) ? (a / b + 1) : (a / b);
}

void cu_BVHgpu_render(
	// Scene definition
	unsigned int gpu_nPrimitives, unsigned int *gpu_primitiveLocations,
	unsigned int gpu_primitiveSize, unsigned char *gpu_primitives,
	unsigned int gpu_nNodes, BVHgpu_node *gpu_nodes,
	// Render buffer
	unsigned int width, unsigned int height, BVH_gpuRenderBuffer *renderBuffer)
{
		/* Variables */

	int pos;
	int nRays;
	BVHgpu_ray *rays;
	int nThreads = 256;
	int nBlocks = 256;
	int nRaysParallel = nThreads * nBlocks;

	unsigned int *cu_gpu_primitiveLocations;
	unsigned char *cu_gpu_primitives;
	BVHgpu_node *cu_gpu_nodes;
	BVHgpu_ray *cu_rays;

	struct timespec time0, time1;
	double timeInit = 0.0;
	double timeAllocation = 0.0;
	double timeHostToDevice = 0.0;
	double timeRaysToDevice = 0.0;
	double timeRaysToHost = 0.0;
	double timeKernel = 0.0;
	double timeOthers = 0.0;
	double timeCPU;
	double timeTotal;
	double timeMemory;


		/* Init rays */

	clock_gettime(CLOCK_REALTIME, &time0);
	nRays = width * height;
	rays = new BVHgpu_ray[nRays];

	for (unsigned int x = 0; x < width; x++)
	{
		for (unsigned int y = 0; y < height; y++)
		{
			pos = x + y * width;

			// Init ray position
			rays[pos].pos[0] = 6.0f;
			rays[pos].pos[1] = 3.0f;
			rays[pos].pos[2] = 0.0f;

			// Generate ray direction
			rays[pos].dir[0] = -1.0f;
			rays[pos].dir[1] = ((float)y - 0.5f * (float)height) / (float)width;
			rays[pos].dir[2] = ((float)x - 0.5f * (float)width) / (float)width;

			/*
			// Init ray position
			rays[pos].pos[0] = 1.0f;
			rays[pos].pos[1] = 11.0f;
			rays[pos].pos[2] = 5.0f;

			// Generate ray direction
			rays[pos].dir[0] = 1.0f;
			rays[pos].dir[1] = ((float)x - 0.5f * (float)height) / (float)width;
			rays[pos].dir[2] = ((float)y - 0.5f * (float)width) / (float)width;*/

			// Reset distance
			rays[pos].distance = -1.0f;

			// Reset buffer
			renderBuffer[pos].rgb[0] = 0.0f;
			renderBuffer[pos].rgb[1] = 0.0f;
			renderBuffer[pos].rgb[2] = 0.0f;
			renderBuffer[pos].alpha = 0.0f;
		}
	}
	clock_gettime(CLOCK_REALTIME, &time1);
	timeInit += (double)(time1.tv_sec - time0.tv_sec) + 1E-9 * (double)(time1.tv_nsec - time0.tv_nsec);


		/* Copy data to GPU memory */

	// Memory allocation
	clock_gettime(CLOCK_REALTIME, &time0);
	cutilSafeCall(cudaMalloc((void**)&cu_gpu_primitiveLocations, gpu_nPrimitives * sizeof(unsigned int)));
	cutilSafeCall(cudaMalloc((void**)&cu_gpu_primitives, gpu_primitiveSize));
	cutilSafeCall(cudaMalloc((void**)&cu_gpu_nodes, sizeof(BVHgpu_node) * gpu_nNodes));
	cutilSafeCall(cudaMalloc((void**)&cu_rays, sizeof(BVHgpu_ray) * nRays));
	clock_gettime(CLOCK_REALTIME, &time1);
	timeAllocation += (double)(time1.tv_sec - time0.tv_sec) + 1E-9 * (double)(time1.tv_nsec - time0.tv_nsec);

	// Copy data to graphic card
	clock_gettime(CLOCK_REALTIME, &time0);
	cutilSafeCall(cudaMemcpy(cu_gpu_primitiveLocations, gpu_primitiveLocations, gpu_nPrimitives * sizeof(unsigned int), cudaMemcpyHostToDevice));
	cutilSafeCall(cudaMemcpy(cu_gpu_primitives, gpu_primitives, gpu_primitiveSize, cudaMemcpyHostToDevice));
	cutilSafeCall(cudaMemcpy(cu_gpu_nodes, gpu_nodes, sizeof(BVHgpu_node) * gpu_nNodes, cudaMemcpyHostToDevice));
	clock_gettime(CLOCK_REALTIME, &time1);
	timeHostToDevice += (double)(time1.tv_sec - time0.tv_sec) + 1E-9 * (double)(time1.tv_nsec - time0.tv_nsec);


		/* Launch threads */

	dim3 threads(nThreads, 1);
    dim3 grid(nBlocks, 1);

    for (int i = 0; i < nRays; i += nRaysParallel)
    {
    	if ((nRays - i) < nRaysParallel)
    	{
    		clock_gettime(CLOCK_REALTIME, &time0);
			cutilSafeCall(cudaMemcpy(cu_rays, &rays[i], sizeof(BVHgpu_ray) * (nRays - i), cudaMemcpyHostToDevice));
			clock_gettime(CLOCK_REALTIME, &time1);
			timeRaysToDevice += (double)(time1.tv_sec - time0.tv_sec) + 1E-9 * (double)(time1.tv_nsec - time0.tv_nsec);
    	}
    	else
    	{
			clock_gettime(CLOCK_REALTIME, &time0);
			cutilSafeCall(cudaMemcpy(cu_rays, &rays[i], sizeof(BVHgpu_ray) * nRaysParallel, cudaMemcpyHostToDevice));
			clock_gettime(CLOCK_REALTIME, &time1);
			timeRaysToDevice += (double)(time1.tv_sec - time0.tv_sec) + 1E-9 * (double)(time1.tv_nsec - time0.tv_nsec);
    	}

    	clock_gettime(CLOCK_REALTIME, &time0);
    	cu_BVHgpu_trace<<<grid, threads>>>(gpu_nNodes, cu_gpu_nodes, cu_rays, cu_gpu_primitives);
    	cudaThreadSynchronize();
    	clock_gettime(CLOCK_REALTIME, &time1);
    	timeKernel += (double)(time1.tv_sec - time0.tv_sec) + 1E-9 * (double)(time1.tv_nsec - time0.tv_nsec);

    	if ((nRays - i) < nRaysParallel)
    	{
    		clock_gettime(CLOCK_REALTIME, &time0);
			cutilSafeCall(cudaMemcpy(&rays[i], cu_rays, sizeof(BVHgpu_ray) * (nRays - i), cudaMemcpyDeviceToHost));
			clock_gettime(CLOCK_REALTIME, &time1);
			timeRaysToHost += (double)(time1.tv_sec - time0.tv_sec) + 1E-9 * (double)(time1.tv_nsec - time0.tv_nsec);
    	}
    	else
    	{
			clock_gettime(CLOCK_REALTIME, &time0);
			cutilSafeCall(cudaMemcpy(&rays[i], cu_rays, sizeof(BVHgpu_ray) * nRaysParallel, cudaMemcpyDeviceToHost));
			clock_gettime(CLOCK_REALTIME, &time1);
			timeRaysToHost += (double)(time1.tv_sec - time0.tv_sec) + 1E-9 * (double)(time1.tv_nsec - time0.tv_nsec);
    	}

    	clock_gettime(CLOCK_REALTIME, &time0);
    	for (int j = i; j < nRays && j < i + nRaysParallel; j++)
    	{
			if (rays[j].distance != -1.0f)
			{
				renderBuffer[j].rgb[0] = rays[j].distance * 0.03f;
				renderBuffer[j].rgb[1] = rays[j].distance * 0.03f;
				renderBuffer[j].rgb[2] = rays[j].distance * 0.03f;
				renderBuffer[j].alpha = 1.0f;
			}
    	}
    	clock_gettime(CLOCK_REALTIME, &time1);
    	timeOthers += (double)(time1.tv_sec - time0.tv_sec) + 1E-9 * (double)(time1.tv_nsec - time0.tv_nsec);
    }

    timeMemory = timeAllocation + timeHostToDevice + timeRaysToDevice + timeRaysToHost;
    timeCPU = timeInit + timeOthers;
    timeTotal = timeKernel + timeCPU + timeMemory;

    std::cout << "Time total             : " << timeTotal << std::endl;
    std::cout << "  Time CPU             : " << timeCPU << "\t(" << (100.0 * timeCPU / timeTotal) << "%)" << std::endl;
    std::cout << "    Time init          : " << timeInit << "\t(" << (100.0 * timeInit / timeTotal) << "%)" << std::endl;
    std::cout << "    Time others        : " << timeOthers << "\t(" << (100.0 * timeOthers / timeTotal) << "%)" << std::endl;
    std::cout << "  Time memory          : " << timeMemory << "\t(" << (100.0 * timeMemory / timeTotal) << "%)" << std::endl;
    std::cout << "    Time allocation    : " << timeAllocation << "\t(" << (100.0 * timeAllocation / timeTotal) << "%)" << std::endl;
    std::cout << "    Time host to device: " << timeHostToDevice << "\t(" << (100.0 * timeHostToDevice / timeTotal) << "%)" << std::endl;
    std::cout << "    Time rays to device: " << timeRaysToDevice << "\t(" << (100.0 * timeRaysToDevice / timeTotal) << "%)" << std::endl;
    std::cout << "    Time rays to host  : " << timeRaysToHost << "\t(" << (100.0 * timeRaysToHost / timeTotal) << "%)" << std::endl;
    std::cout << "  Time Kernel          : " << timeKernel << "\t(" << (100.0 * timeKernel / timeTotal) << "%)" << std::endl;
}
