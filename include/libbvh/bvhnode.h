/*
 * bvhnode.h
 *
 *  Created on: 19 Jan 2011
 *      Author: showroom
 */

#ifndef BVHNODE_H_
#define BVHNODE_H_

/*
 * INCLUDES
 */

#include <stdlib.h>
#include <iostream>
#include <pthread.h>

#include "bvhaabb.h"
#include "bvhprimitive.h"
#include "bvhnodestatic.h"


/*
 * CLASS: BVHNode
 */

class BVHNode
{
private:
		/* Tree structure */
	BVHNode *father;
	BVHNode *child;
	BVHNode *prevBrother;
	BVHNode *nextBrother;

		/* Axis Aligned Bounding Box */
	BVHaabb aabb;

		/* Pointer to primitive */
	BVHPrimitive *primitive;

		/* Counter */
	unsigned long id;
	static unsigned long idCounter;

		/* Comparison function prototype */
	typedef int(*BVHNodeCompare)(BVHNode*, BVHNode*);

public:
		/* Constructor and destructor */
	BVHNode();
	~BVHNode();

		/* Tree operations */
	// Add a new brother
	BVHNode *addBrother(BVHNode *node);

	// Add a new child
	BVHNode *addChild(BVHNode *node);

	// Move a node
	BVHNode *move(BVHNode *newFather);

	// Get number of children
	int getChildrenCount();

	// Divide if the number of children is more than 2
	void divide(int nThreads = 1);

	// Set id
	int setId(unsigned long &id);

	// Fill static node information
	void fillStatic(BVHNodeStatic *nodeList, int nodeSize, int &nodeIndex, int skipIndex);

		/* Primitive operations */
	void setPrimitive(BVHPrimitive *primitive);
	BVHPrimitive *getPrimitive();

private:
		/* Thread to divide the node */
	static void *divideThread(void *node);
};

#endif /* BVHNODE_H_ */
