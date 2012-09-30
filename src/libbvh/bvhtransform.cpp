/*
 * bvhtransform.cpp
 *
 *  Created on: 4 May 2011
 *      Author: showroom
 */

/*
 * INCLUDES
 */

#include "bvhtransform.h"


/*
 * NAMESPACES
 */

using namespace std;


/*
 * CLASS: BVHTransform
 */

	/* Constructor and destructor */

BVHTransform::BVHTransform()
{
	this->identity();
}

BVHTransform::~BVHTransform()
{
	return;
}


	/* Set identity matrix */

void BVHTransform::identity()
{
	this->position[0] = 0.0f;
	this->position[1] = 0.0f;
	this->position[2] = 0.0f;

	this->rotation[0] = 1.0f;
	this->rotation[1] = 0.0f;
	this->rotation[2] = 0.0f;
	this->rotation[3] = 0.0f;
	this->rotation[4] = 1.0f;
	this->rotation[5] = 0.0f;
	this->rotation[6] = 0.0f;
	this->rotation[7] = 0.0f;
	this->rotation[8] = 1.0f;
}


	/* Translate */

void BVHTransform::translate(float x, float y, float z)
{
	this->position[0] += x;
	this->position[1] += y;
	this->position[2] += z;
}

void BVHTransform::translate(float *position)
{
	this->translate(position[0], position[1], position[2]);
}


	/* Rotate */

void BVHTransform::rotate_angleAxis(float angle, float x, float y, float z)
{
	float matrix[9];
	angleAxis2Matrix(angle, x, y, z, matrix);
	this->rotate_matrix(matrix);
}

void BVHTransform::rotate_angleAxis(float angle, float *axis)
{
	this->rotate_angleAxis(angle, axis[0], axis[1], axis[2]);
}

void BVHTransform::rotate_angleAxis(float *angleAxis)
{
	this->rotate_angleAxis(angleAxis[0], angleAxis[1], angleAxis[2], angleAxis[3]);
}

void BVHTransform::rotate_matrix(float *matrix)
{
	float newPosition[3];
	multRotationByVector(matrix, this->position, newPosition);
	memcpy(this->position, newPosition, sizeof(float) * 3);

	float newRotation[9];
	multRotationByRotation(matrix, this->rotation, newRotation);
	memcpy(this->rotation, newRotation, sizeof(float) * 9);
}

void BVHTransform::rotate_quat(float w, float i, float j, float k)
{
	float matrix[9];
	quat2Matrix(w, i, j, k, matrix);
	this->rotate_matrix(matrix);
}

void BVHTransform::rotate_quat(float *quat)
{
	this->rotate_quat(quat[0], quat[1], quat[2], quat[3]);
}

void BVHTransform::rotate_euler(float x, float y, float z, int mode)
{
	float matrix[9];
	euler2Matrix(x, y, z, mode, matrix);
	this->rotate_matrix(matrix);
}

void BVHTransform::rotate_euler(float *angles, int mode)
{
	this->rotate_euler(angles[0], angles[1], angles[2], mode);
}


	/* Scales */

void BVHTransform::scale(float x, float y, float z)
{
	float matrix[9] = {
		x, 0.0f, 0.0f,
		0.0f, y, 0.0f,
		0.0f, 0.0f, z};
	this->rotate_matrix(matrix);
}

void BVHTransform::scale(float *factor)
{
	this->scale(factor[0], factor[1], factor[2]);
}

void BVHTransform::scale(float factor)
{
	this->scale(factor, factor, factor);
}


	/* Look at */

void BVHTransform::lookat_obj(
	float posx, float posy, float posz,
	float objx, float objy, float objz,
	float upx, float upy, float upz)
{
	this->position[0] = posx;
	this->position[1] = posy;
	this->position[2] = posz;
	this->lookat_obj(objx, objy, objz, upx, upy, upz);
}

void BVHTransform::lookat_obj(float *pos, float *obj, float *up)
{
	this->lookat_obj(
		pos[0], pos[1], pos[2],
		obj[0], obj[1], obj[2],
		up[0], up[1], up[2]);
}

