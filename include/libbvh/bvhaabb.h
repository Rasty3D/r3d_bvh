/*
 * bvhaabb.h
 *
 *  Created on: 19 Jan 2011
 *      Author: showroom
 */

#ifndef BVHAABB_H_
#define BVHAABB_H_

/*
 * INCLUDES
 */

#include <stdlib.h>
#include <iostream>

#include "bvhray.h"


/*
 * CLASS: BVHaabb
 */

class BVHaabb
{
public:
	int valid;
	float min[3];
	float max[3];
	float dimensions[3];
	float centre[3];
	float volume;

public:
		/* Constructor and destructor */
	BVHaabb();
	~BVHaabb();

		/* Invalidate the aabb */
	void clear();

		/* Updates the aabb with another aabb */
	void update(BVHaabb &aabb);

		/* Updates the aabb with a vertex */
	void update(float *vertex);

		/* Get values */
	float *getDimensions();
	float *getCentre();
	float getVolume();

		/* Enlarge the AABB */
	void enlarge(float delta);

private:
		/* Updates the dimensions and centre */
	void updateDimCentre();

public:
		/* Other operations */
	int getLargestDirection();
	int checkCentreWithPlane(float *plane);

		/* Check intersection */
	int checkIntersection(BVHRay &ray, float distance);
};

#endif /* BVHAABB_H_ */
