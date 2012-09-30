/*
 * bvhcolor.cpp
 *
 *  Created on: 17 Feb 2011
 *      Author: showroom
 */

/*
 * INCLUDES
 */

#include "bvhcolor.h"


/*
 * NAMESPACES
 */

using namespace std;


/*
 * CLASS: BVHColor
 */

	/* Color tables */

#ifdef BVHCOLOR_RGB
float BVHColor::cieColorMatchAux[BVHCOLOR_COMPONENTS_AUX][3] = {
{0.001400, 0.000000, 0.006500}, {0.004200, 0.000100, 0.020100},
{0.014300, 0.000400, 0.067900}, {0.043500, 0.001200, 0.207400},
{0.134400, 0.004000, 0.645600}, {0.283900, 0.011600, 1.385600},
{0.348300, 0.023000, 1.747100}, {0.336200, 0.038000, 1.772100},
{0.290800, 0.060000, 1.669200}, {0.195400, 0.091000, 1.287600},
{0.095600, 0.139000, 0.813000}, {0.032000, 0.208000, 0.465200},
{0.004900, 0.323000, 0.272000}, {0.009300, 0.503000, 0.158200},
{0.063300, 0.710000, 0.078200}, {0.165500, 0.862000, 0.042200},
{0.290400, 0.954000, 0.020300}, {0.433400, 0.995000, 0.008700},
{0.594500, 0.995000, 0.003900}, {0.762100, 0.952000, 0.002100},
{0.916300, 0.870000, 0.001700}, {1.026300, 0.757000, 0.001100},
{1.062200, 0.631000, 0.000800}, {1.002600, 0.503000, 0.000300},
{0.854400, 0.381000, 0.000200}, {0.642400, 0.265000, 0.000000},
{0.447900, 0.175000, 0.000000}, {0.283500, 0.107000, 0.000000},
{0.164900, 0.061000, 0.000000}, {0.087400, 0.032000, 0.000000},
{0.046800, 0.017000, 0.000000}, {0.022700, 0.008200, 0.000000},
{0.011400, 0.004100, 0.000000}, {0.005800, 0.002100, 0.000000},
{0.002900, 0.001000, 0.000000}, {0.001400, 0.000500, 0.000000},
{0.000700, 0.000200, 0.000000}, {0.000300, 0.000100, 0.000000},
{0.000200, 0.000100, 0.000000}, {0.000100, 0.000000, 0.000000},
{0.000000, 0.000000, 0.000000}};
float BVHColor::cieColorMatch[BVHCOLOR_COMPONENTS][3];
#elif defined(BVHCOLOR_SPECTRAL_VISIBLE_10)
float BVHColor::cieColorMatch[BVHCOLOR_COMPONENTS][3] = {
{0.001400, 0.000000, 0.006500}, {0.004200, 0.000100, 0.020100},
{0.014300, 0.000400, 0.067900}, {0.043500, 0.001200, 0.207400},
{0.134400, 0.004000, 0.645600}, {0.283900, 0.011600, 1.385600},
{0.348300, 0.023000, 1.747100}, {0.336200, 0.038000, 1.772100},
{0.290800, 0.060000, 1.669200}, {0.195400, 0.091000, 1.287600},
{0.095600, 0.139000, 0.813000}, {0.032000, 0.208000, 0.465200},
{0.004900, 0.323000, 0.272000}, {0.009300, 0.503000, 0.158200},
{0.063300, 0.710000, 0.078200}, {0.165500, 0.862000, 0.042200},
{0.290400, 0.954000, 0.020300}, {0.433400, 0.995000, 0.008700},
{0.594500, 0.995000, 0.003900}, {0.762100, 0.952000, 0.002100},
{0.916300, 0.870000, 0.001700}, {1.026300, 0.757000, 0.001100},
{1.062200, 0.631000, 0.000800}, {1.002600, 0.503000, 0.000300},
{0.854400, 0.381000, 0.000200}, {0.642400, 0.265000, 0.000000},
{0.447900, 0.175000, 0.000000}, {0.283500, 0.107000, 0.000000},
{0.164900, 0.061000, 0.000000}, {0.087400, 0.032000, 0.000000},
{0.046800, 0.017000, 0.000000}, {0.022700, 0.008200, 0.000000},
{0.011400, 0.004100, 0.000000}, {0.005800, 0.002100, 0.000000},
{0.002900, 0.001000, 0.000000}, {0.001400, 0.000500, 0.000000},
{0.000700, 0.000200, 0.000000}, {0.000300, 0.000100, 0.000000},
{0.000200, 0.000100, 0.000000}, {0.000100, 0.000000, 0.000000},
{0.000000, 0.000000, 0.000000}};
#elif defined(BVHCOLOR_SPECTRAL_VISIBLE_5)
float BVHColor::cieColorMatch[BVHCOLOR_COMPONENTS][3] = {
{0.001400, 0.000000, 0.006500}, {0.002200, 0.000100, 0.010500},
{0.004200, 0.000100, 0.020100}, {0.007600, 0.000200, 0.036200},
{0.014300, 0.000400, 0.067900}, {0.023200, 0.000600, 0.110200},
{0.043500, 0.001200, 0.207400}, {0.077600, 0.002200, 0.371300},
{0.134400, 0.004000, 0.645600}, {0.214800, 0.007300, 1.039100},
{0.283900, 0.011600, 1.385600}, {0.328500, 0.016800, 1.623000},
{0.348300, 0.023000, 1.747100}, {0.348100, 0.029800, 1.782600},
{0.336200, 0.038000, 1.772100}, {0.318700, 0.048000, 1.744100},
{0.290800, 0.060000, 1.669200}, {0.251100, 0.073900, 1.528100},
{0.195400, 0.091000, 1.287600}, {0.142100, 0.112600, 1.041900},
{0.095600, 0.139000, 0.813000}, {0.058000, 0.169300, 0.616200},
{0.032000, 0.208000, 0.465200}, {0.014700, 0.258600, 0.353300},
{0.004900, 0.323000, 0.272000}, {0.002400, 0.407300, 0.212300},
{0.009300, 0.503000, 0.158200}, {0.029100, 0.608200, 0.111700},
{0.063300, 0.710000, 0.078200}, {0.109600, 0.793200, 0.057300},
{0.165500, 0.862000, 0.042200}, {0.225700, 0.914900, 0.029800},
{0.290400, 0.954000, 0.020300}, {0.359700, 0.980300, 0.013400},
{0.433400, 0.995000, 0.008700}, {0.512100, 1.000000, 0.005700},
{0.594500, 0.995000, 0.003900}, {0.678400, 0.978600, 0.002700},
{0.762100, 0.952000, 0.002100}, {0.842500, 0.915400, 0.001800},
{0.916300, 0.870000, 0.001700}, {0.978600, 0.816300, 0.001400},
{1.026300, 0.757000, 0.001100}, {1.056700, 0.694900, 0.001000},
{1.062200, 0.631000, 0.000800}, {1.045600, 0.566800, 0.000600},
{1.002600, 0.503000, 0.000300}, {0.938400, 0.441200, 0.000200},
{0.854400, 0.381000, 0.000200}, {0.751400, 0.321000, 0.000100},
{0.642400, 0.265000, 0.000000}, {0.541900, 0.217000, 0.000000},
{0.447900, 0.175000, 0.000000}, {0.360800, 0.138200, 0.000000},
{0.283500, 0.107000, 0.000000}, {0.218700, 0.081600, 0.000000},
{0.164900, 0.061000, 0.000000}, {0.121200, 0.044600, 0.000000},
{0.087400, 0.032000, 0.000000}, {0.063600, 0.023200, 0.000000},
{0.046800, 0.017000, 0.000000}, {0.032900, 0.011900, 0.000000},
{0.022700, 0.008200, 0.000000}, {0.015800, 0.005700, 0.000000},
{0.011400, 0.004100, 0.000000}, {0.008100, 0.002900, 0.000000},
{0.005800, 0.002100, 0.000000}, {0.004100, 0.001500, 0.000000},
{0.002900, 0.001000, 0.000000}, {0.002000, 0.000700, 0.000000},
{0.001400, 0.000500, 0.000000}, {0.001000, 0.000400, 0.000000},
{0.000700, 0.000200, 0.000000}, {0.000500, 0.000200, 0.000000},
{0.000300, 0.000100, 0.000000}, {0.000200, 0.000100, 0.000000},
{0.000200, 0.000100, 0.000000}, {0.000100, 0.000000, 0.000000},
{0.000100, 0.000000, 0.000000}, {0.000100, 0.000000, 0.000000},
{0.000000, 0.000000, 0.000000}};
#endif

