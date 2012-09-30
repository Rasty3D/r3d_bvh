/*
 * math.h
 *
 *   Created on: 5 May 2011
 *       Author: Jesus Ortiz
 *
 *  Description: TODO
 *
 */

#ifndef MATH_H_
#define MATH_H_

/*
 * INCLUDES
 */

#include <math.h>
#include <iostream>


/*
 * DEFINES
 */

#define MATH_EULER_XYZ	0
#define MATH_EULER_XZY	1
#define MATH_EULER_YXZ	2
#define MATH_EULER_YZX	3
#define MATH_EULER_ZXY	4
#define MATH_EULER_ZYX	5

#define PI			3.141592654f
#define DEG2RAD		0.017453293f


/*
 * FUNCTIONS
 */

	/* Basic functions */
float max(float value1, float value2);
float min(float value1, float value2);
float sign(float value);
float smooth(float value);

	/* Vector operations */
inline void normalize(float *v)
{
	float m = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	v[0] /= m;
	v[1] /= m;
	v[2] /= m;
}

inline float dot(float *v1, float *v2)
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

inline float dotPos(float *v1, float *v2)
{
	return max(0.0f, v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]);
}

inline float dotAbs(float *v1, float *v2)
{
	return fabsf(v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]);
}

inline void cross(float *v1, float *v2, float *r)
{
	r[0] = v1[1] * v2[2] - v1[2] * v2[1];
	r[1] = v1[2] * v2[0] - v1[0] * v2[2];
	r[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

inline void invert(float *v)
{
	v[0] = -v[0];
	v[1] = -v[1];
	v[2] = -v[2];
}

inline void initVector(float *v, float x, float y, float z)
{
	v[0] = x;
	v[1] = y;
	v[2] = z;
}

inline void reflect(float *n, float *i, float *o)
{
	float inputDot = dot(n, i);

	o[0] = i[0] - 2.0f * inputDot * n[0];
	o[1] = i[1] - 2.0f * inputDot * n[1];
	o[2] = i[2] - 2.0f * inputDot * n[2];
}

inline void refract(float *n, float *i, float *o, float indexRel)
{
	float cosThetai = dot(i, n);

	if (cosThetai > 0.0f)
		indexRel = 1.0f / indexRel;

	float sinThetao2 = indexRel * indexRel * (1.0f - cosThetai * cosThetai);
	float factor;

	if (sinThetao2 > 1.0f)
	{
		// Arrived to the limit angle
		// Make reflection instead of refraction
		reflect(n, i, o);
	}
	else
	{
		if (cosThetai > 0.0f)
			factor = sqrtf(1.0f - sinThetao2) - cosThetai * indexRel;
		else
			factor = -sqrtf(1.0f - sinThetao2) - cosThetai * indexRel;

		o[0] = i[0] * indexRel + n[0] * factor;
		o[1] = i[1] * indexRel + n[1] * factor;
		o[2] = i[2] * indexRel + n[2] * factor;
		normalize(o);
	}
}

inline float fresnel(float *n, float *i, float indexRel)
{
	float cosThetai = dot(n, i);

	if (cosThetai > 0.0f)
		indexRel = 1.0f / indexRel;

	float sinThetao2 = indexRel * indexRel * (1.0f - cosThetai * cosThetai);

	if (sinThetao2 > 1.0f)
	{
		return 1.0f;
	}
	else
	{
		cosThetai = fabsf(cosThetai);
		float cosThetao = sqrtf(1.0f - sinThetao2);
		float Rs = (cosThetai - indexRel * cosThetao) / (cosThetai + indexRel * cosThetao);
		float Rp = (cosThetao - indexRel * cosThetai) / (cosThetao + indexRel * cosThetai);
		return (Rs * Rs + Rp * Rp) * 0.5f;
	}
}


	/* Rotation conversions */
// Rotation matrix from <angle, axis>
void angleAxis2Matrix(const float *angleAxis, float *matrix);
void angleAxis2Matrix(float angle, const float *axis, float *matrix);
void angleAxis2Matrix(float angle, float x, float y, float z, float *matrix);

// Rotation matrix from quaternion
void quat2Matrix(const float *quat, float *matrix);
void quat2Matrix(float w, float i, float j, float k, float *matrix);

// Rotation matrix from Euler angles
void euler2Matrix(const float *xyz, int mode, float *matrix);
void euler2Matrix(float x, float y, float z, int mode, float *matrix);

// Euler to quaterion
void euler2Quat(const float *xyz, int mode, float *quat);
void euler2Quat(float x, float y, float z, int mode, float *quat);

// Matrix to quaternion
void matrix2Quat(const float *matrix, float *quat);

// <Angle, axis> to quaterion
void angleAxis2Quat(const float *angleAxis, float *quat);
void angleAxis2Quat(float angle, const float *axis, float *quat);
void angleAxis2Quat(float angle, float x, float y, float z, float *quat);

	/* Transform operations */
// Multiply rotation matrix by rotation matrix
void multRotationByRotation(const float *t1, const float *t2, float *result);

// Multiply rotation matrix by vector
void multRotationByVector(const float *r, const float *v, float *result);

// Multiply vector by rotation matrix
void multVectorByRotation(const float *v, const float *r, float *result);

#endif	/* MATH_H_ */
