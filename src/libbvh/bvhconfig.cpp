/*
 * BVHConfig.cpp
 *
 *  Created on: 03 May 2011
 *      Author: showroom
 */


/*
 * INCLUDES
 */

#include "bvhconfig.h"


/*
 * NAMESPACE
 */

using namespace std;


/*
 * CLASS: BVHConfig
 */

BVHConfig::BVHConfig()
{
	// Load default values
	this->widthImage = BVHCONFIG_WIDTH_IMAGE;
	this->heightImage = BVHCONFIG_HEIGHT_IMAGE;
	this->widthBucket = BVHCONFIG_WIDTH_BUCKET;
	this->heightBucket = BVHCONFIG_HEIGHT_BUCKET;
	this->spp = BVHCONFIG_SPP;
	this->lightDepth = BVHCONFIG_LIGHT_DEPTH;
	this->eyeDepth = BVHCONFIG_EYE_DEPTH;
	this->blurRadius = BVHCONFIG_BLUR_RADIUS;

	this->nThreadsTree = BVHCONFIG_THREADS_TREE;

	this->renderUnits.clear();
	this->renderUnits.resize(1);
	this->renderUnits[0].mode = BVH_MODE_CPU;
	this->renderUnits[0].id = 0;
	this->renderUnits[0].deviceId = BVHCONFIG_DEVICEID;
	this->renderUnits[0].nThreadsGPU = BVHCONFIG_THREADS_GPU;
	this->renderUnits[0].nThreadsCPU = BVHCONFIG_THREADS_CPU;
}

BVHConfig::~BVHConfig()
{
	this->renderUnits.clear();
}

int BVHConfig::load(const char *filename)
{
	NMLParser parser;
	NMLParser_token token;
	
	if (!parser.open(filename))
		return 0;

	this->renderUnits.clear();

	while (1)
	{
		if (!parser.getWord(token))
		{
			if (token.type == NMLPARSER_EOF)
			{
				parser.close();
				return 1;
			}
			else
			{
				break;
			}
		}

		if (strcmp(token.value, "width-image") == 0)
		{
			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!parser.getWord(token))
				break;

			this->widthImage = atoi(token.value);

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;
		}
		else if (strcmp(token.value, "height-image") == 0)
		{
			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!parser.getWord(token))
				break;

			this->heightImage = atoi(token.value);

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;
		}
		else if (strcmp(token.value, "width-bucket") == 0)
		{
			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!parser.getWord(token))
				break;

			this->widthBucket = atoi(token.value);

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;
		}
		else if (strcmp(token.value, "height-bucket") == 0)
		{
			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!parser.getWord(token))
				break;

			this->heightBucket = atoi(token.value);

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;
		}
		else if (strcmp(token.value, "spp") == 0)
		{
			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!parser.getWord(token))
				break;

			this->spp = atoi(token.value);

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;
		}
		else if (strcmp(token.value, "light-depth") == 0)
		{
			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!parser.getWord(token))
				break;

			this->lightDepth = atoi(token.value);

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;
		}
		else if (strcmp(token.value, "eye-depth") == 0)
		{
			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!parser.getWord(token))
				break;

			this->eyeDepth = atoi(token.value);

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;
		}
		else if (strcmp(token.value, "blur-radius") == 0)
		{
			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!parser.getWord(token))
				break;

			this->blurRadius = atof(token.value);

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;
		}
		else if (strcmp(token.value, "threads-tree") == 0)
		{
			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!parser.getWord(token))
				break;

			this->nThreadsTree = atoi(token.value);

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;
		}
		else if (strcmp(token.value, "cpu") == 0)
		{
			BVHRenderUnit renderUnit;

			if (!this->loadRenderUnit(parser, BVH_MODE_CPU, renderUnit))
				break;

			this->renderUnits.push_back(renderUnit);
		}
		else if (strcmp(token.value, "gpu") == 0)
		{
			BVHRenderUnit renderUnit;

			if (!this->loadRenderUnit(parser, BVH_MODE_GPU, renderUnit))
				break;

			this->renderUnits.push_back(renderUnit);
		}
		else
		{
			break;
		}
	}
	
	NMLParser_error *lastError = parser.getLastError();

	if (lastError != NULL)
		cout << lastError->message << endl;

	parser.close();
	return 0;
}

int BVHConfig::loadRenderUnit(NMLParser &parser, int mode, BVHRenderUnit &renderUnit)
{
	NMLParser_token token;

	if (!parser.getSeparator(NMLPARSER_OPEN_BRACKET, token))
		return 0;

	// Set default values
	renderUnit.mode = mode;
	renderUnit.deviceId = BVHCONFIG_DEVICEID;
	renderUnit.nThreadsGPU = BVHCONFIG_THREADS_GPU;
	renderUnit.nThreadsCPU = BVHCONFIG_THREADS_CPU;

	while (1)
	{
		token = parser.getNextToken();

		if (token.type == NMLPARSER_ERROR)
			break;
		else if (token.type == NMLPARSER_EOF)
			break;
		else if (token.type == NMLPARSER_SEPARATOR &&
			token.value[0] != NMLPARSER_CLOSE_BRACKET)
			break;
		else if (token.type == NMLPARSER_SEPARATOR &&
			token.value[0] == NMLPARSER_CLOSE_BRACKET)
			return 1;

		if (mode == BVH_MODE_GPU && strcmp(token.value, "device-id") == 0)
		{
			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!parser.getWord(token))
				break;

			renderUnit.deviceId = atoi(token.value);

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;
		}
		else if (mode == BVH_MODE_GPU && strcmp(token.value, "threads-gpu") == 0)
		{
			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!parser.getWord(token))
				break;

			renderUnit.nThreadsGPU = atoi(token.value);

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;
		}
		else if (strcmp(token.value, "threads-cpu") == 0)
		{
			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!parser.getWord(token))
				break;

			renderUnit.nThreadsCPU = atoi(token.value);

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;
		}
		else
		{
			break;
		}
	}

	return 0;
}

int BVHConfig::save(const char *filename)
{
	// TODO
	return 0;
}

int BVHConfig::getWidthImage()
{
	return this->widthImage;
}

int BVHConfig::getHeightImage()
{
	return this->heightImage;
}

int BVHConfig::getWidthBucket()
{
	return this->widthBucket;
}

int BVHConfig::getHeightBucket()
{
	return this->heightBucket;
}

int BVHConfig::getSPP()
{
	return this->spp;
}

int BVHConfig::getLightDepth()
{
	return this->lightDepth;
}

int BVHConfig::getEyeDepth()
{
	return this->eyeDepth;
}

float BVHConfig::getBlurRadius()
{
	return this->blurRadius;
}

int BVHConfig::getNThreadsTree()
{
	return this->nThreadsTree;
}

unsigned int BVHConfig::getNRenderUnits()
{
	return this->renderUnits.size();
}

BVHRenderUnit *BVHConfig::getRenderUnit(unsigned int id)
{
	if (id >= this->renderUnits.size())
		return NULL;

	return &this->renderUnits[id];
}

std::vector<BVHRenderUnit> *BVHConfig::getRenderUnits()
{
	return &this->renderUnits;
}
