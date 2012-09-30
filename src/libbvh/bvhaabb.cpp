/*
 * bvh.cpp
 *
 *  Created on: 19 Jan 2011
 *      Author: showroom
 */


/*
 * INCLUDES
 */

#include "bvhaabb.h"


/*
 * CLASS: BVHaabb
 */

	/* Constructor and destructor */

BVHaabb::BVHaabb()
{
	this->valid = 0;
	this->volume = 0.0f;
}

BVHaabb::~BVHaabb()
{
	return;
}


	/* Invalidate the aabb */

void BVHaabb::clear()
{
	this->valid = 0;
}


	/* Updates the aabb with another aabb */

void BVHaabb::update(BVHaabb &aabb)
{
	if (!aabb.valid)
		return;

	if (this->valid)
	{
		if (aabb.min[0] < this->min[0])
			this->min[0] = aabb.min[0];

		if (aabb.min[1] < this->min[1])
			this->min[1] = aabb.min[1];

		if (aabb.min[2] < this->min[2])
			this->min[2] = aabb.min[2];

		if (aabb.max[0] > this->max[0])
			this->max[0] = aabb.max[0];

		if (aabb.max[1] > this->max[1])
			this->max[1] = aabb.max[1];

		if (aabb.max[2] > this->max[2])
			this->max[2] = aabb.max[2];
	}
	else
	{
		this->min[0] = aabb.min[0];
		this->min[1] = aabb.min[1];
		this->min[2] = aabb.min[2];
		this->max[0] = aabb.max[0];
		this->max[1] = aabb.max[1];
		this->max[2] = aabb.max[2];
		this->valid = 1;
	}

	this->updateDimCentre();
}


	/* Updates the aabb with a vertex */

void BVHaabb::update(float *vertex)
{
	if (this->valid)
	{
		if (vertex[0] < this->min[0])
			this->min[0] = vertex[0];

		if (vertex[1] < this->min[1])
			this->min[1] = vertex[1];

		if (vertex[2] < this->min[2])
			this->min[2] = vertex[2];

		if (vertex[0] > this->max[0])
			this->max[0] = vertex[0];

		if (vertex[1] > this->max[1])
			this->max[1] = vertex[1];

		if (vertex[2] > this->max[2])
			this->max[2] = vertex[2];
	}
	else
	{
		this->min[0] = vertex[0];
		this->min[1] = vertex[1];
		this->min[2] = vertex[2];
		this->max[0] = vertex[0];
		this->max[1] = vertex[1];
		this->max[2] = vertex[2];
		this->valid = 1;
	}

	this->updateDimCentre();
}


	/* Enlarge the AABB */

void BVHaabb::enlarge(float delta)
{
	if (!this->valid)
		return;

	this->min[0] -= delta;
	this->min[1] -= delta;
	this->min[2] -= delta;
	this->max[0] += delta;
	this->max[1] += delta;
	this->max[2] += delta;

	this->updateDimCentre();
}


	/* Updates the dimensions and centre */

void BVHaabb::updateDimCentre()
{
	this->centre[0] = 0.5f * (this->min[0] + this->max[0]);
	this->centre[1] = 0.5f * (this->min[1] + this->max[1]);
	this->centre[2] = 0.5f * (this->min[2] + this->max[2]);
	this->dimensions[0] = this->max[0] - this->min[0];
	this->dimensions[1] = this->max[1] - this->min[1];
	this->dimensions[2] = this->max[2] - this->min[2];
	this->volume = this->dimensions[0] * this->dimensions[1] * this->dimensions[2];
}


	/* Get values */

float *BVHaabb::getDimensions()
{
	return this->dimensions;
}

float *BVHaabb::getCentre()
{
	return this->centre;
}

float BVHaabb::getVolume()
{
	return this->volume;
}


	/* Other operations */

int BVHaabb::getLargestDirection()
{
	if (this->dimensions[0] >= this->dimensions[1] &&
		this->dimensions[0] >= this->dimensions[2])
		return 0;
	else if (this->dimensions[1] > this->dimensions[0] &&
		this->dimensions[1] >= this->dimensions[2])
		return 1;
	else
		return 2;
}

int BVHaabb::checkCentreWithPlane(float *plane)
{
	float t = plane[3] - (this->centre[0] * plane[0] + this->centre[1] * plane[1] + this->centre[2] * plane[2]);
	return t > 0.0f;
}


	/* Check intersection */

int BVHaabb::checkIntersection(BVHRay &ray, float distance)
{
	// Calculate intersection
	float tpos[3];
	float tneg[3];
	float tmax[3];
	float tmin[3];
	float lmin;
	float lmax;

	tpos[0] = (this->max[0] - ray.pos[0]) * ray.dirInv[0];
	tpos[1] = (this->max[1] - ray.pos[1]) * ray.dirInv[1];
	tpos[2] = (this->max[2] - ray.pos[2]) * ray.dirInv[2];
	tneg[0] = (this->min[0] - ray.pos[0]) * ray.dirInv[0];
	tneg[1] = (this->min[1] - ray.pos[1]) * ray.dirInv[1];
	tneg[2] = (this->min[2] - ray.pos[2]) * ray.dirInv[2];

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

	if (distance == -1.0f)
		return lmax > lmin && lmax > 0.0f;
	else
		return lmax > lmin && lmax > 0.0f && lmin < distance;
}
