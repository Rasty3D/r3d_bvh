/*
 * bvhmesh.h
 *
 *  Created on: 10 May 2011
 *      Author: showroom
 */

#ifndef BVHMESH_H_
#define BVHMESH_H_

/*
 * INCLUDES
 */

	/* General */
#include <stdlib.h>
#include <list>
#include <iostream>
#include <string.h>

	/* Utils */
#include "path.h"
#include "pkt.h"

	/* BVH */
#include "bvhprimitive.h"


/*
 * DEFINES
 */

#define BVHMESH_LINESIZE		256
#define BVHMESH_MAXINDICES		256

#define BVHMESH_ATLAS_VERTEX					0
#define BVHMESH_ATLAS_VERTEX_TEXCOORD			1
#define BVHMESH_ATLAS_VERTEX_TEXCOORD_NORMAL	2
#define BVHMESH_ATLAS_VERTEX_NORMAL				3


/*
 * TYPES
 */

typedef struct
{
	float x;
	float y;
	float z;
}BVHMesh_vertex;

typedef struct
{
	float x;
	float y;
}BVHMesh_texcoord;

typedef struct
{
	float x;
	float y;
	float z;
}BVHMesh_normal;


/*
 * CLASS: BVHMesh
 */

class BVHMesh
{
public:
		/* Load a mesh from a file */
	static int load(const char *filename, std::list<BVHPrimitive> &primitives, BVHShader *material = NULL);

private:
		/* Load in different formats */
	static int loadOBJ(const char *filename, std::list<BVHPrimitive> &primitives, BVHShader *material = NULL);
	static int loadR3D(const char *filename, std::list<BVHPrimitive> &primitives, BVHShader *material = NULL);
	static int loadVBO(const char *filename, std::list<BVHPrimitive> &primitives, BVHShader *material = NULL);
};

#endif /* BVHIMPORTOBJ_H_ */
