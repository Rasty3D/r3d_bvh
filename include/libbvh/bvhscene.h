/*
 * bvhscene.h
 *
 *  Created on: 04 Apr 2011
 *      Author: showroom
 */

#ifndef BVHSCENE_H_
#define BVHSCENE_H_

/*
 * INCLUDES
 */

	/* General */
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <math.h>
#include <list>

	/* Utils */
#include "indexedlist.h"
#include "nmlparser.h"

	/* BVH */
#include "bvhcamera.h"
#include "bvhlight.h"
#include "bvhmesh.h"
#include "bvhprimitive.h"
#include "bvhshader.h"
#include "bvhtexture.h"


/*
 * DEFINES
 */

	/* Flags for global */
#define BVHSCENE_GLOBAL_SKY				0x01

	/* Flags for texture */
#define BVHSCENE_TEXTURE_NAME			0x01
#define BVHSCENE_TEXTURE_IMG			0x02
#define BVHSCENE_TEXTURE_WRAPT			0x04
#define BVHSCENE_TEXTURE_WRAPS			0x08
#define BVHSCENE_TEXTURE_MUST			(BVHSCENE_TEXTURE_NAME | BVHSCENE_TEXTURE_IMG)

	/* Flags for shader-texture, shader-mask and material */
#define BVHSCENE_SHADER_NAME			0x01
#define BVHSCENE_SHADER_SHADER			0x02
#define BVHSCENE_SHADER_PARAM			0x04
#define BVHSCENE_SHADER_MUST			(BVHSCENE_SHADER_NAME | BVHSCENE_SHADER_SHADER)

	/* Flags for camera */
#define BVHSCENE_CAMERA_NAME			0x01
#define BVHSCENE_CAMERA_SHADER			0x02
#define BVHSCENE_CAMERA_JITTER			0x04
#define BVHSCENE_CAMERA_PARAM_SHADER	0x08
#define BVHSCENE_CAMERA_PARAM_JITTER	0x10
#define BVHSCENE_CAMERA_MUST			(BVHSCENE_CAMERA_NAME | BVHSCENE_CAMERA_SHADER | BVHSCENE_CAMERA_JITTER)

	/* Flags for light */
#define BVHSCENE_LIGHT_NAME				0x01
#define BVHSCENE_LIGHT_SHADER			0x02
#define BVHSCENE_LIGHT_EMITTER_SHADER	0x04
#define BVHSCENE_LIGHT_EMITTER_OBJECT	0x08
#define BVHSCENE_LIGHT_PARAM_SHADER		0x10
#define BVHSCENE_LIGHT_PARAM_EMITTER	0x20
#define BVHSCENE_LIGHT_MUST_1			(BVHSCENE_LIGHT_NAME | BVHSCENE_LIGHT_SHADER | BVHSCENE_LIGHT_EMITTER_SHADER)
#define BVHSCENE_LIGHT_MUST_2			(BVHSCENE_LIGHT_NAME | BVHSCENE_LIGHT_EMITTER_OBJECT)

	/* Flags for object */
#define BVHSCENE_OBJECT_NAME			0x01
#define BVHSCENE_OBJECT_MESH			0x02
#define BVHSCENE_OBJECT_SHADER			0x04
#define BVHSCENE_OBJECT_PARAM			0x08
#define BVHSCENE_OBJECT_MATERIAL		0x10
#define BVHSCENE_OBJECT_MUST			(BVHSCENE_OBJECT_NAME | BVHSCENE_OBJECT_MESH | BVHSCENE_OBJECT_SHADER | BVHSCENE_OBJECT_MATERIAL)


/*
 * TYPES
 */

typedef struct
{
	BVHTransform transform;
	BVHLight *light;
}BVHLightRef;

typedef struct
{
	BVHLightRef *lightRef;
	BVHShader *material;
}BVHLightInfo;

typedef struct
{
	BVHTransform transform;
	BVHCamera *camera;
}BVHCameraRef;

typedef struct
{
	BVHShader shader;
	BVHShader *material;
	std::list<BVHPrimitive> primitives;
}BVHObject;

typedef struct
{
	BVHTransform transform;
	BVHObject *object;
}BVHObjectRef;


/*
 * CLASS: BVHScene
 */

class BVH;

class BVHScene
{
	friend class BVH;
private:
	// Global options
	BVHColor skyColor;

	// Textures & materials
	IndexedList<BVHTexture> textures;
	IndexedList<BVHShader> shaderTextures;
	IndexedList<BVHShader> shaderMasks;
	IndexedList<BVHShader> materials;

	// Lights
	IndexedList<BVHLight> lights;
	std::list<BVHLightRef> lightRefs;

	// Cameras
	IndexedList<BVHCamera> cameras;
	std::list<BVHCameraRef> cameraRefs;
	BVHCameraRef *cameraCurrent;

	// Objects
	IndexedList<BVHObject> objects;
	std::list<BVHObjectRef> objectRefs;

	// Final primitive list
	std::list<BVHPrimitive> primitives;

public:
		/* Constructor and destructor */
	BVHScene();
	~BVHScene();

		/* Load scene */
	int load(const char *filename);

		/* Clear scene */
	void clear();

		/* Get camera ray */
	void getCameraRay(
		float x, float y,
		int *kernelDim, int *kernelIdx,
		float *rayPos, float *rayDir,
		gsl_rng *rng);

		/* Get light ray */
	// It returns the pointer of the selected light
	BVHLightInfo getLightRay(float *rayPos, float *rayDir, float *normal, float &pdf, gsl_rng *rng);
	void getLightColor(BVHLightInfo &lightInfo, BVHShader_light *lightEnv);


private:
		/* Load general from parser */
	int loadGeneral(NMLParser &parser);

		/* Load parts */
	int loadGlobal(NMLParser &parser);
	int loadTexture(NMLParser &parser, BVHTexture *texture, char *name);
	int loadShader(NMLParser &parser, BVHShader *shader, char *name, int standalone);
	int loadCamera(NMLParser &parser, BVHCamera *camera, char *name, int standalone);
	int loadLight(NMLParser &parser, BVHLight *light, char *name, int standalone);
	int loadObject(NMLParser &parser, BVHObject *object, char *name, int standalone);
	int loadTransform(NMLParser &parser, BVHTransform transform);

		/* Load params */
	int loadColor(NMLParser &parser, BVHColor *color);
	int loadColorIntensity(NMLParser &parser, BVHColor *color);
	int loadColorRGB(NMLParser &parser, BVHColor *color);
	int loadColorTemp(NMLParser &parser, BVHColor *color);
	int loadColorSpectrum(NMLParser &parser, BVHColor *color);
	int loadTranslate(NMLParser &parser, BVHTransform *transform);
	int loadRotate(NMLParser &parser, BVHTransform *transform);
	int loadScale(NMLParser &parser, BVHTransform *transform);
	int loadLookAtObj(NMLParser &parser, BVHTransform *transform);
	int loadLookAtDir(NMLParser &parser, BVHTransform *transform);
	int loadParam(NMLParser &parser, BVHShader *shader);

		/* Generate final primitive list */
	void transformPrimitives();
};

#endif /* BVHSCENE_H_ */
