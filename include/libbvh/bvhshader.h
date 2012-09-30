/*
 * bvhshader.h
 *
 *  Created on: 24 Feb 2011
 *      Author: showroom
 */

#ifndef BVHSHADER_H_
#define BVHSHADER_H_

/*
 * INCLUDES
 */

	/* General */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <iostream>
#include <fstream>

	/* Utils */
#include "utils.h"

	/* BVH */
#include "bvhcolor.h"
#include "bvhtransform.h"


/*
 * DEFINES
 */

	/* Compilation defaults */
#define BVHSHADER_COMPILE_COMMAND	(char*)"g++ -O3 -Wall -fno-strict-aliasing -fPIC -shared"
#define BVHSHADER_COMPILE_INCPATH	(char*)"-I../include/libbvh -I../include/utils"
#define BVHSHADER_COMPILE_SRCFILES	(char*)"../src/libbvh"
#define BVHSHADER_COMPILE_SRCUTILS	(char*)"../src/utils"
#define BVHSHADER_COMPILE_SRCPATH	(char*)"../src/shaders"
#define BVHSHADER_COMPILE_SRCEXT	(char*)".cpp"
#define BVHSHADER_COMPILE_OBJPATH	(char*)"shaders"
#define BVHSHADER_COMPILE_OBJEXT	(char*)".sh"

	/* Maximum compile log size */
#define BVHSHADER_COMPILE_LOG_MAX_SIZE	16384

	/* Shader types */
#define BVHSHADER_TYPE_UNDEFINED	0
#define BVHSHADER_TYPE_JITTER_1D	1
#define BVHSHADER_TYPE_JITTER_2D	2
#define BVHSHADER_TYPE_CAMERA		3
#define BVHSHADER_TYPE_SURFACE		4
#define BVHSHADER_TYPE_LIGHT		5
#define BVHSHADER_TYPE_EMITTER		6
#define BVHSHADER_TYPE_TEXTURE		7
#define BVHSHADER_TYPE_MASK			8
#define BVHSHADER_TYPE_VERTEX		9

	/* Attribute types */
#define BVHSHADER_TYPE_INT8		0
#define BVHSHADER_TYPE_INT16	1
#define BVHSHADER_TYPE_INT32	2
#define BVHSHADER_TYPE_UINT8	3
#define BVHSHADER_TYPE_UINT16	4
#define BVHSHADER_TYPE_UINT32	5
#define BVHSHADER_TYPE_FLOAT	6

	/* Maximum name length */
#define BVHSHADER_NAME_LENGTH	3

	/* Sampling types */
#define BVHSHADER_SAMPLING_DIFFUSE				0
#define BVHSHADER_SAMPLING_SPECULAR				1
#define BVHSHADER_SAMPLING_REFLECTION			2
#define BVHSHADER_SAMPLING_REFRACTION			3
#define BVHSHADER_SAMPLING_GLOSSY_REFLECTION	4
#define BVHSHADER_SAMPLING_GLOSSY_REFRACTION	5
#define BVHSHADER_SAMPLING_MAX					32


/*
 * TYPES
 */

/*
 *	Attribute structure (8 bytes, 32 bits) -> Aligned in 32 bits
 *
 *		|  type  | length |     offset      |           name           |  size  |
 *		|   0    |   1    |   2    |   3    |   4    |   5    |   6    |   7    |
 */

typedef struct
{
	unsigned char type;					// Type of the attribute (see define types up)
										// Type sizes:
										//   int8   -> 1 byte
										//   int16  -> 2 bytes
										//   int32  -> 4 bytes
										//   uint8  -> 1 byte
										//   uint16 -> 2 bytes
										//   uint32 -> 4 bytes
										//   float  -> 4 bytes
	unsigned char length;				// Number of elements of the attribute
										// For example:
										//   1  -> Single value
										//   2  -> 2D vector
										//   3  -> 3D vector
										//   4  -> 4D vector, 2x2 matrix
										//   6  -> 2x3, 3x2 matrix
										//   8  -> 2x4, 4x2 matrix
										//   9  -> 3x3 matrix
										//   12 -> 3x4, 4x3 matrix
										//   16 -> 4x4 matrix
	unsigned short offset;				// Offset of the attribute inside the vertex buffer
	char name[BVHSHADER_NAME_LENGTH];	// Name of the attribute for reference (two characters max, the last is always '\0')
	unsigned char size;					// Size of the attribute in bytes (length * typeSize)
}BVHAttrib;


/*
 * Attribute atlas structure (8 bytes + N * 8 bytes = 32 bits + N * 32 bits) -> Aligned in 32 bits
 *
 *		|     number    |      size     | Attrib 1 | Attrib 2 | ... |
 *		| 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |  8 - 15  | 16 - 31  | ... |
 */

typedef struct
{
	int number;				// Number of attributes
	int size;				// Size in bytes of the attributes
	BVHAttrib *attrList;	// List of attributes
}BVHAttribAtlas;

	/* Aux functions */
typedef struct
{
	// Textures
	BVHColor (*getColor)(const void*, float*);
	BVHColor (*getColorUV)(const void*, float, float);
	BVHColor (*getColorUVW)(const void*, float, float, float);
	float (*getIntensity)(const void*, float*);
	float (*getIntensityUV)(const void*, float, float);
	float (*getIntensityUVW)(const void*, float, float, float);

	// Random Number Generator
	float (*rng)(void*, float, float);
}BVHShader_functions;

	/* Jitter 1D */
