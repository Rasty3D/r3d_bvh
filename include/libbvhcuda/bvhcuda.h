/*
 * bvhcuda.h
 *
 *  Created on: 7 Feb 2011
 *      Author: showroom
 */

#ifndef BVHCUDA_H_
#define BVHCUDA_H_


/*
 * INCLUDES
 */

#include <unistd.h>


/*
 * CUDA functions
 */

void BVHcuda_getDeviceInfo();
void BVHcuda_init(int device);
void BVHcuda_finish();

#endif /* BVHCUDA_H_ */
