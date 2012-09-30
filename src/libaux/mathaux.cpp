/*
 * math.cpp
 *
 *   Created on: 5 May 2011
 *       Author: Jesus Ortiz
 *
 *  Description: TODO
 *
 */


/*
 * INCLUDES
 */

#include "mathaux.h"


/*
 * FUNCTIONS
 */

	/* Basic functions */

float max(float value1, float value2)
{
	return (value1 > value2) ? value1: value2;
}

float min(float value1, float value2)
{
	return (value1 < value2) ? value1: value2;
}

float sign(float value)
{
	if (value > 0.0f)
		return 1.0f;
	else if (value < 0.0f)
		return -1.0f;
	else
		return 0.0f;
}

float smooth(float value)
{
	if (value <= 0.0f)
		return 0.0f;
	else if (value >= 1.0f)
		return 1.0f;
	else
		return value * value * (3.0f - 2.0f * value);
}


	/* Vector operations */
// Defined inline in the header file


	/* Rotation conversions */

// Rotation matrix from <angle, axis>
void angleAxis2Matrix(const float *angleAxis, float *matrix)
{
	angleAxis2Matrix(angleAxis[0], angleAxis[1], angleAxis[2], angleAxis[3], matrix);
}

void angleAxis2Matrix(float angle, const float *axis, float *matrix)
{
	angleAxis2Matrix(angle, axis[0], axis[1], axis[2], matrix);
}

void angleAxis2Matrix(float angle, float x, float y, float z, float *matrix)
{
	// Variables
	float axisModule;
	float sinAngle;
	float cosAngle;
	float xx;
	float xy;
	float xz;
	float yy;
	float yz;
	float zz;

	// Normalize axis
	axisModule = sqrtf(x * x + y * y + z * z);

	if (axisModule == 0.0f)
		return;

	x /= axisModule;
	y /= axisModule;
	z /= axisModule;

	// Calculate sin and cos of the angle
	sinAngle = sinf(angle);
	cosAngle = cosf(angle);

	// Calculate cross products
	xx = x * x;
	xy = x * y;
	xz = x * z;
	yy = y * y;
	yz = y * z;
	zz = z * z;

	// Calculate matrix
	matrix[0] = xx + (1.0f - xx) * cosAngle;
	matrix[1] = xy - xy * cosAngle + z * sinAngle;
	matrix[2] = xz - xz * cosAngle - y * sinAngle;

	matrix[3] = xy - xy * cosAngle - z * sinAngle;
	matrix[4] = yy + (1.0f - yy) * cosAngle;
	matrix[5] = yz - yz * cosAngle + x * sinAngle;

	matrix[6] = xz - xz * cosAngle + y * sinAngle;
	matrix[7] = yz - yz * cosAngle - x * sinAngle;
	matrix[8] = zz + (1.0f - zz) * cosAngle;
}

// Rotation matrix from quaternion
void quat2Matrix(const float *quat, float *matrix)
{
	quat2Matrix(quat[0], quat[1], quat[2], quat[3], matrix);
}

void quat2Matrix(float w, float i, float j, float k, float *matrix)
{
	matrix[0] = w * w + i * i - j * j - k * k;
	matrix[1] = 2.0f * (i * j + w * k);
	matrix[2] = 2.0f * (i * k - w * j);

	matrix[3] = 2.0f * (i * j - w * k);
	matrix[4] = w * w - i * i + j * j - k * k;
	matrix[5] = 2.0f * (j * k + w * i);

	matrix[6] = 2.0f * (i * k + w * j);
	matrix[7] = 2.0f * (j * k - w * i);
	matrix[8] = w * w - i * i - j * j + k * k;
}

// Rotation matrix from Euler angles
void euler2Matrix(const float *xyz, int mode, float *matrix)
{
	euler2Matrix(xyz[0], xyz[1], xyz[2], mode, matrix);
}