#ifdef BVHCOLOR_RGB
float BVHColor::rgbColorMatchAux[BVHCOLOR_COMPONENTS_AUX][3];
#endif
float BVHColor::rgbColorMatch[BVHCOLOR_COMPONENTS][3];
float BVHColor::colorRampRGB[BVHCOLOR_COMPONENTS][3];


	/* Color systems */

BVHColor_system BVHColor::colorSystems[BVHCOLOR_COLORSYSTEM_NUMBER] = {
{"NTSC",
	{0.6700f,  0.3300f,  0.0000f,  0.2100f,  0.7100f,  0.0800f,  0.1400f,  0.0800f,  0.7800f},
	{0.5474f, -0.2574f,  0.0264f, -0.1526f,  0.5226f, -0.0536f, -0.0826f, -0.0074f,  0.4064f},
	{1.9099f, -0.9846f,  0.0583f, -0.5324f,  1.9991f, -0.1184f, -0.2882f, -0.0283f,  0.8979f},
	{0.3101f, 0.3162f, 0.3737f},
	{0.2866f, 0.2614f, 0.4526f},
	BVHCOLOR_GAMMA_REC709},
{"EBU (PAL/SECAM)",
	{0.6400f,  0.3300f,  0.0300f,  0.2900f,  0.6000f,  0.1100f,  0.1500f,  0.0600f,  0.7900f},
	{0.4674f, -0.2589f,  0.0183f, -0.2126f,  0.5011f, -0.0617f, -0.0726f,  0.0111f,  0.2883f},
	{3.0651f, -0.9690f,  0.0679f, -1.3942f,  1.8755f, -0.2290f, -0.4761f,  0.0415f,  1.0698f},
	{0.3127f, 0.3291f, 0.3582f},
	{0.1525f, 0.2672f, 0.2695f},
	BVHCOLOR_GAMMA_REC709},
{"SMPTE",
	{0.6300f,  0.3400f,  0.0300f,  0.3100f,  0.5950f,  0.0950f,  0.1550f,  0.0700f,  0.7750f},
	{0.4545f, -0.2614f,  0.0145f, -0.2255f,  0.4836f, -0.0506f, -0.0705f,  0.0086f,  0.2695f},
	{3.5083f, -1.0687f,  0.0563f, -1.7410f,  1.9772f, -0.1971f, -0.5444f,  0.0352f,  1.0507f},
	{0.3127f, 0.3291f, 0.3582f},
	{0.1525f, 0.2672f, 0.2695f},
	BVHCOLOR_GAMMA_REC709},
{"HDTV",
	{0.6700f,  0.3300f,  0.0000f,  0.2100f,  0.7100f,  0.0800f,  0.1500f,  0.0600f,  0.7900f},
	{0.5561f, -0.2607f,  0.0264f, -0.1539f,  0.5293f, -0.0536f, -0.0939f,  0.0093f,  0.4064f},
	{2.0423f, -0.8937f,  0.0638f, -0.5652f,  1.8144f, -0.1295f, -0.3449f,  0.0319f,  0.9821f},
	{0.3127f, 0.3291f, 0.3582f},
	{0.1525f, 0.2672f, 0.2695f},
	BVHCOLOR_GAMMA_REC709},
{"CIE",
	{0.7355f,  0.2645f,  0.0000f,  0.2658f,  0.7243f,  0.0099f,  0.1669f,  0.0085f,  0.8246f},
	{0.5972f, -0.2181f,  0.0026f, -0.2175f,  0.6065f, -0.0073f, -0.1186f,  0.0379f,  0.4624f},
	{2.2882f, -0.5116f,  0.0057f, -0.8335f,  1.4227f, -0.0159f, -0.4545f,  0.0889f,  1.0099f},
	{0.3333f, 0.3333f, 0.3334f},
	{0.2610f, 0.4263f, 0.4579f},
	BVHCOLOR_GAMMA_REC709},
{"CIE REC 709",
	{0.6400f,  0.3300f,  0.0300f,  0.3000f,  0.6000f,  0.1000f,  0.1500f,  0.0600f,  0.7900f},
	{0.4680f, -0.2589f,  0.0150f, -0.2220f,  0.5011f, -0.0550f, -0.0720f,  0.0111f,  0.2850f},
	{3.2430f, -0.9690f,  0.0557f, -1.5383f,  1.8755f, -0.2041f, -0.4989f,  0.0415f,  1.0577f},
	{0.3127f, 0.3291f, 0.3582f},
	{0.1525f, 0.2672f, 0.2695f},
	BVHCOLOR_GAMMA_REC709}};

