/*
 * bvhtexture.cpp
 *
 *  Created on: 7 Mar 2011
 *      Author: showroom
 */

/*
 * INCLUDES
 */

#include "bvhtexture.h"


/*
 * NAMESPACES
 */

using namespace std;


/*
 * CLASS: BVHTexture
 */

	/* Constructor and destructor */

BVHTexture::BVHTexture()
{
		/* Default values */

	// Buffer to store the data
	this->bufferSize = 0;
	this->buffer = NULL;

	// Texture dimensions
	this->width = 0;
	this->height = 0;
	this->depth = 0;

	// Channels, format
	this->channels = 3;
	this->format = BVHTEXTURE_FORMAT_RGB;
	this->dataFormat = BVHTEXTURE_INT8;

	// Wrap modes
	this->wraps = BVHTEXTURE_WRAP_CLAMP;
	this->wrapt = BVHTEXTURE_WRAP_CLAMP;
	this->wrapr = BVHTEXTURE_WRAP_CLAMP;
}

BVHTexture::~BVHTexture()
{
	if (this->bufferSize != 0)
		delete [] this->buffer;
}


	/* Init function */

void BVHTexture::init()
{
	ilInit();
}


	/* Load */

int BVHTexture::load(const char *filename)
{
		/* Init image */

	ILuint imgId = 0;
	ilGenImages(1, &imgId);
	ilBindImage(imgId);

	if (!ilLoadImage(filename))
	{
		ILenum error = ilGetError();

		if (error == IL_COULD_NOT_OPEN_FILE)
			cout << "The file pointed to by FileName could not be opened. Either the file does not exist or is in use by another process." << endl;
		else if (error == IL_ILLEGAL_OPERATION)
			cout << "There is currently no image bound. Use ilGenImages and ilBindImage before calling this function." << endl;
		else if (error == IL_INVALID_EXTENSION)
			cout << "The file could not be loaded based on extension or header." << endl;
		else if (error == IL_INVALID_PARAM)
			cout << "FileName was not valid. It was most likely NULL." << endl;
		else
			cout << "Unknown error." << endl;

		return 0;
	}


		/* Get dimensions */

	this->width = ilGetInteger(IL_IMAGE_WIDTH);
	this->height = ilGetInteger(IL_IMAGE_HEIGHT);
	this->depth = 0;
	this->channels = ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL);


		/* Destroy previous buffer */

	if (this->bufferSize != 0)
		delete [] this->buffer;


		/* Check dimensions */

	if (this->width <= 0 ||
		this->height <= 0)
	{
		ilBindImage(0);
		ilDeleteImage(imgId);
		return 0;
	}


		/* Setup image information and generate buffer */

	this->dataFormat = ilGetInteger(IL_IMAGE_TYPE);

	if (this->dataFormat == IL_UNSIGNED_BYTE)
	{
		this->dataFormat = BVHTEXTURE_INT8;
	}
	else if (this->dataFormat == IL_UNSIGNED_SHORT)
	{
		this->dataFormat = BVHTEXTURE_INT16;
	}
	else if (this->dataFormat == IL_FLOAT)
	{
		this->dataFormat = BVHTEXTURE_FLOAT32;
	}
	else
	{
		ilBindImage(0);
		ilDeleteImage(imgId);
		return 0;
	}

	this->format = ilGetInteger(IL_IMAGE_FORMAT);

	if (this->format == IL_RGB)
	{
		this->format = BVHTEXTURE_FORMAT_RGB;
	}
	else if (this->format == IL_RGBA)
	{
		this->format = BVHTEXTURE_FORMAT_RGBA;
	}
	else if (this->format == IL_LUMINANCE)
	{
		this->format = BVHTEXTURE_FORMAT_INTENSITY;
	}
	else
	{
		ilBindImage(0);
		ilDeleteImage(imgId);
		return 0;
	}

	this->bufferSize = this->width * this->height * this->channels;
	this->buffer = new unsigned char[this->bufferSize];
	memcpy(this->buffer, ilGetData(), this->bufferSize);


		/* Destroy image */

	ilBindImage(0);
	ilDeleteImage(imgId);


		/* Return ok */

	return 1;
}


	/* Set parameters */