void euler2Matrix(float x, float y, float z, int mode, float *matrix)
{
	float matrixX[9] = {
		1.0f, 0.0f, 0.0f,
		0.0f, cosf(x), sinf(x),
		0.0f, -sinf(x), cosf(x)};
	float matrixY[9] = {
		cosf(y), 0.0f, -sinf(y),
		0.0f, 1.0f, 0.0f,
		sinf(y), 0.0f, cosf(y)};
	float matrixZ[9] = {
		cosf(z), sinf(z), 0.0f,
		-sinf(z), cosf(z), 0.0f,
		0.0f, 0.0f, 1.0f};
	float result[9];

	if (mode == MATH_EULER_XYZ)
	{
		multRotationByRotation(matrixX, matrixY, result);
		multRotationByRotation(result, matrixZ, matrix);
	}
	else if (mode == MATH_EULER_XZY)
	{
		multRotationByRotation(matrixX, matrixZ, result);
		multRotationByRotation(result, matrixY, matrix);
	}
	else if (mode == MATH_EULER_YXZ)
	{
		multRotationByRotation(matrixY, matrixX, result);
		multRotationByRotation(result, matrixZ, matrix);
	}
	else if (mode == MATH_EULER_YZX)
	{
		multRotationByRotation(matrixY, matrixZ, result);
		multRotationByRotation(result, matrixX, matrix);
	}
	else if (mode == MATH_EULER_ZXY)
	{
		multRotationByRotation(matrixZ, matrixX, result);
		multRotationByRotation(result, matrixY, matrix);
	}
	else if (mode == MATH_EULER_ZYX)
	{
		multRotationByRotation(matrixZ, matrixY, result);
		multRotationByRotation(result, matrixX, matrix);
	}
}

// Euler to quaterion
void euler2Quat(const float *xyz, int mode, float *quat)
{
	euler2Quat(xyz[0], xyz[1], xyz[2], mode, quat);
}

void euler2Quat(float x, float y, float z, int mode, float *quat)
{
	float matrix[9];
	euler2Matrix(x, y, z, mode, matrix);
	matrix2Quat(matrix, quat);
}

// Matrix to quaternion
void matrix2Quat(const float *matrix, float *quat)
{
	quat[0] = sqrtf(max(0.0f, 1.0f + matrix[0] + matrix[4] + matrix[8])) / 2.0f;
	quat[1] = sign(matrix[5] - matrix[7]) * sqrtf(max(0.0f, 1.0f + matrix[0] - matrix[4] - matrix[8])) / 2.0f;
	quat[2] = sign(matrix[6] - matrix[2]) * sqrtf(max(0.0f, 1.0f - matrix[0] + matrix[4] - matrix[8])) / 2.0f;
	quat[3] = sign(matrix[1] - matrix[3]) * sqrtf(max(0.0f, 1.0f - matrix[0] - matrix[4] + matrix[8])) / 2.0f;
}

// <Angle, axis> to quaterion
void angleAxis2Quat(const float *angleAxis, float *quat)
{
	angleAxis2Quat(angleAxis[0], angleAxis[1], angleAxis[2], angleAxis[3], quat);
}

void angleAxis2Quat(float angle, const float *axis, float *quat)
{
	angleAxis2Quat(angle, axis[0], axis[1], axis[2], quat);
}

void angleAxis2Quat(float angle, float x, float y, float z, float *quat)
{
	float s = sinf(0.5f * angle);
	float module = sqrtf(x * x + y * y + z * z);

	if (module == 0.0f)
	{
		quat[0] = 1.0f;
		quat[1] = 0.0f;
		quat[2] = 0.0f;
		quat[3] = 0.0f;
	}
	else
	{
		s /= module;
		quat[0] = cosf(0.5f * angle);
		quat[1] = x * s;
		quat[2] = y * s;
		quat[3] = z * s;
	}
}


	/* Transform operations */