BVHColor_system BVHColor::colorSystemCurrent = {
	"HDTV",
	{0.6700f,  0.3300f,  0.0000f,  0.2100f,  0.7100f,  0.0800f,  0.1500f,  0.0600f,  0.7900f},
	{0.5561f, -0.2607f,  0.0264f, -0.1539f,  0.5293f, -0.0536f, -0.0939f,  0.0093f,  0.4064f},
	{2.0423f, -0.8937f,  0.0638f, -0.5652f,  1.8144f, -0.1295f, -0.3449f,  0.0319f,  0.9821f},
	{0.3127f, 0.3291f, 0.3582f},
	{0.1525f, 0.2672f, 0.2695f},
	BVHCOLOR_GAMMA_REC709};


	/* Color ramps */

BVHColor_ramp BVHColor::colorRamp;


	/* Constructor and destructor */

BVHColor::BVHColor()
{
	return;
}

BVHColor::~BVHColor()
{
	return;
}


	/* Initing function */

void BVHColor::init(int mode, void *data)
{
	if (mode == BVHCOLOR_INIT_RGB)
	{
		// The data is the rgb color
		this->init(((float*)data)[0], ((float*)data)[1], ((float*)data)[2]);
	}
	else if (mode == BVHCOLOR_INIT_INTENSITY)
	{
		// The data is color intensity
		this->init(((float*)data)[0]);
	}
	else if (mode == BVHCOLOR_INIT_COMPONENTS)
	{
		// The data is the color components
		this->init((float*)data);
	}
	else if (mode == BVHCOLOR_INIT_TEMP)
	{
		// The data is the temperature in kelvin
#ifdef BVHCOLOR_RGB
		float *temp = (float*)data;
		float waveLength;
		float aux;

		this->c[0] = 0.0f;
		this->c[1] = 0.0f;
		this->c[2] = 0.0f;

		for (int i = 0; i < BVHCOLOR_COMPONENTS_AUX; i++)
		{
			waveLength = 380.0e-9 + BVHCOLOR_STEPL_AUX * i;
			aux =
				(3.741771182e-16 * powf(waveLength, -5.0f)) /
				(expf(1.4388e-2 / (waveLength * *temp)) - 1.0f);
			this->c[0] += aux * BVHColor::rgbColorMatchAux[i][0];
			this->c[1] += aux * BVHColor::rgbColorMatchAux[i][1];
			this->c[2] += aux * BVHColor::rgbColorMatchAux[i][2];
		}

		this->c[0] *= BVHCOLOR_SCALE_AUX;
		this->c[1] *= BVHCOLOR_SCALE_AUX;
		this->c[2] *= BVHCOLOR_SCALE_AUX;
#elif defined(BVHCOLOR_SPECTRAL_VISIBLE_10) || defined(BVHCOLOR_SPECTRAL_VISIBLE_5)
		float *temp = (float*)data;
		float waveLength;

		for (int i = 0; i < BVHCOLOR_COMPONENTS; i++)
		{
			waveLength = 380.0e-9 + BVHCOLOR_STEPL * i;

			this->c[i] =
				(3.741771182e-16 * powf(waveLength, -5.0f)) /
				(expf(1.4388e-2 / (waveLength * *temp)) - 1.0f);
		}
#endif
	}
	else if (mode == BVHCOLOR_INIT_SPECTRUM)
	{
#ifdef BVHCOLOR_RGB
		float waveLength;
		float aux;

		this->c[0] = 0.0f;
		this->c[1] = 0.0f;
		this->c[2] = 0.0f;

		for (int i = 0; i < BVHCOLOR_COMPONENTS_AUX; i++)
		{
			waveLength = 380.0e-9 + BVHCOLOR_STEPL_AUX * i;
			aux = BVHColor::getValue(waveLength, (BVHColor_spectrum*)data);
			this->c[0] += aux * BVHColor::rgbColorMatchAux[i][0];
			this->c[1] += aux * BVHColor::rgbColorMatchAux[i][1];
			this->c[2] += aux * BVHColor::rgbColorMatchAux[i][2];
		}

		this->c[0] *= BVHCOLOR_SCALE_AUX;
		this->c[1] *= BVHCOLOR_SCALE_AUX;
		this->c[2] *= BVHCOLOR_SCALE_AUX;
#elif defined(BVHCOLOR_SPECTRAL_VISIBLE_10) || defined(BVHCOLOR_SPECTRAL_VISIBLE_5)
		float waveLength;

		for (int i = 0; i < BVHCOLOR_COMPONENTS; i++)
		{
			waveLength = 380.0e-9 + BVHCOLOR_STEPL * i;
			this->c[i] = BVHColor::getValue(waveLength, (BVHColor_spectrum*)data);
		}
#endif
	}
}