typedef struct
{
	// Inputs
	int kernelDim;
	int kernelIdx;

	// Outputs
	float dx;

	// Random Number Generator
	void *rng;
}BVHShader_jitter1d;

	/* Jitter 2D */
typedef struct
{
	// Inputs
	int kernelDim[2];
	int kernelIdx[2];

	// Outputs
	float dx;
	float dy;

	// Random Number Generator
	void *rng;
}BVHShader_jitter2d;

	/* Camera */
typedef struct
{
	// Inputs
	float x;
	float y;
	float dx;
	float dy;
	float wavelength;

	// Outputs
	float rayPos[3];
	float rayDir[3];
}BVHShader_camera;

	/* Surface & light */
typedef struct
{
	// Inputs
	float rayInput[3];
	float rayOutput[3];

	// Fixed attributes
	float pos[3];
	float normal[3];
	float primCoords[3];

	// Attributes
	BVHAttribAtlas *attribAtlas;
	char *attribBuffer;
	int attribBufferSize;	// This should be hidden

	// Outputs
	BVHColor color;
	BVHColor emission;
}BVHShader_surface, BVHShader_light;

	/* Light */
/*
typedef struct
{
	// Inputs
	float rayOutput[3];

	// Fixed attributes
	float pos[3];
	float normal[3];
	float primCoords[3];

	// Attributes
	BVHAttribAtlas *attribAtlas;
	char *attribBuffer;
	int attribBufferSize;	// This should be hidden

	// Outputs
	BVHColor color;
}BVHShader_light;*/

	/* Emitter */
typedef struct
{
	// Outputs
	float rayPos[3];
	float rayDir[3];
	float normal[3];
	float pdf;

	// Random Number Generator
	void *rng;
}BVHShader_emitter;

	/* Texture */
typedef struct
{
	// Inputs
	float coords[3];

	// Outputs
	BVHColor color;
}BVHShader_texture;

	/* Mask */
typedef struct
{
	// Inputs
	float coords[3];

	// Outputs
	float intensity;
}BVHShader_mask;

	/* Vertex */
typedef struct
{
	// Input
	BVHTransform transform;

	// Attributes
	BVHAttribAtlas *attribAtlas;
	char *attribBuffer;

	// Output
	char *newAttribBuffer;
}BVHShader_vertex;

	/* Sampling */
typedef struct
{
	int type;
	float exponent;
	float indexRel;
	float pdf;
	float pdfAcc;
}BVHShader_sampling;

typedef struct
{
	int samplingNumber;
	BVHShader_sampling samplings[BVHSHADER_SAMPLING_MAX];
}BVHShader_samplingList;


/*
 * CLASS: BVHShader
 */

class BVHShader
{
private:
		/* Dynamic library */
	void *handle;
	int (*pGetType)();
	void *(*pGetParams)();
	void (*pSetParam)(void*, const char*, void*);
	void (*pExecute)(void*, BVHShader_functions*, void*);
	void *params;

	// Only for surface shader (optional functions)
	void (*pGetSampling)(void*, BVHShader_samplingList*);
	void (*pGetNormal)(void*, BVHShader_functions*, void*);
	BVHShader_samplingList *samplingList;

		/* Compilation options */
	static char *compileCommand;
	static char *compileIncPath;
	static char *compileSrcFiles;
	static char *compileSrcUtils;
	static char *compileSrcPath;
	static char *compileSrcExt;
	static char *compileObjPath;
	static char *compileObjExt;

		/* Compile log */
	static int compileLogSize;
	static char compileLog[BVHSHADER_COMPILE_LOG_MAX_SIZE];

public:
		/* Auxiliary functions for the shaders */
	static BVHShader_functions functions;

public:
		/* Constructor and destructor */
	BVHShader();
	~BVHShader();

		/* Compile */
	static void setCompileCommand(const char *command);
	static void setCompileIncPath(const char *path);
	static void setCompileSrcFiles(const char *files);
	static void setCompileSrcUtils(const char *utils);
	static void setCompileSrcPath(const char *path);
	static void setCompileSrcExt(const char *ext);
	static void setCompileObjPath(const char *path);
	static void setCompileObjExt(const char *ext);
	int compile(const char *name, int verbose = 0);
	int getCompileLogSize();
	void getCompileLog(char *info);
	void printCompileLog();


		/* Load and unload dynamic library */
	int compileAndLoad(const char *name, int verbose = 0);
	int load(const char *name);
	void unload();

		/* Call library functions */
	int getType();
	void setParam(const char *name, void *value);
	void execute(void *env);

	// Only for surface shader (optional functions)
	BVHShader_samplingList *getSampling();
	void getNormal(void *env);

		/* Set auxiliary functions */
	static void setFunc_rng(float(*func)(void*, float, float));
	static void setFunc_getColor(BVHColor(*func)(const void*, float*));
	static void setFunc_getColorUV(BVHColor(*func)(const void*, float, float));
	static void setFunc_getColorUVW(BVHColor(*func)(const void*, float, float, float));
	static void setFunc_getIntensity(float(*func)(const void*, float*));
	static void setFunc_getIntensityUV(float(*func)(const void*, float, float));
	static void setFunc_getIntensityUVW(float(*func)(const void*, float, float, float));

private:
		/* Look for required classes */
	int checkClass(const char *name, const char *className);
};


#endif /* BVHSHADER_H_ */
