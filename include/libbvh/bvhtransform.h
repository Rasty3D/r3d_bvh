/*
 * bvhtransform.h
 *
 *  Created on: 4 May 2011
 *      Author: showroom
 */

#ifndef BVHTRANSFORM_H_
#define BVHTRANSFORM_H_

/*
 * INCLUDES
 */

#include <string.h>
#include <iostream>

#include "mathaux.h"


/*
 * DEFINES
 */



/*
 * CLASS: BVHTransform
 */

class BVHTransform
{
private:
	float position[3];
	float rotation[9];

public:
		/* Constructor and destructor */
	BVHTransform();
	~BVHTransform();

		/* Set identity matrix */
	void identity();

		/* Translate */
	void translate(float x, float y, float z);
	void translate(float *position);

		/* Rotate */
	void rotate_angleAxis(float angle, float x, float y, float z);
	void rotate_angleAxis(float angle, float *axis);
	void rotate_angleAxis(float *angleAxis);
	void rotate_matrix(float *matrix);
	void rotate_quat(float w, float i, float j, float k);
	void rotate_quat(float *quat);
	void rotate_euler(float x, float y, float z, int mode);
	void rotate_euler(float *angles, int mode);

		/* Scales */
	void scale(float x, float y, float z);
	void scale(float *factor);
	void scale(float factor);

		/* Look at */
	void lookat_obj(
		float posx, float posy, float posz,
		float objx, float objy, float objz,
		float upx, float upy, float upz);
	void lookat_obj(float *pos, float *obj, float *up);
	void lookat_dir(
		float posx, float posy, float posz,
		float dirx, float diry, float dirz,
		float upx, float upy, float upz);
	void lookat_dir(float *pos, float *dir, float *up);
	void lookat_obj(
		float objx, float objy, float objz,
		float upx, float upy, float upz);
	void lookat_obj(float *obj, float *up);
	void lookat_dir(
		float dirx, float diry, float dirz,
		float upx, float upy, float upz);
	void lookat_dir(float *dir, float *up);

		/* Transforms */
	void transform(BVHTransform &transform);
	void transformVector(const float *v, float *result);
	void transformVector(float *v);
	void rotateVector(const float *v, float *result);
	void rotateVector(float *v);
};

#endif /* BVHTRANSFORM_H_ */