void BVHTexture::setWraps(int wrap)
{
	if (wrap == BVHTEXTURE_WRAP_CLAMP ||
		wrap == BVHTEXTURE_WRAP_MIRRORED_REPEAT ||
		wrap == BVHTEXTURE_WRAP_REPEAT)
		this->wraps = wrap;
}

void BVHTexture::setWrapt(int wrap)
{
	if (wrap == BVHTEXTURE_WRAP_CLAMP ||
		wrap == BVHTEXTURE_WRAP_MIRRORED_REPEAT ||
		wrap == BVHTEXTURE_WRAP_REPEAT)
		this->wrapt = wrap;
}

void BVHTexture::setWrapr(int wrap)
{
	if (wrap == BVHTEXTURE_WRAP_CLAMP ||
		wrap == BVHTEXTURE_WRAP_MIRRORED_REPEAT ||
		wrap == BVHTEXTURE_WRAP_REPEAT)
		this->wrapr = wrap;
}


	/* Get info */

int BVHTexture::getWidth()
{
	return this->width;
}

int BVHTexture::getHeight()
{
	return this->height;
}

int BVHTexture::getDepth()
{
	return this->depth;
}

int BVHTexture::getChannels()
{
	return this->channels;
}

int BVHTexture::getFormat()
{
	return this->format;
}

int BVHTexture::getDataFormat()
{
	return this->dataFormat;
}

int BVHTexture::getWraps()
{
	return this->wraps;
}

int BVHTexture::getWrapt()
{
	return this->wrapt;
}

int BVHTexture::getWrapr()
{
	return this->wrapr;
}


	/* Get color */

BVHColor BVHTexture::getColor(float x, float y)
{
	BVHColor color;
	float rgb[20];
	int i[2];
	int j[2];
	float factori = 0.0f;
	float factorj = 0.0f;

	getInperp(this->wraps, this->width, x, i, factori);
	getInperp(this->wrapt, this->height, y, j, factorj);

	if (this->dataFormat == BVHTEXTURE_INT8)
	{
		for (int k = 0; k < this->channels; k++)
		{
			rgb[k] = (float)this->buffer[(i[0] + j[0] * this->width) * this->channels + k] / 255.0f;
			rgb[4 + k] = (float)this->buffer[(i[1] + j[0] * this->width) * this->channels + k] / 255.0f;
			rgb[8 + k] = (float)this->buffer[(i[0] + j[1] * this->width) * this->channels + k] / 255.0f;
			rgb[12 + k] = (float)this->buffer[(i[1] + j[1] * this->width) * this->channels + k] / 255.0f;
		}
	}
	else if (this->dataFormat == BVHTEXTURE_INT16)
	{
		for (int k = 0; k < this->channels; k++)
		{
			rgb[k] = (float)((unsigned short*)this->buffer)[(i[0] + j[0] * this->width) * this->channels + k] / 65535.0f;
			rgb[4 + k] = (float)((unsigned short*)this->buffer)[(i[1] + j[0] * this->width) * this->channels + k] / 65535.0f;
			rgb[8 + k] = (float)((unsigned short*)this->buffer)[(i[0] + j[1] * this->width) * this->channels + k] / 65535.0f;
			rgb[12 + k] = (float)((unsigned short*)this->buffer)[(i[1] + j[1] * this->width) * this->channels + k] / 65535.0f;
		}
	}
	else if (this->dataFormat == BVHTEXTURE_FLOAT32)
	{
		for (int k = 0; k < this->channels; k++)
		{
			rgb[k] = ((float*)this->buffer)[(i[0] + j[0] * this->width) * this->channels + k];
			rgb[4 + k] = ((float*)this->buffer)[(i[1] + j[0] * this->width) * this->channels + k];
			rgb[8 + k] = ((float*)this->buffer)[(i[0] + j[1] * this->width) * this->channels + k];
			rgb[12 + k] = ((float*)this->buffer)[(i[1] + j[1] * this->width) * this->channels + k];
		}
	}

	for (int k = 0; k < this->channels; k++)
	{
		rgb[16 + k] =
			rgb[k] * factori * factorj +
			rgb[4 + k] * (1.0f - factori) * factorj +
			rgb[8 + k] * factori * (1.0f - factorj) +
			rgb[12 + k] * (1.0f - factori) * (1.0f - factorj);
	}

	if (this->channels == 1)
		color.init(rgb[16]);
	else if (this->channels == 3 || this->channels == 4)
		color.init(rgb[16], rgb[17], rgb[18]);

	return color;
}

