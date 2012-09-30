/*
 * bvhnode.cpp
 *
 *  Created on: 19 Jan 2011
 *      Author: showroom
 */


/*
 * INCLUDES
 */

#include "bvhnode.h"


/*
 * CLASS: BVHNode
 */

	/* Static members */

unsigned long BVHNode::idCounter = 0;


	/* Constructor and destructor */

BVHNode::BVHNode()
{
	this->father = NULL;
	this->child = NULL;
	this->prevBrother = NULL;
	this->nextBrother = NULL;
	this->primitive = NULL;

	this->id = BVHNode::idCounter;
	BVHNode::idCounter++;
}

BVHNode::~BVHNode()
{
	// Delete references from previous brother
	if (this->prevBrother != NULL)
		this->prevBrother->nextBrother = this->nextBrother;

	// Delete next brother
	if (this->nextBrother != NULL)
		delete this->nextBrother;

	// Remove reference from parent
	if (this->father != NULL && this->father->child == this)
		this->father->child = this->nextBrother;

	// Delete all children
	if (this->child != NULL)
	{
		BVHNode *node = this->child;

		while (node->nextBrother != NULL)
		{
			node = node->nextBrother;
		}

		BVHNode *nextBrother;

		while (node != NULL)
		{
			nextBrother = node;
			node = node->prevBrother;
			delete nextBrother;
		}
	}
}


	/* Tree operations */

// Add a new brother
BVHNode *BVHNode::addBrother(BVHNode *node)
{
	if (node == NULL)
		return NULL;

	if (this->nextBrother != NULL)
		this->nextBrother->prevBrother = node;
	node->nextBrother = this->nextBrother;
	this->nextBrother = node;
	node->prevBrother = this;

	return node;
}

// Add a new child
BVHNode *BVHNode::addChild(BVHNode *node)
{
	if (node == NULL)
		return NULL;

	if (this->child == NULL)
	{
		this->child = node;
		this->child->prevBrother = NULL;
		this->child->nextBrother = NULL;
	}
	else
	{
		this->child->addBrother(node);
	}

	node->father = this;

	return node;
}

// Move a node
BVHNode *BVHNode::move(BVHNode *newFather)
{
	// If the father is already its father, do nothing
	if (this->father == newFather)
		return NULL;

	// Change brothers
	if (this->prevBrother != NULL)
		this->prevBrother->nextBrother = this->nextBrother;

	if (this->nextBrother != NULL)
		this->nextBrother->prevBrother = this->prevBrother;

	// Change father's child
	if (this->prevBrother == NULL && this->father != NULL)
		this->father->child = this->nextBrother;

	// Add new child to the new father
	if (newFather != NULL)
		newFather->addChild(this);

	return this;
}

// Get number of children
int BVHNode::getChildrenCount()
{
	BVHNode *node = this->child;
	int count = 0;

	while (node != NULL)
	{
		node = node->nextBrother;
		count++;
	}

	return count;
}