void BVHColor::init(float r, float g, float b)
{
	gammaUncorrect(r);
	gammaUncorrect(g);
	gammaUncorrect(b);

#ifdef BVHCOLOR_RGB
	this->c[0] = r;
	this->c[1] = g;
	this->c[2] = b;
#elif defined(BVHCOLOR_SPECTRAL_VISIBLE_10) || defined(BVHCOLOR_SPECTRAL_VISIBLE_5)
	float rgbInv[3];

	rgbInv[0] =
		r * BVHColor::colorRamp.cMatrix[0] +
		g * BVHColor::colorRamp.cMatrix[3] +
		b * BVHColor::colorRamp.cMatrix[6];
	rgbInv[1] =
		r * BVHColor::colorRamp.cMatrix[1] +
		g * BVHColor::colorRamp.cMatrix[4] +
		b * BVHColor::colorRamp.cMatrix[7];
	rgbInv[2] =
		r * BVHColor::colorRamp.cMatrix[2] +
		g * BVHColor::colorRamp.cMatrix[5] +
		b * BVHColor::colorRamp.cMatrix[8];

	for (int i = 0; i < BVHCOLOR_COMPONENTS; i++)
	{
		this->c[i] =
			rgbInv[0] * BVHColor::colorRampRGB[i][0] +
			rgbInv[1] * BVHColor::colorRampRGB[i][1] +
			rgbInv[2] * BVHColor::colorRampRGB[i][2];
	}
#endif
}

void BVHColor::init(float intensity)
{
#ifdef BVHCOLOR_RGB
	this->c[0] = intensity;
	this->c[1] = intensity;
	this->c[2] = intensity;
#elif defined(BVHCOLOR_SPECTRAL_VISIBLE_10) || defined(BVHCOLOR_SPECTRAL_VISIBLE_5)
	for (int i = 0; i < BVHCOLOR_COMPONENTS; i++)
		this->c[i] = intensity;
#endif
}

void BVHColor::init(float *components)
{
	memcpy(this->c, components, sizeof(float) * BVHCOLOR_COMPONENTS);
}


	/* Set color system */

void BVHColor::setColorSystem(const char *name)
{
	for (int i = 0; i < BVHCOLOR_COLORSYSTEM_NUMBER; i++)
	{
		if (strcmp(name, BVHColor::colorSystems[i].name) == 0)
		{
			BVHColor::colorSystemCurrent = BVHColor::colorSystems[i];
			BVHColor::updateColorTables();
			BVHColor::updateColorRamps();
			return;
		}
	}
}