// Multiply rotation matrix by rotation matrix
void multRotationByRotation(const float *t1, const float *t2, float *result)
{
	result[0] = t1[0] * t2[0] + t1[3] * t2[1] + t1[6] * t2[2];
	result[1] = t1[1] * t2[0] + t1[4] * t2[1] + t1[7] * t2[2];
	result[2] = t1[2] * t2[0] + t1[5] * t2[1] + t1[8] * t2[2];

	result[3] = t1[0] * t2[3] + t1[3] * t2[4] + t1[6] * t2[5];
	result[4] = t1[1] * t2[3] + t1[4] * t2[4] + t1[7] * t2[5];
	result[5] = t1[2] * t2[3] + t1[5] * t2[4] + t1[8] * t2[5];

	result[6] = t1[0] * t2[6] + t1[3] * t2[7] + t1[6] * t2[8];
	result[7] = t1[1] * t2[6] + t1[4] * t2[7] + t1[7] * t2[8];
	result[8] = t1[2] * t2[6] + t1[5] * t2[7] + t1[8] * t2[8];
}

// Multiply rotation matrix by vector
void multRotationByVector(const float *r, const float *v, float *result)
{
	result[0] = r[0] * v[0] + r[3] * v[1] + r[6] * v[2];
	result[1] = r[1] * v[0] + r[4] * v[1] + r[7] * v[2];
	result[2] = r[2] * v[0] + r[5] * v[1] + r[8] * v[2];
}

// Multiply vector by rotation matrix
void multVectorByRotation(const float *v, const float *r, float *result)
{
	result[0] = v[0] * r[0] + v[1] * r[1] + v[2] * r[2];
	result[3] = v[0] * r[3] + v[1] * r[4] + v[2] * r[5];
	result[6] = v[0] * r[6] + v[1] * r[7] + v[2] * r[8];
}

/*
// Inverse transform
void inverseTransform(const float *transform, float *inverse)
{
	float rotationInverse[9];
	float Z;

	Z =
		transform[0] * (transform[5] * transform[10] - transform[6] * transform[9]) +
		transform[4] * (transform[2] * transform[9] - transform[1] * transform[10]) +
		transform[8] * (transform[1] * transform[6] - transform[2] * transform[5]);

	rotationInverse[0] =
		(transform[5] * transform[10] - transform[6] * transform[9]) / Z;
	rotationInverse[1] =
		(transform[6] * transform[8] - transform[4] * transform[10]) / Z;
	rotationInverse[2] =
		(transform[4] * transform[9] - transform[5] * transform[8]) / Z;

	rotationInverse[3] =
		(transform[2] * transform[9] - transform[1] * transform[10]) / Z;
	rotationInverse[4] =
		(transform[0] * transform[10] - transform[2] * transform[8]) / Z;
	rotationInverse[5] =
		(transform[1] * transform[8] - transform[0] * transform[9]) / Z;

	rotationInverse[6] =
		(transform[1] * transform[6] - transform[2] * transform[5]) / Z;
	rotationInverse[7] =
		(transform[2] * transform[4] - transform[0] * transform[6]) / Z;
	rotationInverse[8] =
		(transform[0] * transform[5] - transform[1] * transform[4]) / Z;

	// Copy the inverse matrix
	inverse[0] = rotationInverse[0];
	inverse[1] = rotationInverse[3];
	inverse[2] = rotationInverse[6];
	inverse[3] = 0.0f;

	inverse[4] = rotationInverse[1];
	inverse[5] = rotationInverse[4];
	inverse[6] = rotationInverse[7];
	inverse[7] = 0.0f;

	inverse[8] = rotationInverse[2];
	inverse[9] = rotationInverse[5];
	inverse[10] = rotationInverse[8];
	inverse[11] = 0.0f;

	inverse[12] = -(
		rotationInverse[0] * transform[12] +
		rotationInverse[1] * transform[13] +
		rotationInverse[2] * transform[14]);
	inverse[13] = -(
		rotationInverse[3] * transform[12] +
		rotationInverse[4] * transform[13] +
		rotationInverse[5] * transform[14]);
	inverse[14] = -(
		rotationInverse[6] * transform[12] +
		rotationInverse[7] * transform[13] +
		rotationInverse[8] * transform[14]);
	inverse[15] = 1.0f;
}*/