BVHColor BVHTexture::getColor(float x, float y, float z)
{
	if (this->depth <= 1)
		return this->getColor(x, y);

	BVHColor color;
	float rgb[36];
	int i[2];
	int j[2];
	int k[2];
	float factori = 0.0f;
	float factorj = 0.0f;
	float factork = 0.0f;

	getInperp(this->wraps, this->width, x, i, factori);
	getInperp(this->wrapt, this->height, y, j, factorj);
	getInperp(this->wrapr, this->depth, z, k, factork);

	if (this->dataFormat == BVHTEXTURE_INT8)
	{
		for (int l = 0; l < this->channels; l++)
		{
			rgb[     l] = (float)this->buffer[(i[0] + j[0] * this->width + k[0] * this->width * this->height) * this->channels + l] / 255.0f;
			rgb[4  + l] = (float)this->buffer[(i[1] + j[0] * this->width + k[0] * this->width * this->height) * this->channels + l] / 255.0f;
			rgb[8  + l] = (float)this->buffer[(i[0] + j[1] * this->width + k[0] * this->width * this->height) * this->channels + l] / 255.0f;
			rgb[12 + l] = (float)this->buffer[(i[1] + j[1] * this->width + k[0] * this->width * this->height) * this->channels + l] / 255.0f;
			rgb[16 + l] = (float)this->buffer[(i[0] + j[0] * this->width + k[1] * this->width * this->height) * this->channels + l] / 255.0f;
			rgb[20 + l] = (float)this->buffer[(i[1] + j[0] * this->width + k[1] * this->width * this->height) * this->channels + l] / 255.0f;
			rgb[24 + l] = (float)this->buffer[(i[0] + j[1] * this->width + k[1] * this->width * this->height) * this->channels + l] / 255.0f;
			rgb[28 + l] = (float)this->buffer[(i[1] + j[1] * this->width + k[1] * this->width * this->height) * this->channels + l] / 255.0f;
		}
	}
	else if (this->dataFormat == BVHTEXTURE_INT16)
	{
		for (int l = 0; l < this->channels; l++)
		{
			rgb[     l] = (float)((unsigned short*)this->buffer)[(i[0] + j[0] * this->width + k[0] * this->width * this->height) * this->channels + l] / 65535.0f;
			rgb[4  + l] = (float)((unsigned short*)this->buffer)[(i[1] + j[0] * this->width + k[0] * this->width * this->height) * this->channels + l] / 65535.0f;
			rgb[8  + l] = (float)((unsigned short*)this->buffer)[(i[0] + j[1] * this->width + k[0] * this->width * this->height) * this->channels + l] / 65535.0f;
			rgb[12 + l] = (float)((unsigned short*)this->buffer)[(i[1] + j[1] * this->width + k[0] * this->width * this->height) * this->channels + l] / 65535.0f;
			rgb[16 + l] = (float)((unsigned short*)this->buffer)[(i[0] + j[0] * this->width + k[1] * this->width * this->height) * this->channels + l] / 65535.0f;
			rgb[20 + l] = (float)((unsigned short*)this->buffer)[(i[1] + j[0] * this->width + k[1] * this->width * this->height) * this->channels + l] / 65535.0f;
			rgb[24 + l] = (float)((unsigned short*)this->buffer)[(i[0] + j[1] * this->width + k[1] * this->width * this->height) * this->channels + l] / 65535.0f;
			rgb[28 + l] = (float)((unsigned short*)this->buffer)[(i[1] + j[1] * this->width + k[1] * this->width * this->height) * this->channels + l] / 65535.0f;
		}
	}
	else if (this->dataFormat == BVHTEXTURE_FLOAT32)
	{
		for (int l = 0; l < this->channels; l++)
		{
			rgb[     l] = ((float*)this->buffer)[(i[0] + j[0] * this->width + k[0] * this->width * this->height) * this->channels + l];
			rgb[4  + l] = ((float*)this->buffer)[(i[1] + j[0] * this->width + k[0] * this->width * this->height) * this->channels + l];
			rgb[8  + l] = ((float*)this->buffer)[(i[0] + j[1] * this->width + k[0] * this->width * this->height) * this->channels + l];
			rgb[12 + l] = ((float*)this->buffer)[(i[1] + j[1] * this->width + k[0] * this->width * this->height) * this->channels + l];
			rgb[16 + l] = ((float*)this->buffer)[(i[0] + j[0] * this->width + k[1] * this->width * this->height) * this->channels + l];
			rgb[20 + l] = ((float*)this->buffer)[(i[1] + j[0] * this->width + k[1] * this->width * this->height) * this->channels + l];
			rgb[24 + l] = ((float*)this->buffer)[(i[0] + j[1] * this->width + k[1] * this->width * this->height) * this->channels + l];
			rgb[28 + l] = ((float*)this->buffer)[(i[1] + j[1] * this->width + k[1] * this->width * this->height) * this->channels + l];
		}
	}

	for (int l = 0; l < this->channels; l++)
	{
		rgb[32 + l] =
			rgb[     l] * factori          * factorj          * factork          +
			rgb[4  + l] * (1.0f - factori) * factorj          * factork          +
			rgb[8  + l] * factori          * (1.0f - factorj) * factork          +
			rgb[12 + l] * (1.0f - factori) * (1.0f - factorj) * factork          +
			rgb[16 + l] * factori          * factorj          * (1.0f - factork) +
			rgb[20 + l] * (1.0f - factori) * factorj          * (1.0f - factork) +
			rgb[24 + l] * factori          * (1.0f - factorj) * (1.0f - factork) +
			rgb[28 + l] * (1.0f - factori) * (1.0f - factorj) * (1.0f - factork);
	}

	if (this->channels == 1)
		color.init(rgb[32]);
	else if (this->channels == 3 || this->channels == 4)
		color.init(rgb[32], rgb[33], rgb[34]);

	return color;
}