void BVHColor::setColorSystem(
	float xRed, float yRed, float zRed,
	float xGreen, float yGreen, float zGreen,
	float xBlue, float yBlue, float zBlue,
	float xWhite, float yWhite, float zWhite,
	float gamma)
{
	BVHColor::colorSystemCurrent.name = BVHCOLOR_COLORSYSTEM_CUSTOM;

	BVHColor::colorSystemCurrent.rgb2xyz[0] = xRed;
	BVHColor::colorSystemCurrent.rgb2xyz[1] = yRed;
	BVHColor::colorSystemCurrent.rgb2xyz[2] = zRed;
	BVHColor::colorSystemCurrent.rgb2xyz[3] = xGreen;
	BVHColor::colorSystemCurrent.rgb2xyz[4] = yGreen;
	BVHColor::colorSystemCurrent.rgb2xyz[5] = zGreen;
	BVHColor::colorSystemCurrent.rgb2xyz[6] = xBlue;
	BVHColor::colorSystemCurrent.rgb2xyz[7] = yBlue;
	BVHColor::colorSystemCurrent.rgb2xyz[8] = zBlue;

	BVHColor::colorSystemCurrent.xyz2rgb[0] = yGreen * zBlue  - yBlue  * zGreen;
	BVHColor::colorSystemCurrent.xyz2rgb[1] = zRed   * yBlue  - zBlue  * yRed;
	BVHColor::colorSystemCurrent.xyz2rgb[2] = yRed   * zGreen - yGreen * zRed;
	BVHColor::colorSystemCurrent.xyz2rgb[3] = zGreen * xBlue  - zBlue  * xGreen;
	BVHColor::colorSystemCurrent.xyz2rgb[4] = xRed   * zBlue  - xBlue  * zRed;
	BVHColor::colorSystemCurrent.xyz2rgb[5] = zRed   * xGreen - zGreen * xRed;
	BVHColor::colorSystemCurrent.xyz2rgb[6] = xGreen * yBlue  - xBlue  * yGreen;
	BVHColor::colorSystemCurrent.xyz2rgb[7] = yRed   * xBlue  - yBlue  * xRed;
	BVHColor::colorSystemCurrent.xyz2rgb[8] = xRed   * yGreen - xGreen * yRed;

	BVHColor::colorSystemCurrent.whitexyz[0] = xWhite;
	BVHColor::colorSystemCurrent.whitexyz[1] = yWhite;
	BVHColor::colorSystemCurrent.whitexyz[2] = zWhite;

	BVHColor::colorSystemCurrent.whitergb[0] =
		BVHColor::colorSystemCurrent.xyz2rgb[0] * xWhite +
		BVHColor::colorSystemCurrent.xyz2rgb[3] * yWhite +
		BVHColor::colorSystemCurrent.xyz2rgb[6] * zWhite;
	BVHColor::colorSystemCurrent.whitergb[1] =
		BVHColor::colorSystemCurrent.xyz2rgb[1] * xWhite +
		BVHColor::colorSystemCurrent.xyz2rgb[4] * yWhite +
		BVHColor::colorSystemCurrent.xyz2rgb[7] * zWhite;
	BVHColor::colorSystemCurrent.whitergb[2] =
		BVHColor::colorSystemCurrent.xyz2rgb[2] * xWhite +
		BVHColor::colorSystemCurrent.xyz2rgb[5] * yWhite +
		BVHColor::colorSystemCurrent.xyz2rgb[8] * zWhite;

	BVHColor::colorSystemCurrent.xyz2rgbCorrected[0] =
		BVHColor::colorSystemCurrent.xyz2rgb[0] / BVHColor::colorSystemCurrent.whitergb[0];
	BVHColor::colorSystemCurrent.xyz2rgbCorrected[1] =
		BVHColor::colorSystemCurrent.xyz2rgb[1] / BVHColor::colorSystemCurrent.whitergb[1];
	BVHColor::colorSystemCurrent.xyz2rgbCorrected[2] =
		BVHColor::colorSystemCurrent.xyz2rgb[2] / BVHColor::colorSystemCurrent.whitergb[2];
	BVHColor::colorSystemCurrent.xyz2rgbCorrected[3] =
		BVHColor::colorSystemCurrent.xyz2rgb[3] / BVHColor::colorSystemCurrent.whitergb[0];
	BVHColor::colorSystemCurrent.xyz2rgbCorrected[4] =
		BVHColor::colorSystemCurrent.xyz2rgb[4] / BVHColor::colorSystemCurrent.whitergb[1];
	BVHColor::colorSystemCurrent.xyz2rgbCorrected[5] =
		BVHColor::colorSystemCurrent.xyz2rgb[5] / BVHColor::colorSystemCurrent.whitergb[2];
	BVHColor::colorSystemCurrent.xyz2rgbCorrected[6] =
		BVHColor::colorSystemCurrent.xyz2rgb[6] / BVHColor::colorSystemCurrent.whitergb[0];
	BVHColor::colorSystemCurrent.xyz2rgbCorrected[7] =
		BVHColor::colorSystemCurrent.xyz2rgb[7] / BVHColor::colorSystemCurrent.whitergb[1];
	BVHColor::colorSystemCurrent.xyz2rgbCorrected[8] =
		BVHColor::colorSystemCurrent.xyz2rgb[8] / BVHColor::colorSystemCurrent.whitergb[2];

	BVHColor::colorSystemCurrent.gamma = gamma;

	BVHColor::updateColorTables();
	BVHColor::updateColorRamps();
}


	/* Conversion to RGB */

void BVHColor::toRGB(float *rgb)
{
#ifdef BVHCOLOR_RGB
	rgb[0] = this->c[0];
	rgb[1] = this->c[1];
	rgb[2] = this->c[2];
#elif defined(BVHCOLOR_SPECTRAL_VISIBLE_10) || defined(BVHCOLOR_SPECTRAL_VISIBLE_5)
	rgb[0] = 0.0f;
	rgb[1] = 0.0f;
	rgb[2] = 0.0f;

	for (int i = 0; i < BVHCOLOR_COMPONENTS; i++)
	{
		rgb[0] += this->c[i] * BVHColor::rgbColorMatch[i][0];
		rgb[1] += this->c[i] * BVHColor::rgbColorMatch[i][1];
		rgb[2] += this->c[i] * BVHColor::rgbColorMatch[i][2];
	}

	rgb[0] *= BVHCOLOR_SCALE;
	rgb[1] *= BVHCOLOR_SCALE;
	rgb[2] *= BVHCOLOR_SCALE;
#endif
}


	/* Get intensity */

float BVHColor::getIntensity()
{
	float intensity = 0.0f;

#ifdef BVHCOLOR_RGB
	intensity = (this->c[0] + this->c[1] + this->c[2]) / 3.0f;
#elif defined(BVHCOLOR_SPECTRAL_VISIBLE_10)  || defined(BVHCOLOR_SPECTRAL_VISIBLE_5)
	for (int i = 0; i < BVHCOLOR_COMPONENTS; i++)
		intensity += this->c[i];
	intensity /= BVHCOLOR_COMPONENTS;
#endif

	return intensity;
}


	/* Print values */

void BVHColor::print()
{
#ifdef BVHCOLOR_RGB
	cout << this->c[0] << ", " << this->c[1] << ", " << this->c[2] << endl;
#elif defined(BVHCOLOR_SPECTRAL_VISIBLE_10) || defined(BVHCOLOR_SPECTRAL_VISIBLE_5)
	cout << "L (nm)\tIntensity" << endl;
	cout << "------\t---------" << endl;

	for (int i = 0; i < BVHCOLOR_COMPONENTS; i++)
		cout << (380.0 + 1.0e9 * BVHCOLOR_STEPL * i) << "\t" << this->c[i] << endl;
#endif
}

	/* Operators */

BVHColor BVHColor::operator +(const BVHColor &c)
{
	BVHColor cr;

	for (int i = 0; i < BVHCOLOR_COMPONENTS; i++)
		cr.c[i] = this->c[i] + c.c[i];

	return cr;
}