void BVHTransform::lookat_dir(
	float posx, float posy, float posz,
	float dirx, float diry, float dirz,
	float upx, float upy, float upz)
{
	this->position[0] = posx;
	this->position[1] = posy;
	this->position[2] = posz;
	this->lookat_dir(dirx, diry, dirz, upx, upy, upz);
}

void BVHTransform::lookat_dir(float *pos, float *dir, float *up)
{
	this->lookat_dir(
		pos[0], pos[1], pos[2],
		dir[0], dir[1], dir[2],
		up[0], up[1], up[2]);
}

void BVHTransform::lookat_obj(
	float objx, float objy, float objz,
	float upx, float upy, float upz)
{
	this->lookat_dir(
		objx - this->position[0],
		objy - this->position[1],
		objz - this->position[2],
		upx, upy, upz);
}

void BVHTransform::lookat_obj(float *obj, float *up)
{
	this->lookat_obj(
		obj[0], obj[1], obj[2],
		up[0], up[1], up[2]);
}

void BVHTransform::lookat_dir(
	float dirx, float diry, float dirz,
	float upx, float upy, float upz)
{
	float module;

	this->rotation[0] = dirx;
	this->rotation[1] = diry;
	this->rotation[2] = dirz;
	module = sqrtf(
		this->rotation[0] * this->rotation[0] +
		this->rotation[1] * this->rotation[1] +
		this->rotation[2] * this->rotation[2]);
	this->rotation[0] /= module;
	this->rotation[1] /= module;
	this->rotation[2] /= module;

	this->rotation[3] =
		this->rotation[1] * upz -
		this->rotation[2] * upy;
	this->rotation[4] =
		this->rotation[2] * upx -
		this->rotation[0] * upz;
	this->rotation[5] =
		this->rotation[0] * upy -
		this->rotation[1] * upx;
	module = sqrtf(
		this->rotation[3] * this->rotation[3] +
		this->rotation[4] * this->rotation[4] +
		this->rotation[5] * this->rotation[5]);
	this->rotation[3] /= module;
	this->rotation[4] /= module;
	this->rotation[5] /= module;

	this->rotation[6] =
		this->rotation[2] * this->rotation[4] -
		this->rotation[1] * this->rotation[5];
	this->rotation[7] =
		this->rotation[0] * this->rotation[5] -
		this->rotation[2] * this->rotation[3];
	this->rotation[8] =
		this->rotation[1] * this->rotation[3] -
		this->rotation[0] * this->rotation[4];
	module = sqrtf(
		this->rotation[6] * this->rotation[6] +
		this->rotation[7] * this->rotation[7] +
		this->rotation[8] * this->rotation[8]);
	this->rotation[6] /= module;
	this->rotation[7] /= module;
	this->rotation[8] /= module;
}

void BVHTransform::lookat_dir(float *dir, float *up)
{
	this->lookat_dir(
		dir[0], dir[1], dir[2],
		up[0], up[1], up[2]);
}


	/* Transforms */

void BVHTransform::transform(BVHTransform &transform)
{
	this->translate(transform.position);
	this->rotate_matrix(transform.rotation);
}

void BVHTransform::transformVector(const float *v, float *result)
{
	multRotationByVector(this->rotation, v, result);
	result[0] += this->position[0];
	result[1] += this->position[1];
	result[2] += this->position[2];
}

void BVHTransform::transformVector(float *v)
{
	float newVector[3];
	multRotationByVector(this->rotation, v, newVector);
	v[0] = newVector[0] + this->position[0];
	v[1] = newVector[1] + this->position[1];
	v[2] = newVector[2] + this->position[2];
}

void BVHTransform::rotateVector(const float *v, float *result)
{
	multRotationByVector(this->rotation, v, result);
}

void BVHTransform::rotateVector(float *v)
{
	float newVector[3];
	multRotationByVector(this->rotation, v, newVector);
	memcpy(v, newVector, sizeof(float) * 3);
}
