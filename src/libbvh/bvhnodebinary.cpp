/*
 * BVHNodeBinarybinary.cpp
 *
 *  Created on: 27 Jan 2011
 *      Author: showroom
 */


/*
 * INCLUDES
 */

#include "bvhnodebinary.h"


/*
 * CLASS: BVHNodeBinary
 */

	/* Static members */

unsigned long BVHNodeBinary::idCounter = 0;


	/* Constructor and destructor */

BVHNodeBinary::BVHNodeBinary()
{
	this->children[0] = NULL;
	this->children[1] = NULL;
	this->primitive = NULL;

	this->id = BVHNodeBinary::idCounter;
	BVHNodeBinary::idCounter++;
}

BVHNodeBinary::~BVHNodeBinary()
{
	// Delete children
	if (this->children[0] != NULL)
		delete this->children[0];

	if (this->children[1] != NULL)
		delete this->children[1];
}


	/* Tree operations */

// Set children
void BVHNodeBinary::setChildren(BVHNodeBinary *child1, BVHNodeBinary *child2)
{
	this->children[0] = child1;
	this->children[1] = child2;

	this->aabb.valid = 1;
	this->aabb.min[0] = (child1->aabb.min[0] < child2->aabb.min[0]) ? child1->aabb.min[0] : child2->aabb.min[0];
	this->aabb.min[1] = (child1->aabb.min[1] < child2->aabb.min[1]) ? child1->aabb.min[1] : child2->aabb.min[1];
	this->aabb.min[2] = (child1->aabb.min[2] < child2->aabb.min[2]) ? child1->aabb.min[2] : child2->aabb.min[2];
	this->aabb.max[0] = (child1->aabb.max[0] > child2->aabb.max[0]) ? child1->aabb.max[0] : child2->aabb.max[0];
	this->aabb.max[1] = (child1->aabb.max[1] > child2->aabb.max[1]) ? child1->aabb.max[1] : child2->aabb.max[1];
	this->aabb.max[2] = (child1->aabb.max[2] > child2->aabb.max[2]) ? child1->aabb.max[2] : child2->aabb.max[2];
	this->aabb.volume =
		(this->aabb.max[0] - this->aabb.min[0]) *
		(this->aabb.max[1] - this->aabb.min[1]) *
		(this->aabb.max[2] - this->aabb.min[2]);
}

// Get id
unsigned long BVHNodeBinary::getId()
{
	return this->id;
}

// Set id
void BVHNodeBinary::setId(unsigned long &id)
{
	this->id = id;
	id++;

	if (this->children[0] != NULL)
		this->children[0]->setId(id);

	if (this->children[1] != NULL)
		this->children[1]->setId(id);
}

// Fill static node information
void BVHNodeBinary::fillStatic(BVHNodeStatic *nodeList, int nodeSize, int &nodeIndex, int skipIndex)
{
	if (nodeIndex >= nodeSize)
		return;

	nodeList[nodeIndex].aabb = this->aabb;
	nodeList[nodeIndex].primitive = this->primitive;
	nodeList[nodeIndex].skipIndex = skipIndex;
	nodeIndex++;

	if (this->children[0] != NULL)
		this->children[0]->fillStatic(nodeList, nodeSize, nodeIndex, this->children[1]->id);

	if (this->children[1] != NULL)
		this->children[1]->fillStatic(nodeList, nodeSize, nodeIndex, skipIndex);
}


	/* Primitive operations */

void BVHNodeBinary::setPrimitive(BVHPrimitive *primitive)
{
	if (this->primitive != primitive)
	{
		this->primitive = primitive;

		if (this->primitive != NULL)
			this->aabb = this->primitive->getAABB();
	}
}

BVHPrimitive *BVHNodeBinary::getPrimitive()
{
	return this->primitive;
}


	/* Check intersection */

int BVHNodeBinary::checkIntersection(float *ray, float &distance)
{
	if (!this->aabb.checkIntersection(ray, distance))
		return 0;

	if (this->primitive != NULL)
	{
		return this->primitive->checkIntersection(ray, distance);
	}
	else
	{
		int intersection = 0;

		if (this->children[0] != NULL &&
			this->children[0]->checkIntersection(ray, distance))
			intersection = 1;

		if (this->children[1] != NULL &&
			this->children[1]->checkIntersection(ray, distance))
			intersection = 1;

		return intersection;
	}
}


	/* Print AABB */

void BVHNodeBinary::printAABB()
{
	this->aabb.print();
}


	/* Get score */

/*
float BVHNodeBinary::getScore(BVHNodeBinary *node1, BVHNodeBinary *node2)
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
}*/
