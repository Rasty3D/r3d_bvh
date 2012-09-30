/*
 * bvhtexture.h
 *
 *  Created on: 7 Mar 2011
 *      Author: showroom
 */

#ifndef BVHTEXTURE_H_
#define BVHTEXTURE_H_

/*
 * INCLUDES
 */

	/* General */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <png.h>
#include <jpeglib.h>
#include <IL/il.h>

	/* BVH */
#include "bvhcolor.h"


/*
 * DEFINES
 */

	/* Texture format */
#define BVHTEXTURE_FORMAT_INTENSITY	0
#define BVHTEXTURE_FORMAT_RGB		1
#define BVHTEXTURE_FORMAT_RGBA		2
#define BVHTEXTURE_FORMAT_BGR		3
#define BVHTEXTURE_FORMAT_BGRA		4

	/* Texture data format */
#define BVHTEXTURE_INT8		0	// Integer format always without sign, positive
#define BVHTEXTURE_INT16	1	// Integer format always without sign, positive
#define BVHTEXTURE_FLOAT32	2

	/* Wrap modes */
#define BVHTEXTURE_WRAP_CLAMP			0
#define BVHTEXTURE_WRAP_MIRRORED_REPEAT	1
#define BVHTEXTURE_WRAP_REPEAT			2


/*
 * CLASS: BVHTexture
 */

class BVHTexture
{
private:
		/* Buffer to store the data */
	int bufferSize;
	unsigned char *buffer;

		/* Texture dimensions */
	int width;
	int height;
	int depth;

		/* Channels, format */
	int channels;
	int format;
	int dataFormat;

		/* Wrap modes */
	int wraps;
	int wrapt;
	int wrapr;

public:
		/* Constructor and destructor */
	BVHTexture();
	~BVHTexture();

		/* Init function */
	static void init();

		/* Load */
	int load(const char *filename);

		/* Set parameters */
	void setWraps(int wrap);
	void setWrapt(int wrap);
	void setWrapr(int wrap);

		/* Get info */
	int getWidth();
	int getHeight();
	int getDepth();
	int getChannels();
	int getFormat();
	int getDataFormat();
	int getWraps();
	int getWrapt();
	int getWrapr();

		/* Get color */
	BVHColor getColor(float x, float y);
	BVHColor getColor(float x, float y, float z);
	BVHColor getColor(float *texCoords);

		/* Get intensity */
	float getIntensity(float x, float y);
	float getIntensity(float x, float y, float z);
	float getIntensity(float *texCoords);

		/* Static functions */
	static BVHColor getColorUV(const void *texture, float x, float y);
	static BVHColor getColorUVW(const void *texture, float x, float y, float z);
	static BVHColor getColor(const void *texture, float *texCoords);
	static float getIntensityUV(const void *texture, float x, float y);
	static float getIntensityUVW(const void *texture, float x, float y, float z);
	static float getIntensity(const void *texture, float *texCoords);

private:
		/* Aux functions */
	void getInperp(int type, int w, float p, int *pint, float &factor);
};


#endif /* BVHTEXTURE_H_ */