BVHColor BVHTexture::getColor(float *texCoords)
{
	if (this->depth <= 1)
		return this->getColor(texCoords[0], texCoords[1]);
	else
		return this->getColor(texCoords[0], texCoords[1], texCoords[2]);
}


	/* Get intensity */

float BVHTexture::getIntensity(float x, float y)
{
	float rgb[20];
	int i[2];
	int j[2];
	float factori = 0.0f;
	float factorj = 0.0f;

	getInperp(this->wraps, this->width, x, i, factori);
	getInperp(this->wrapt, this->height, y, j, factorj);

	if (this->dataFormat == BVHTEXTURE_INT8)
	{
		for (int k = 0; k < this->channels; k++)
		{
			rgb[k] = (float)this->buffer[(i[0] + j[0] * this->width) * this->channels + k] / 255.0f;
			rgb[4 + k] = (float)this->buffer[(i[1] + j[0] * this->width) * this->channels + k] / 255.0f;
			rgb[8 + k] = (float)this->buffer[(i[0] + j[1] * this->width) * this->channels + k] / 255.0f;
			rgb[12 + k] = (float)this->buffer[(i[1] + j[1] * this->width) * this->channels + k] / 255.0f;
		}
	}
	else if (this->dataFormat == BVHTEXTURE_INT16)
	{
		for (int k = 0; k < this->channels; k++)
		{
			rgb[k] = (float)((unsigned short*)this->buffer)[(i[0] + j[0] * this->width) * this->channels + k] / 65535.0f;
			rgb[4 + k] = (float)((unsigned short*)this->buffer)[(i[1] + j[0] * this->width) * this->channels + k] / 65535.0f;
			rgb[8 + k] = (float)((unsigned short*)this->buffer)[(i[0] + j[1] * this->width) * this->channels + k] / 65535.0f;
			rgb[12 + k] = (float)((unsigned short*)this->buffer)[(i[1] + j[1] * this->width) * this->channels + k] / 65535.0f;
		}
	}
	else if (this->dataFormat == BVHTEXTURE_FLOAT32)
	{
		for (int k = 0; k < this->channels; k++)
		{
			rgb[k] = ((float*)this->buffer)[(i[0] + j[0] * this->width) * this->channels + k];
			rgb[4 + k] = ((float*)this->buffer)[(i[1] + j[0] * this->width) * this->channels + k];
			rgb[8 + k] = ((float*)this->buffer)[(i[0] + j[1] * this->width) * this->channels + k];
			rgb[12 + k] = ((float*)this->buffer)[(i[1] + j[1] * this->width) * this->channels + k];
		}
	}

	for (int k = 0; k < this->channels; k++)
	{
		rgb[16 + k] =
			rgb[k] * factori * factorj +
			rgb[4 + k] * (1.0f - factori) * factorj +
			rgb[8 + k] * factori * (1.0f - factorj) +
			rgb[12 + k] * (1.0f - factori) * (1.0f - factorj);
	}

	if (this->channels == 1)
		return rgb[16];
	else if (this->channels == 3 || this->channels == 4)
		return (rgb[16] + rgb[17] + rgb[18]) / 3.0f;
	else
		return 0.0f;
}

