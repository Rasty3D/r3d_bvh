/*
 * bvhnray.h
 *
 *  Created on: 03 Feb 2011
 *      Author: showroom
 */

#ifndef BVHRAY_H_
#define BVHRAY_H_

/*
 * TYPES
 */

typedef struct
{
	float pos[3];
	float dir[3];
	float dirInv[3];
}BVHRay;

#endif /* BVHRAY_H_ */