BVHColor BVHColor::operator +=(const BVHColor &c)
{
	for (int i = 0; i < BVHCOLOR_COMPONENTS; i++)
		this->c[i] += c.c[i];

	return *this;
}

BVHColor BVHColor::operator -()
{
	BVHColor cr;

	for (int i = 0; i < BVHCOLOR_COMPONENTS; i++)
		cr.c[i] = -this->c[i];

	return cr;
}

BVHColor BVHColor::operator -(const BVHColor &c)
{
	BVHColor cr;

	for (int i = 0; i < BVHCOLOR_COMPONENTS; i++)
		cr.c[i] = this->c[i] - c.c[i];

	return cr;
}

BVHColor BVHColor::operator -=(const BVHColor &c)
{
	for (int i = 0; i < BVHCOLOR_COMPONENTS; i++)
		this->c[i] -= c.c[i];

	return *this;
}

BVHColor BVHColor::operator *(const BVHColor &c)
{
	BVHColor cr;

	for (int i = 0; i < BVHCOLOR_COMPONENTS; i++)
		cr.c[i] = this->c[i] * c.c[i];

	return cr;
}

BVHColor BVHColor::operator *=(const BVHColor &c)
{
	for (int i = 0; i < BVHCOLOR_COMPONENTS; i++)
		this->c[i] *= c.c[i];

	return *this;
}

BVHColor BVHColor::operator *(const float &c)
{
	BVHColor cr;

	for (int i = 0; i < BVHCOLOR_COMPONENTS; i++)
		cr.c[i] = this->c[i] * c;

	return cr;
}

BVHColor BVHColor::operator *=(const float &c)
{
	for (int i = 0; i < BVHCOLOR_COMPONENTS; i++)
		this->c[i] *= c;

	return *this;
}

BVHColor BVHColor::operator /(const BVHColor &c)
{
	BVHColor cr;

	for (int i = 0; i < BVHCOLOR_COMPONENTS; i++)
		cr.c[i] = this->c[i] / c.c[i];

	return cr;
}

BVHColor BVHColor::operator /=(const BVHColor &c)
{
	for (int i = 0; i < BVHCOLOR_COMPONENTS; i++)
		this->c[i] /= c.c[i];

	return *this;
}

BVHColor BVHColor::operator /(const float &c)
{
	BVHColor cr;

	for (int i = 0; i < BVHCOLOR_COMPONENTS; i++)
		cr.c[i] = this->c[i] / c;

	return cr;
}

BVHColor BVHColor::operator /=(const float &c)
{
	for (int i = 0; i < BVHCOLOR_COMPONENTS; i++)
		this->c[i] /= c;

	return *this;
}


	/* Update color tables */

void BVHColor::updateColorTables()
{
	for (int i = 0; i < BVHCOLOR_COMPONENTS; i++)
	{
		BVHColor::rgbColorMatch[i][0] =
			BVHColor::cieColorMatch[i][0] * BVHColor::colorSystemCurrent.xyz2rgbCorrected[0] +
			BVHColor::cieColorMatch[i][1] * BVHColor::colorSystemCurrent.xyz2rgbCorrected[3] +
			BVHColor::cieColorMatch[i][2] * BVHColor::colorSystemCurrent.xyz2rgbCorrected[6];
		BVHColor::rgbColorMatch[i][1] =
			BVHColor::cieColorMatch[i][0] * BVHColor::colorSystemCurrent.xyz2rgbCorrected[1] +
			BVHColor::cieColorMatch[i][1] * BVHColor::colorSystemCurrent.xyz2rgbCorrected[4] +
			BVHColor::cieColorMatch[i][2] * BVHColor::colorSystemCurrent.xyz2rgbCorrected[7];
		BVHColor::rgbColorMatch[i][2] =
			BVHColor::cieColorMatch[i][0] * BVHColor::colorSystemCurrent.xyz2rgbCorrected[2] +
			BVHColor::cieColorMatch[i][1] * BVHColor::colorSystemCurrent.xyz2rgbCorrected[5] +
			BVHColor::cieColorMatch[i][2] * BVHColor::colorSystemCurrent.xyz2rgbCorrected[8];
	}

#ifdef BVHCOLOR_RGB
	for (int i = 0; i < BVHCOLOR_COMPONENTS_AUX; i++)
	{
		BVHColor::rgbColorMatchAux[i][0] =
			BVHColor::cieColorMatchAux[i][0] * BVHColor::colorSystemCurrent.xyz2rgbCorrected[0] +
			BVHColor::cieColorMatchAux[i][1] * BVHColor::colorSystemCurrent.xyz2rgbCorrected[3] +
			BVHColor::cieColorMatchAux[i][2] * BVHColor::colorSystemCurrent.xyz2rgbCorrected[6];
		BVHColor::rgbColorMatchAux[i][1] =
			BVHColor::cieColorMatchAux[i][0] * BVHColor::colorSystemCurrent.xyz2rgbCorrected[1] +
			BVHColor::cieColorMatchAux[i][1] * BVHColor::colorSystemCurrent.xyz2rgbCorrected[4] +
			BVHColor::cieColorMatchAux[i][2] * BVHColor::colorSystemCurrent.xyz2rgbCorrected[7];
		BVHColor::rgbColorMatchAux[i][2] =
			BVHColor::cieColorMatchAux[i][0] * BVHColor::colorSystemCurrent.xyz2rgbCorrected[2] +
			BVHColor::cieColorMatchAux[i][1] * BVHColor::colorSystemCurrent.xyz2rgbCorrected[5] +
			BVHColor::cieColorMatchAux[i][2] * BVHColor::colorSystemCurrent.xyz2rgbCorrected[8];
	}
#endif
}


	/* update color ramps */