float BVHTexture::getIntensity(float x, float y, float z)
{
	if (this->depth <= 1)
		return this->getIntensity(x, y);

	float rgb[36];
	int i[2];
	int j[2];
	int k[2];
	float factori = 0.0f;
	float factorj = 0.0f;
	float factork = 0.0f;

	getInperp(this->wraps, this->width, x, i, factori);
	getInperp(this->wrapt, this->height, y, j, factorj);
	getInperp(this->wrapr, this->depth, z, k, factork);

	if (this->dataFormat == BVHTEXTURE_INT8)
	{
		for (int l = 0; l < this->channels; l++)
		{
			rgb[     l] = (float)this->buffer[(i[0] + j[0] * this->width + k[0] * this->width * this->height) * this->channels + l] / 255.0f;
			rgb[4  + l] = (float)this->buffer[(i[1] + j[0] * this->width + k[0] * this->width * this->height) * this->channels + l] / 255.0f;
			rgb[8  + l] = (float)this->buffer[(i[0] + j[1] * this->width + k[0] * this->width * this->height) * this->channels + l] / 255.0f;
			rgb[12 + l] = (float)this->buffer[(i[1] + j[1] * this->width + k[0] * this->width * this->height) * this->channels + l] / 255.0f;
			rgb[16 + l] = (float)this->buffer[(i[0] + j[0] * this->width + k[1] * this->width * this->height) * this->channels + l] / 255.0f;
			rgb[20 + l] = (float)this->buffer[(i[1] + j[0] * this->width + k[1] * this->width * this->height) * this->channels + l] / 255.0f;
			rgb[24 + l] = (float)this->buffer[(i[0] + j[1] * this->width + k[1] * this->width * this->height) * this->channels + l] / 255.0f;
			rgb[28 + l] = (float)this->buffer[(i[1] + j[1] * this->width + k[1] * this->width * this->height) * this->channels + l] / 255.0f;
		}
	}
	else if (this->dataFormat == BVHTEXTURE_INT16)
	{
		for (int l = 0; l < this->channels; l++)
		{
			rgb[     l] = (float)((unsigned short*)this->buffer)[(i[0] + j[0] * this->width + k[0] * this->width * this->height) * this->channels + l] / 65535.0f;
			rgb[4  + l] = (float)((unsigned short*)this->buffer)[(i[1] + j[0] * this->width + k[0] * this->width * this->height) * this->channels + l] / 65535.0f;
			rgb[8  + l] = (float)((unsigned short*)this->buffer)[(i[0] + j[1] * this->width + k[0] * this->width * this->height) * this->channels + l] / 65535.0f;
			rgb[12 + l] = (float)((unsigned short*)this->buffer)[(i[1] + j[1] * this->width + k[0] * this->width * this->height) * this->channels + l] / 65535.0f;
			rgb[16 + l] = (float)((unsigned short*)this->buffer)[(i[0] + j[0] * this->width + k[1] * this->width * this->height) * this->channels + l] / 65535.0f;
			rgb[20 + l] = (float)((unsigned short*)this->buffer)[(i[1] + j[0] * this->width + k[1] * this->width * this->height) * this->channels + l] / 65535.0f;
			rgb[24 + l] = (float)((unsigned short*)this->buffer)[(i[0] + j[1] * this->width + k[1] * this->width * this->height) * this->channels + l] / 65535.0f;
			rgb[28 + l] = (float)((unsigned short*)this->buffer)[(i[1] + j[1] * this->width + k[1] * this->width * this->height) * this->channels + l] / 65535.0f;
		}
	}
	else if (this->dataFormat == BVHTEXTURE_FLOAT32)
	{
		for (int l = 0; l < this->channels; l++)
		{
			rgb[     l] = ((float*)this->buffer)[(i[0] + j[0] * this->width + k[0] * this->width * this->height) * this->channels + l];
			rgb[4  + l] = ((float*)this->buffer)[(i[1] + j[0] * this->width + k[0] * this->width * this->height) * this->channels + l];
			rgb[8  + l] = ((float*)this->buffer)[(i[0] + j[1] * this->width + k[0] * this->width * this->height) * this->channels + l];
			rgb[12 + l] = ((float*)this->buffer)[(i[1] + j[1] * this->width + k[0] * this->width * this->height) * this->channels + l];
			rgb[16 + l] = ((float*)this->buffer)[(i[0] + j[0] * this->width + k[1] * this->width * this->height) * this->channels + l];
			rgb[20 + l] = ((float*)this->buffer)[(i[1] + j[0] * this->width + k[1] * this->width * this->height) * this->channels + l];
			rgb[24 + l] = ((float*)this->buffer)[(i[0] + j[1] * this->width + k[1] * this->width * this->height) * this->channels + l];
			rgb[28 + l] = ((float*)this->buffer)[(i[1] + j[1] * this->width + k[1] * this->width * this->height) * this->channels + l];
		}
	}

	for (int l = 0; l < this->channels; l++)
	{
		rgb[32 + l] =
			rgb[     l] * factori          * factorj          * factork          +
			rgb[4  + l] * (1.0f - factori) * factorj          * factork          +
			rgb[8  + l] * factori          * (1.0f - factorj) * factork          +
			rgb[12 + l] * (1.0f - factori) * (1.0f - factorj) * factork          +
			rgb[16 + l] * factori          * factorj          * (1.0f - factork) +
			rgb[20 + l] * (1.0f - factori) * factorj          * (1.0f - factork) +
			rgb[24 + l] * factori          * (1.0f - factorj) * (1.0f - factork) +
			rgb[28 + l] * (1.0f - factori) * (1.0f - factorj) * (1.0f - factork);
	}

	if (this->channels == 1)
		return rgb[32];
	else if (this->channels == 3 || this->channels == 4)
		return (rgb[32] + rgb[33] + rgb[34]) / 3.0f;
	else
		return 0.0f;
}

