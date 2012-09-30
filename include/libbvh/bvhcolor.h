/*
 * bvhcolor.h
 *
 *  Created on: 17 Feb 2011
 *      Author: showroom
 */

#ifndef BVHCOLOR_H_
#define BVHCOLOR_H_

/*
 * INCLUDES
 */

#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <string.h>


/*
 * DEFINES
 */

	/* Color spaces */
#define BVHCOLOR_RGB					// Classical RGB
//#define BVHCOLOR_SPECTRAL_VISIBLE_10	// Visible spectra (380nm-780nm) with samples each 10 nm
//#define BVHCOLOR_SPECTRAL_VISIBLE_5	// Visible spectra (380nm-780nm) with samples each 5 nm

#ifdef BVHCOLOR_RGB
#define BVHCOLOR_COMPONENTS		3
//#define BVHCOLOR_STEPL		1
//#define BVHCOLOR_SCALE		1.0f
#define BVHCOLOR_COMPONENTS_AUX	41
#define BVHCOLOR_STEPL_AUX		10.0e-9
#define BVHCOLOR_SCALE_AUX		0.1f
#elif defined(BVHCOLOR_SPECTRAL_VISIBLE_10)
#define BVHCOLOR_COMPONENTS	41
#define BVHCOLOR_STEPL		10.0e-9
#define BVHCOLOR_SCALE		0.1f
#elif defined(BVHCOLOR_SPECTRAL_VISIBLE_5)
#define BVHCOLOR_COMPONENTS	81
#define BVHCOLOR_STEPL		5.0e-9
#define BVHCOLOR_SCALE		0.05f
#endif

	/* Init modes */
#define BVHCOLOR_INIT_RGB			0
#define BVHCOLOR_INIT_INTENSITY		1
#define BVHCOLOR_INIT_TEMP			2
#define BVHCOLOR_INIT_COMPONENTS	3
#define BVHCOLOR_INIT_SPECTRUM		4

	/* Color systems */
#define BVHCOLOR_COLORSYSTEM_NUMBER	6
#define BVHCOLOR_COLORSYSTEM_CUSTOM	"Custom"

	/* Gamma correction */
#define BVHCOLOR_GAMMA_REC709		0.0f


/*
 * TYPES
 */

typedef struct
{
	const char *name;

	float rgb2xyz[9];
	float xyz2rgb[9];
	float xyz2rgbCorrected[9];

	float whitexyz[3];
	float whitergb[3];

	float gamma;
}BVHColor_system;

typedef struct
{
	float rmax;
	float gmax;
	float bmax;

	float lrmax;
	float lgmax;
	float lbmax;

	float cMatrix[9];
}BVHColor_ramp;

typedef struct
{
	float freq;		// m
	float value;	// Normalized intensity
}BVHColor_component;

typedef struct
{
	int nComponents;
	BVHColor_component *components;
}BVHColor_spectrum;


/*
 * CLASS: BVHColor
 */

class BVHColor
{
private:
		/* Color components */
	float c[BVHCOLOR_COMPONENTS];

		/* Color tables */
#ifdef BVHCOLOR_RGB
	static float cieColorMatchAux[BVHCOLOR_COMPONENTS_AUX][3];
	static float rgbColorMatchAux[BVHCOLOR_COMPONENTS_AUX][3];
#endif
	static float cieColorMatch[BVHCOLOR_COMPONENTS][3];
	static float rgbColorMatch[BVHCOLOR_COMPONENTS][3];

		/* Color systems */
	static BVHColor_system colorSystems[BVHCOLOR_COLORSYSTEM_NUMBER];
	static BVHColor_system colorSystemCurrent;

		/* Color ramps */
	static BVHColor_ramp colorRamp;
	static float colorRampRGB[BVHCOLOR_COMPONENTS][3];

public:
		/* Constructor and destructor */
	BVHColor();
	~BVHColor();

		/* Initing functions */
	void init(int mode, void *data);
	void init(float r, float g, float b);
	void init(float intensity);
	void init(float *components);

		/* Set color system */
	static void setColorSystem(const char *name);
	static void setColorSystem(
		float xRed, float yRed, float zRed,
		float xGreen, float yGreen, float zGreen,
		float xBlue, float yBlue, float zBlue,
		float xWhite, float yWhite, float zWhite,
		float gamma);

		/* Conversion to RGB */
	void toRGB(float *rgb);

		/* Get intensity */
	float getIntensity();

		/* Print values */
	void print();

		/* Operators */
	BVHColor operator +(const BVHColor &c);
	BVHColor operator +=(const BVHColor &c);

	BVHColor operator -();
	BVHColor operator -(const BVHColor &c);
	BVHColor operator -=(const BVHColor &c);

	BVHColor operator *(const BVHColor &c);
	BVHColor operator *=(const BVHColor &c);
	BVHColor operator *(const float &c);
	BVHColor operator *=(const float &c);

	BVHColor operator /(const BVHColor &c);
	BVHColor operator /=(const BVHColor &c);
	BVHColor operator /(const float &c);
	BVHColor operator /=(const float &c);

private:
		/* Update color tables */
	static void updateColorTables();

		/* Color ramp functions */
	static void updateColorRamps();
	static float getColorRampR(float l);
	static float getColorRampG(float l);
	static float getColorRampB(float l);

		/* Normalize rgb color */
	static float normalize(float *rgb);

		/* Get a value from a freq-intensity spectrum */
	static float getValue(float frequency, BVHColor_spectrum *spectrum);

public:
		/* Gamma correction */
	static void gammaCorrect(float &c);
	static void gammaCorrectRGB(float *rgb);

	static void gammaUncorrect(float &c);
	static void gammaUncorrectRGB(float *rgb);
};

#endif /* BVHCOLOR_H_ */