void BVHColor::updateColorRamps()
{
#ifdef BVHCOLOR_RGB
	return;
#elif defined(BVHCOLOR_SPECTRAL_VISIBLE_10) || defined(BVHCOLOR_SPECTRAL_VISIBLE_5)

		/* Get maximum R, G and B */

	// Reset values
	BVHColor::colorRamp.rmax = 0.0f;
	BVHColor::colorRamp.gmax = 0.0f;
	BVHColor::colorRamp.bmax = 0.0f;

	// Get the maximums and positions
	for (int i = 0; i < BVHCOLOR_COMPONENTS; i++)
	{
		if (BVHColor::rgbColorMatch[i][0] > BVHColor::colorRamp.rmax)
		{
			BVHColor::colorRamp.rmax = BVHColor::rgbColorMatch[i][0];
			BVHColor::colorRamp.lrmax = 380.0e-9 + i * BVHCOLOR_STEPL;
		}

		if (BVHColor::rgbColorMatch[i][1] > BVHColor::colorRamp.gmax)
		{
			BVHColor::colorRamp.gmax = BVHColor::rgbColorMatch[i][1];
			BVHColor::colorRamp.lgmax = 380.0e-9 + i * BVHCOLOR_STEPL;
		}

		if (BVHColor::rgbColorMatch[i][2] > BVHColor::colorRamp.bmax)
		{
			BVHColor::colorRamp.bmax = BVHColor::rgbColorMatch[i][2];
			BVHColor::colorRamp.lbmax = 380.0e-9 + i * BVHCOLOR_STEPL;
		}
	}


		/* Calcualte contributions */

	float cMatrix[9];
	float aux;

	// RED
	cMatrix[0] = 0.0f;
	cMatrix[1] = 0.0f;
	cMatrix[2] = 0.0f;

	for (int i = 0; i < BVHCOLOR_COMPONENTS; i++)
	{
		aux = BVHColor::getColorRampR(380.0e-9 + i * BVHCOLOR_STEPL);
		cMatrix[0] += aux * BVHColor::rgbColorMatch[i][0];
		cMatrix[1] += aux * BVHColor::rgbColorMatch[i][1];
		cMatrix[2] += aux * BVHColor::rgbColorMatch[i][2];
	}

	// GREEN
	cMatrix[3] = 0.0f;
	cMatrix[4] = 0.0f;
	cMatrix[5] = 0.0f;

	for (int i = 0; i < BVHCOLOR_COMPONENTS; i++)
	{
		aux = BVHColor::getColorRampG(380.0e-9 + i * BVHCOLOR_STEPL);
		cMatrix[3] += aux * BVHColor::rgbColorMatch[i][0];
		cMatrix[4] += aux * BVHColor::rgbColorMatch[i][1];
		cMatrix[5] += aux * BVHColor::rgbColorMatch[i][2];
	}

	// BLUE
	cMatrix[6] = 0.0f;
	cMatrix[7] = 0.0f;
	cMatrix[8] = 0.0f;

	for (int i = 0; i < BVHCOLOR_COMPONENTS; i++)
	{
		aux = BVHColor::getColorRampB(380.0e-9 + i * BVHCOLOR_STEPL);
		cMatrix[6] += aux * BVHColor::rgbColorMatch[i][0];
		cMatrix[7] += aux * BVHColor::rgbColorMatch[i][1];
		cMatrix[8] += aux * BVHColor::rgbColorMatch[i][2];
	}


		/* Invert the contribution matrix */

	aux = 1.0f /
		((cMatrix[0] * cMatrix[4] * cMatrix[8] +
		 cMatrix[1] * cMatrix[5] * cMatrix[6] +
		 cMatrix[2] * cMatrix[3] * cMatrix[7] -
		 cMatrix[0] * cMatrix[5] * cMatrix[7] -
		 cMatrix[1] * cMatrix[3] * cMatrix[8] -
		 cMatrix[2] * cMatrix[4] * cMatrix[6]) * BVHCOLOR_SCALE);

	BVHColor::colorRamp.cMatrix[0] =
		(cMatrix[4] * cMatrix[8] - cMatrix[5] * cMatrix[7]) * aux;
	BVHColor::colorRamp.cMatrix[1] =
		(cMatrix[2] * cMatrix[7] - cMatrix[1] * cMatrix[8]) * aux;
	BVHColor::colorRamp.cMatrix[2] =
		(cMatrix[1] * cMatrix[5] - cMatrix[2] * cMatrix[4]) * aux;
	BVHColor::colorRamp.cMatrix[3] =
		(cMatrix[5] * cMatrix[6] - cMatrix[3] * cMatrix[8]) * aux;
	BVHColor::colorRamp.cMatrix[4] =
		(cMatrix[0] * cMatrix[8] - cMatrix[2] * cMatrix[6]) * aux;
	BVHColor::colorRamp.cMatrix[5] =
		(cMatrix[2] * cMatrix[3] - cMatrix[0] * cMatrix[5]) * aux;
	BVHColor::colorRamp.cMatrix[6] =
		(cMatrix[3] * cMatrix[7] - cMatrix[4] * cMatrix[6]) * aux;
	BVHColor::colorRamp.cMatrix[7] =
		(cMatrix[1] * cMatrix[6] - cMatrix[0] * cMatrix[7]) * aux;
	BVHColor::colorRamp.cMatrix[8] =
		(cMatrix[0] * cMatrix[4] - cMatrix[1] * cMatrix[3]) * aux;


		/* Calculate color ramp tables */

	float l;

	for (int i = 0; i < BVHCOLOR_COMPONENTS; i++)
	{
		l = 380.0e-9 + BVHCOLOR_STEPL * i;

		BVHColor::colorRampRGB[i][0] = BVHColor::getColorRampR(l);
		BVHColor::colorRampRGB[i][1] = BVHColor::getColorRampG(l);
		BVHColor::colorRampRGB[i][2] = BVHColor::getColorRampB(l);
	}
#endif
}