float BVHTexture::getIntensity(float *texCoords)
{
	if (this->depth <= 1)
		return this->getIntensity(texCoords[0], texCoords[1]);
	else
		return this->getIntensity(texCoords[0], texCoords[1], texCoords[2]);
}


	/* Static functions */

BVHColor BVHTexture::getColorUV(const void *texture, float x, float y)
{
	BVHColor color;

	if (texture != NULL)
		color = ((BVHTexture*)texture)->getColor(x, y);

	return color;
}

BVHColor BVHTexture::getColorUVW(const void *texture, float x, float y, float z)
{
	BVHColor color;

	if (texture != NULL)
		color = ((BVHTexture*)texture)->getColor(x, y, z);

	return color;
}

BVHColor BVHTexture::getColor(const void *texture, float *texCoords)
{
	BVHColor color;

	if (texture != NULL)
		color = ((BVHTexture*)texture)->getColor(texCoords);

	return color;
}

float BVHTexture::getIntensityUV(const void *texture, float x, float y)
{
	float intensity = 0.0f;

	if (texture != NULL)
		intensity = ((BVHTexture*)texture)->getIntensity(x, y);

	return intensity;
}

float BVHTexture::getIntensityUVW(const void *texture, float x, float y, float z)
{
	float intensity = 0.0f;

	if (texture != NULL)
		intensity = ((BVHTexture*)texture)->getIntensity(x, y, z);

	return intensity;
}

