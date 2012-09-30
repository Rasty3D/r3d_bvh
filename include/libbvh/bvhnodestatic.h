/*
 * bvhnodestatic.h
 *
 *  Created on: 01 Feb 2011
 *      Author: showroom
 */

#ifndef BVHNODESTATIC_H_
#define BVHNODESTATIC_H_

/*
 * INCLUDES
 */

#include "bvhaabb.h"
#include "bvhprimitive.h"


/*
 * TYPES
 */

typedef struct
{
	int skipIndex;
	BVHaabb aabb;
	BVHPrimitive *primitive;
}BVHNodeStatic;

#endif /* BVHNODESTATIC_H_ */
