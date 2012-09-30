/*
 * bvhnodebinary.h
 *
 *  Created on: 27 Jan 2011
 *      Author: showroom
 */

#ifndef BVHNODEBINARY_H_
#define BVHNODEBINARY_H_

/*
 * INCLUDES
 */

#include <stdlib.h>
#include <iostream>

#include "bvhaabb.h"
#include "bvhprimitive.h"
#include "bvhnodestatic.h"


/*
 * CLASS: BVHNodeBinary
 */

class BVHNodeBinary
{
private:
		/* Tree structure */
	BVHNodeBinary *children[2];

		/* Axis Aligned Bounding Box */
	BVHaabb aabb;

		/* Pointer to primitive */
	BVHPrimitive *primitive;

		/* Counter */
	unsigned long id;
	static unsigned long idCounter;

public:
		/* Constructor and destructor */
	BVHNodeBinary();
	~BVHNodeBinary();

		/* Tree operations */
	// Set children
	void setChildren(BVHNodeBinary *child1, BVHNodeBinary *child2);

	// Get id
	unsigned long getId();

	// Set id
	void setId(unsigned long &id);

	// Fill static node information
	void fillStatic(BVHNodeStatic *nodeList, int nodeSize, int &nodeIndex, int skipIndex);

		/* Primitive operations */
	void setPrimitive(BVHPrimitive *primitive);
	BVHPrimitive *getPrimitive();

		/* Check intersection */
	int checkIntersection(float *ray, float &distance);

		/* Print AABB */
	void printAABB();

		/* Get score */
	inline static float getScore(BVHNodeBinary *node1, BVHNodeBinary *node2)
	{
		static float min[3];
		static float max[3];
		static float volumeTotal;

		min[0] = (node1->aabb.min[0] < node2->aabb.min[0]) ? node1->aabb.min[0] : node2->aabb.min[0];
		min[1] = (node1->aabb.min[1] < node2->aabb.min[1]) ? node1->aabb.min[1] : node2->aabb.min[1];
		min[2] = (node1->aabb.min[2] < node2->aabb.min[2]) ? node1->aabb.min[2] : node2->aabb.min[2];
		max[0] = (node1->aabb.max[0] > node2->aabb.max[0]) ? node1->aabb.max[0] : node2->aabb.max[0];
		max[1] = (node1->aabb.max[1] > node2->aabb.max[1]) ? node1->aabb.max[1] : node2->aabb.max[1];
		max[2] = (node1->aabb.max[2] > node2->aabb.max[2]) ? node1->aabb.max[2] : node2->aabb.max[2];

		volumeTotal =
			(max[0] - min[0]) *
			(max[1] - min[1]) *
			(max[2] - min[2]);

		return volumeTotal - node1->aabb.volume - node2->aabb.volume;
		//return (volumeTotal - node1->aabb.volume - node2->aabb.volume) / volumeTotal;
	}
};

#endif /* BVHNODEBINARY_H_ */