float BVHTexture::getIntensity(const void *texture, float *texCoords)
{
	float intensity = 0.0f;

	if (texture != NULL)
		intensity = ((BVHTexture*)texture)->getIntensity(texCoords);

	return intensity;
}


	/* Aux functions */

void BVHTexture::getInperp(int type, int w, float p, int *pint, float &factor)
{
	p = w * p - 0.5f;

	if (type == BVHTEXTURE_WRAP_CLAMP)
	{
		if (p <= 0.0f)
		{
			pint[0] = 0;
			pint[1] = 0;
			factor = 0.0f;
		}
		else if (p >= w - 1)
		{
			pint[0] = w - 1;
			pint[1] = w - 1;
			factor = 0.0f;
		}
		else
		{
			pint[0] = (int)p;
			pint[1] = pint[0] + 1;
			factor = 1.0f - p + pint[0];
		}
	}
	else if (type == BVHTEXTURE_WRAP_REPEAT)
	{
		if (p < 0.0f)
		{
			pint[1] = (int)p;
			pint[0] = pint[1] - 1;
			factor = 1.0f - p + pint[0];
		}
		else
		{
			pint[0] = (int)p;
			pint[1] = pint[0] + 1;
			factor = 1.0f - p + pint[0];
		}

		pint[0] = pint[0] % w;
		pint[1] = pint[1] % w;

		if (pint[0] < 0) pint[0] = w + pint[0];
		if (pint[1] < 0) pint[1] = w + pint[1];
	}
	else if (type == BVHTEXTURE_WRAP_MIRRORED_REPEAT)
	{
		if (p <= 0.0f)
		{
			pint[1] = (int)p;
			pint[0] = pint[1] - 1;
			factor = 1.0f - p + pint[0];
		}
		else
		{
			pint[0] = (int)p;
			pint[1] = pint[0] + 1;
			factor = 1.0f - p + pint[0];
		}

		if (pint[0] < 0)
			pint[0] = -pint[0] - 1;

		if (pint[1] < 0)
			pint[1] = -pint[1] - 1;

		if (((pint[0] / w) % 2) == 1)
			pint[0] = w - 1 - (pint[0] % w);
		else
			pint[0] = pint[0] % w;

		if (((pint[1] / w) % 2) == 1)
			pint[1] = w - 1 - (pint[1] % w);
		else
			pint[1] = pint[1] % w;
	}
	else
	{
		pint[0] = 0;
		pint[1] = 0;
		factor = 0.0f;
	}
}