// Divide if the number of children is more than 2
void BVHNode::divide(int nThreads)
{
	// Variables
	int childrenCount;
	BVHNode *node;
	BVHNode *aux;
	BVHaabb aabbLeft;
	BVHaabb aabbRight;
	int childrenLeft;
	int childrenRight;
	static const int splittingPlanes = 9;
	static const float splittingFactors[splittingPlanes] = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f};
	int bestSplittingDirection;
	float splittingPlane;
	float bestSplittingPlane;
	int direction;
	int plane;
	float cost;
	float bestCost;

	// Check the number of children
	childrenCount = this->getChildrenCount();

	if (childrenCount <= 4)
		return;

	// Test binning planes in the three directions
	bestSplittingDirection = 0;
	bestCost = -1.0f;
	bestSplittingPlane = 0.0f;

	for (direction = 0; direction < 3; direction++)
	{
		for (plane = 0; plane < splittingPlanes; plane++)
		{
			// Reset variables
			childrenLeft = 0;
			childrenRight = 0;
			aabbLeft.clear();
			aabbRight.clear();

			// Calculate split plane
			splittingPlane =
				this->aabb.min[direction] +
				(this->aabb.max[direction] - this->aabb.min[direction]) * splittingFactors[plane];

			// Loop in the children and split left & right
			node = this->child;

			while (node != NULL)
			{
				if (node->aabb.centre[direction] < splittingPlane)
				{
					childrenLeft++;
					aabbLeft.update(node->aabb);
				}
				else
				{
					childrenRight++;
					aabbRight.update(node->aabb);
				}

				node = node->nextBrother;
			}

			if (childrenLeft <= 0 || childrenRight <= 0)
				continue;

			// Calculate cost of splitting
			cost = childrenLeft * aabbLeft.getVolume() + childrenRight * aabbRight.getVolume();

			// Update best cost
			if (bestCost == -1.0f || cost < bestCost)
			{
				bestCost = cost;
				bestSplittingPlane = splittingPlane;
				bestSplittingDirection = direction;
			}
		}
	}

	if (bestCost == -1.0f)
		return;

	// Generate two new children
	BVHNode *child1 = new BVHNode;
	BVHNode *child2 = new BVHNode;

	// Move nodes
	node = this->child;

	while (node != NULL)
	{
		aux = node->nextBrother;

		if (node->aabb.centre[bestSplittingDirection] < bestSplittingPlane)
		{
			node->move(child1);
			child1->aabb.update(node->aabb);
		}
		else
		{
			node->move(child2);
			child2->aabb.update(node->aabb);
		}

		node = aux;
	}

	// Add the new children
	this->addChild(child1);
	this->addChild(child2);

	// Subdivide new children
	if (nThreads <= 1)
	{
		child1->divide();
		child2->divide();
	}
	else if (nThreads == 2)
	{
		pthread_t threadChild1;
		pthread_t threadChild2;
		pthread_create(&threadChild1, NULL, BVHNode::divideThread, (void*)child1);
		pthread_create(&threadChild2, NULL, BVHNode::divideThread, (void*)child2);
		pthread_join(threadChild1, NULL);
		pthread_join(threadChild2, NULL);
	}
	else
	{
		child1->divide(nThreads / 2);
		child2->divide(nThreads - nThreads / 2);
	}
}

// Set id
int BVHNode::setId(unsigned long &id)
{
	this->id = id;
	id++;

	BVHNode *node = this->child;

	while (node != NULL)
	{
		node->setId(id);
		node = node->nextBrother;
	}

	return id;
}

// Fill static node information
void BVHNode::fillStatic(BVHNodeStatic *nodeList, int nodeSize, int &nodeIndex, int skipIndex)
{
	if (nodeIndex >= nodeSize)
		return;

	nodeList[nodeIndex].aabb = this->aabb;
	nodeList[nodeIndex].primitive = this->primitive;
	nodeList[nodeIndex].skipIndex = skipIndex;
	nodeIndex++;

	BVHNode *node = this->child;

	while (node != NULL)
	{
		if (node->nextBrother != NULL)
			node->fillStatic(nodeList, nodeSize, nodeIndex, node->nextBrother->id);
		else
			node->fillStatic(nodeList, nodeSize, nodeIndex, skipIndex);

		node = node->nextBrother;
	}
}


	/* Primitive operations */

void BVHNode::setPrimitive(BVHPrimitive *primitive)
{
	if (this->primitive != primitive)
	{
		this->primitive = primitive;

		if (this->primitive != NULL)
		{
			this->aabb = this->primitive->getAABB();

			// Update father aabb
			if (this->father != NULL)
				this->father->aabb.update(this->aabb);
		}
	}
}

BVHPrimitive *BVHNode::getPrimitive()
{
	return this->primitive;
}


	/* Thread to divide the node */

void *BVHNode::divideThread(void *node)
{
	((BVHNode*)node)->divide();
	return NULL;
}
