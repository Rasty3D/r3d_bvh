/*
 * bvhcuda.cu
 *
 *  Created on: 7 Feb 2011
 *      Author: showroom
 */


/*
 * INCLUDES
 */

	/* General */
#include <iostream>

	/* CUDA */
#include <cuda.h>

	/* BVH */
#include "bvhcudainline.h"
#include "bvhcuda.h"


/*
 * NAMESPACES
 */

using namespace std;


/*
 * CUDA functions
 */

void BVHcuda_getDeviceInfo()
{
	int deviceCount;
	int device;
	struct cudaDeviceProp deviceProperties;

	cudaGetDeviceCount(&deviceCount);

	cout << "Device list [" << deviceCount << "]:" << endl;

	for (device = 0; device < deviceCount; device++)
	{
		cudaGetDeviceProperties(&deviceProperties, device);

		cout << "  " << (device + 1) << ".- " << deviceProperties.name << ":" << endl;
		cout << "     Total global memory    : " << deviceProperties.totalGlobalMem << endl;
		cout << "     Shared memory per block: " << deviceProperties.sharedMemPerBlock << endl;
		cout << "     Registers per block    : " << deviceProperties.regsPerBlock << endl;
		cout << "     Warp size              : " << deviceProperties.warpSize << endl;
		cout << "     Memory pitch           : " << deviceProperties.memPitch << endl;
		cout << "     Max threads per block  : " << deviceProperties.maxThreadsPerBlock << endl;
		cout << "     Max threads dimension  : " <<
			deviceProperties.maxThreadsDim[0] << ", " <<
			deviceProperties.maxThreadsDim[1] << ", " <<
			deviceProperties.maxThreadsDim[2] << endl;
		cout << "     Max grid size          : " <<
			deviceProperties.maxGridSize[0] << ", " <<
			deviceProperties.maxGridSize[1] << ", " <<
			deviceProperties.maxGridSize[2] << endl;
		cout << "     Major                  : " << deviceProperties.major << endl;
		cout << "     Minor                  : " << deviceProperties.minor << endl;
		cout << "     Clock rate             : " << deviceProperties.clockRate << endl;
		cout << "     Texture alignment      : " << deviceProperties.textureAlignment << endl;
		cout << "     Multi processor count  : " << deviceProperties.multiProcessorCount << endl;
		cout << "     Kernel execution time  : " << deviceProperties.kernelExecTimeoutEnabled << endl;
		cout << "     Integrated             : " << deviceProperties.integrated << endl;
		cout << "     Can map host memory    : " << deviceProperties.canMapHostMemory << endl;
		cout << "     Compute mode           : " << deviceProperties.computeMode << endl;
		cout << "     Concurrent kernels     : " << deviceProperties.concurrentKernels << endl;
		cout << "     ECC enabled            : " << deviceProperties.ECCEnabled << endl;
		cout << "     PCI bus ID             : " << deviceProperties.pciBusID << endl;
		cout << "     PCI device ID          : " << deviceProperties.pciDeviceID << endl;
		cout << "     TCC driver             : " << deviceProperties.tccDriver << endl;
	}

	cout << "Number of CPU processors: " << sysconf(_SC_NPROCESSORS_ONLN) << endl;
}

void BVHcuda_init(int device)
{
		/* Init device */

	cudaSetDevice(device);


		/* Send a wakeup event */

	cudaEvent_t wakeGPU;
	cuda_safeCall(cudaEventCreate(&wakeGPU));


		/* Read the properties of the selected device */

	struct cudaDeviceProp deviceProperties;

	cudaGetDevice(&device);
	cudaGetDeviceProperties(&deviceProperties, device);

	//cout << "Using " << deviceProperties.name << endl;
}

void BVHcuda_finish()
{
	cudaThreadExit();
}