float BVHColor::getColorRampR(float l)
{
	if (l >= 780.0e-9)
	{
		return  0.0f;
	}
	else if (l <= BVHColor::colorRamp.lgmax)
	{
		return	0.0f;
	}
	else if (l >= BVHColor::colorRamp.lrmax)
	{
		return	BVHColor::colorRamp.rmax *
				(780.0e-9 - l) /
				(780.0e-9 - BVHColor::colorRamp.lrmax);
	}
	else
	{
		return 	BVHColor::colorRamp.rmax *
				(l - BVHColor::colorRamp.lgmax) /
				(BVHColor::colorRamp.lrmax - BVHColor::colorRamp.lgmax);
	}
}

float BVHColor::getColorRampG(float l)
{
	if (l >= BVHColor::colorRamp.lrmax)
	{
		return	0.0f;
	}
	else if (l <= BVHColor::colorRamp.lbmax)
	{
		return	0.0f;
	}
	else if (l > BVHColor::colorRamp.lgmax)
	{
		return	BVHColor::colorRamp.gmax *
				(BVHColor::colorRamp.lrmax - l) /
				(BVHColor::colorRamp.lrmax - BVHColor::colorRamp.lgmax);
	}
	else
	{
		return	BVHColor::colorRamp.gmax *
				(l - BVHColor::colorRamp.lbmax) /
				(BVHColor::colorRamp.lgmax - BVHColor::colorRamp.lbmax);
	}
}

float BVHColor::getColorRampB(float l)
{
	if (l <= 380.0e-9)
	{
		return	0.0f;
	}
	else if (l >= BVHColor::colorRamp.lgmax)
	{
		return	0.0f;
	}
	else if (l <= BVHColor::colorRamp.lbmax)
	{
		return	BVHColor::colorRamp.bmax *
				(l - 380.0e-9) /
				(BVHColor::colorRamp.lbmax - 380.0e-9);
	}
	else
	{
		return	BVHColor::colorRamp.bmax *
				(BVHColor::colorRamp.lgmax - l) /
				(BVHColor::colorRamp.lgmax - BVHColor::colorRamp.lbmax);
	}
}


	/* Normalize rgb color */

float BVHColor::normalize(float *rgb)
{
	if (rgb[0] < 0.0f)
	{
		rgb[1] -= rgb[0];
		rgb[2] -= rgb[0];
		rgb[0] = 0.0f;
	}

	if (rgb[1] < 0.0f)
	{
		rgb[0] -= rgb[1];
		rgb[2] -= rgb[1];
		rgb[1] = 0.0f;
	}

	if (rgb[2] < 0.0f)
	{
		rgb[0] -= rgb[2];
		rgb[1] -= rgb[2];
		rgb[2] = 0.0f;
	}

	float max = rgb[0];

	if (rgb[1] > max)
		max = rgb[1];
	if (rgb[2] > max)
		max = rgb[2];

	rgb[0] /= max;
	rgb[1] /= max;
	rgb[2] /= max;

	return max;
}


	/* Get a value from a freq-intensity spectrum */

float BVHColor::getValue(float frequency, BVHColor_spectrum *spectrum)
{
	if (frequency < spectrum->components[0].freq)
		return 0.0f;
	else if (frequency > spectrum->components[spectrum->nComponents - 1].freq)
		return 0.0f;

	for (int i = 0; i < spectrum->nComponents - 1; i++)
	{
		if (frequency >= spectrum->components[i].freq &&
			frequency < spectrum->components[i + 1].freq)
		{
			return spectrum->components[i].value +
				(frequency - spectrum->components[i].freq) *
				(spectrum->components[i + 1].value - spectrum->components[i].value) /
				(spectrum->components[i + 1].freq - spectrum->components[i].freq);
		}
	}

	return 0.0f;
}


	/* Gamma correction */

void BVHColor::gammaCorrect(float &c)
{
	static const float cc = 0.018f;
	static const float k = (1.099f * powf(cc, 0.45f)) - 0.099f;

    if (BVHColor::colorSystemCurrent.gamma == BVHCOLOR_GAMMA_REC709)
    {
		if (c <= cc)
			c = c * k / cc;
		else
			c = (1.099f * powf(c, 0.45f)) - 0.099f;
	}
    else
    {
		c = powf(c, 1.0f / BVHColor::colorSystemCurrent.gamma);
    }
}

void BVHColor::gammaCorrectRGB(float *rgb)
{
    gammaCorrect(rgb[0]);
    gammaCorrect(rgb[1]);
    gammaCorrect(rgb[2]);
}

void BVHColor::gammaUncorrect(float &c)
{
	static const float cc = 0.018f;
	static const float k = (1.099f * powf(cc, 0.45f)) - 0.099f;
	static const float cte = powf(1.0f / 1.099f, 1.0f / 0.45f);

    if (BVHColor::colorSystemCurrent.gamma == BVHCOLOR_GAMMA_REC709)
    {
		if (c <= k)
			c = c * cc / k;
		else
			c = powf(c + 0.099f, 1.0f / 0.45f) * cte;
	}
    else
    {
		c = powf(c, BVHColor::colorSystemCurrent.gamma);
    }
}

void BVHColor::gammaUncorrectRGB(float *rgb)
{
	gammaUncorrect(rgb[0]);
    gammaUncorrect(rgb[1]);
    gammaUncorrect(rgb[2]);
}
