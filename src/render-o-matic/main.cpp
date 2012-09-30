/*
 * main.cpp
 *
 *	Render-O-Matic
 *
 *  Created on: 19 Jan 2011
 *      Author: Jesus Ortiz Sanchez-Lafuente
 *      Email : jortizsl@gmail.com
 */


/*
 * INCLUDES
 */

	/* General */
#include <iostream>
#include <fstream>
#include <signal.h>
#include <math.h>

	/* SDL */
#include <SDL/SDL.h>

	/* Utils */
#include "image.h"
#include "params.h"
#include "utils.h"

	/* BVH */
#include "bvh.h"
#include "bvhcamera.h"
#include "bvhcolor.h"
#include "bvhconfig.h"
#include "bvhcuda.h"
#include "bvhlight.h"
#include "bvhscene.h"
#include "bvhshader.h"
#include "bvhtexture.h"


/*
 * NAMESPACES
 */

using namespace std;


/*
 * DEFINES
 */

#define MAX_RENDER_UNITS	256


/*
 * AUX FUNCTIONS
 */

unsigned char clamp(float value)
{
	if (value <= 0.0f)
		return 0;
	else if (value >= 1.0f)
		return 255;
	else
		return (unsigned char)(255.0f * value);
}


/*
 * PRINT USAGE
 */

void printUsage(ParamList &params)
{
#ifdef linux
	cout << endl;
	cout << "\033[1mNAME\033[0m" << endl;
	cout << "       Render-O-Matic" << endl;
	cout << endl;
	cout << "\033[1mSYNOPSIS\033[0m" << endl;
	cout << "       \033[1mrender-o-matic\033[0m [\033[4mOPTION\033[0m]..." << endl;
	cout << endl;
	cout << "\033[1mDESCRIPTION\033[0m" << endl;
	cout << "       Render-O-Matic" << endl;
	cout << endl;
	cout << "       For more information refer to the authors" << endl;
	cout << endl;
	params.printUsage();
	cout << endl;
	cout << "\033[1mAUTHOR\033[0m" << endl;
	cout << "       Written by Jesus Ortiz Sanchez-Lafuente" << endl;
	cout << endl;
	cout << "\033[1mREPORTING BUGS\033[0m" << endl;
	cout << "       Report bugs to <jortizsl@gmail.com>" << endl;
	cout << endl;
	cout << "\033[1mCOPYRIGHT\033[0m" << endl;
	cout << "       This product is not for public distribution" << endl;
	cout << endl;
#else
	cout << endl;
	cout << "NAME" << endl;
	cout << "       Render-O-Matic" << endl;
	cout << endl;
	cout << "SYNOPSIS" << endl;
	cout << "       render-o-matic [OPTION]..." << endl;
	cout << endl;
	cout << "DESCRIPTION" << endl;
	cout << "       Render-O-Matic" << endl;
	cout << endl;
	cout << "       For more information refer to the authors" << endl;
	cout << endl;
	params.printUsage();
	cout << endl;
	cout << "AUTHOR" << endl;
	cout << "       Written by Jesus Ortiz Sanchez-Lafuente" << endl;
	cout << endl;
	cout << "REPORTING BUGS" << endl;
	cout << "       Report bugs to <jortizsl@gmail.com>" << endl;
	cout << endl;
	cout << "COPYRIGHT" << endl;
	cout << "       This product is not for public distribution" << endl;
	cout << endl;
#endif
}


/*
 * MAIN
 */

int main(int argc, char *argv[])
{
		/* Init things */

	BVH::init("HDTV");


		/* Variables */

	// BVH
	BVH bvh;

	// Time
	struct timespec time0, time1;
	double timeElapsed;

	// Buffers
	BVH_gpuRenderBuffer *renderBuffer;
	BVH_gpuRenderBuffer *renderBufferAcc;
	unsigned char *imageBuffer;

	// Render units
	int nRenderUnits;
	BVHRenderUnit renderUnits[MAX_RENDER_UNITS];
	char renderUnitString[PARAM_VALUE_LENGHT];
	int renderUnitStringPos;

	// SDL
	SDL_Surface *sdlScreen = NULL;
	int sdlWidth;
	int sdlHeight;
	SDL_Event sdlEvent;
	int sdlSample;


		/* Params */

	// Params variables
	char param_help;
	char param_info;
	char param_renderUnits[PARAM_VALUE_LENGHT];
	int param_renderUnits_length;
	int param_nThreadsGPU;
	int param_nThreadsCPU;
	int param_nThreadsTree;
	int param_spp;
	int param_width;
	int param_height;
	int param_widthBucket;
	int param_heightBucket;
	int param_lightDepth;
	int param_eyeDepth;
	float param_blurRadius;
	char param_loadConfig[PARAM_VALUE_LENGHT];
	char param_output[PARAM_VALUE_LENGHT];
	char param_scene[PARAM_VALUE_LENGHT];
	char param_display;
	char param_verbose;
	ParamList params;

	// Add params
	params.addParamFlag(
		"help",
		"-h",
		"--help",
		"Shows this message");

	params.addParamFlag(
		"info",
		"-i",
		"--info",
		"Show device information");

	params.addParamString(
		"renderunits",
		"-r",
		"--render-units",
		"Configures the render units (default: cpu)\n" \
        "              For cpu: -r cpu\n" \
        "              For gpu: -r gpu1\n" \
        "              For multiple CPUs: -r cpu,cpu,cpu,cpu\n" \
        "              For multiple GPUs: -r gpu1,gpu2",
        "cpu", 0);

	params.addParamInt(
		"nthreadsgpu",
		"-tg",
		"--thread-GPU-number",
		"Number of threads (default 256)",
		256, 0);

	params.addParamInt(
		"nthreadscpu",
		"-tc",
		"--thread-CPU-number",
		"Number of CPU threads (default 1)",
		1, 0);

	params.addParamInt(
		"nthreadstree",
		"-tt",
		"--thread-tree",
		"Number of threads for the tree generation (default 1)",
		1, 0);

	params.addParamInt(
		"spp",
		"-spp",
		"--samples-per-pixel",
		"Samples per pixel (default 10)",
		10, 0);

	params.addParamInt(
		"width",
		"-wi",
		"--width",
		"Image width (default 512)",
		512, 0);

	params.addParamInt(
		"height",
		"-hi",
		"--height",
		"Image height (default 512)",
		512, 0);

	params.addParamInt(
		"widthbucket",
		"-wb",
		"--width-bucket",
		"Image width (default 128)",
		128, 0);

	params.addParamInt(
		"heightbucket",
		"-hb",
		"--height-bucket",
		"Image height (default 128)",
		128, 0);

	params.addParamInt(
		"lightdepth",
		"-ld",
		"--light-depth",
		"Light depth (default 4)",
		4, 0);

	params.addParamInt(
		"eyedepth",
		"-ed",
		"--eye-depth",
		"Eye depth (default 4)",
		4, 0);

	params.addParamFloat(
		"blurradius",
		"-b",
		"--blur-radius",
		"Eye depth (default 0.5)",
		0.5f, 0);

	params.addParamString(
		"loadconfig",
		"-l",
		"--load-config",
		"Loads a configuration file (default: no configuration file)",
        "", 0);

	params.addParamString(
		"output",
		"-o",
		"--output-image",
		"Output image file (default: output.png)",
        "output.png", 0);

	params.addParamString(
		"scene",
		"-s",
		"--scene",
		"Scene file",
        "", 1);

	params.addParamFlag(
		"display",
		"-d",
		"--display",
		"Display the render in a window");

	params.addParamFlag(
		"verbose",
		"-v",
		"--verbose",
		"Display render information");

	// Fill params
	params.fillValues(argc, argv);

	// Help param
	params.getValue("help", &param_help);

	if (param_help == 1)
	{
		printUsage(params);
		return 0;
	}

	// Info param
	params.getValue("info", &param_info);

	if (param_info == 1)
	{
		BVHcuda_getDeviceInfo();
		return 0;
	}

	// Check mandatory params
	if (!params.checkMandatoryParams())
	{
		return 0;
	}

	// Get params
	if (!params.getValue("renderunits", (char*)&param_renderUnits))
	{
		cout << "Error getting parameter 'renderunits'" << endl;
		return 0;
	}
	if (!params.getValue("nthreadsgpu", (char*)&param_nThreadsGPU))
	{
		cout << "Error getting parameter 'nthreadsgpu'" << endl;
		return 0;
	}
	if (!params.getValue("nthreadscpu", (char*)&param_nThreadsCPU))
	{
		cout << "Error getting parameter 'nthreadscpu'" << endl;
		return 0;
	}
	if (!params.getValue("nthreadstree", (char*)&param_nThreadsTree))
	{
		cout << "Error getting parameter 'nthreadstree'" << endl;
		return 0;
	}
	if (!params.getValue("spp", (char*)&param_spp))
	{
		cout << "Error getting parameter 'spp'" << endl;
		return 0;
	}
	if (!params.getValue("width", (char*)&param_width))
	{
		cout << "Error getting parameter 'width'" << endl;
		return 0;
	}
	if (!params.getValue("height", (char*)&param_height))
	{
		cout << "Error getting parameter 'height'" << endl;
		return 0;
	}
	if (!params.getValue("widthbucket", (char*)&param_widthBucket))
	{
		cout << "Error getting parameter 'widthbucket'" << endl;
		return 0;
	}
	if (!params.getValue("heightbucket", (char*)&param_heightBucket))
	{
		cout << "Error getting parameter 'heightbucket'" << endl;
		return 0;
	}
	if (!params.getValue("lightdepth", (char*)&param_lightDepth))
	{
		cout << "Error getting parameter 'lightdepth'" << endl;
		return 0;
	}
	if (!params.getValue("eyedepth", (char*)&param_eyeDepth))
	{
		cout << "Error getting parameter 'eyedepth'" << endl;
		return 0;
	}
	if (!params.getValue("blurradius", (char*)&param_blurRadius))
	{
		cout << "Error getting parameter 'blurradius'" << endl;
		return 0;
	}
	if (!params.getValue("loadconfig", (char*)&param_loadConfig))
	{
		cout << "Error getting parameter 'loadconfig'" << endl;
		return 0;
	}
	if (!params.getValue("output", (char*)&param_output))
	{
		cout << "Error getting parameter 'output'" << endl;
		return 0;
	}
	if (!params.getValue("scene", (char*)&param_scene))
	{
		cout << "Error getting parameter 'scene'" << endl;
		return 0;
	}
	if (!params.getValue("display", (char*)&param_display))
	{
		cout << "Error getting parameter 'display'" << endl;
		return 0;
	}
	if (!params.getValue("verbose", (char*)&param_verbose))
	{
		cout << "Error getting parameter 'verbose'" << endl;
		return 0;
	}

	// Set verbose
	BVH::setVerbose(param_verbose);

	// Check if there is configuration file the other parameters are ignored
	if (param_loadConfig[0] != '\0')
	{
		BVHConfig config;
		BVHRenderUnit *renderUnit;

		if (!config.load(param_loadConfig))
		{
			cout << "Error opening configuration file" << endl;
			return 0;
		}

		param_width = config.getWidthImage();
		param_height = config.getHeightImage();
		param_widthBucket = config.getWidthBucket();
		param_heightBucket = config.getHeightBucket();
		param_spp = config.getSPP();
		param_lightDepth = config.getLightDepth();
		param_eyeDepth = config.getEyeDepth();
		param_blurRadius = config.getBlurRadius();
		param_nThreadsTree = config.getNThreadsTree();
		nRenderUnits = config.getNRenderUnits();

		for (int i = 0; i < nRenderUnits; i++)
		{
			if ((renderUnit = config.getRenderUnit(i)) == NULL)
			{
				cout << "Error opening configuration file" << endl;
				return 0;
			}

			renderUnits[i] = *renderUnit;
		}
	}
	else
	{
		// Configure render units
		nRenderUnits = 0;
		renderUnitStringPos = 0;
		param_renderUnits_length = strlen(param_renderUnits);

		for (int i = 0; i < param_renderUnits_length; i++)
		{
			renderUnitString[renderUnitStringPos] = param_renderUnits[i];

			if (param_renderUnits[i] == ',')
			{
				renderUnitString[renderUnitStringPos] = '\0';
				renderUnitStringPos = 0;
			}
			else if (i == param_renderUnits_length - 1)
			{
				renderUnitString[renderUnitStringPos + 1] = '\0';
				renderUnitStringPos = 0;
			}
			else
			{
				renderUnitStringPos++;
			}

			if (renderUnitStringPos == 0)
			{
				if (renderUnitString[0] == 'c' &&
					renderUnitString[1] == 'p' &&
					renderUnitString[2] == 'u' &&
					renderUnitString[3] == '\0')
				{
					renderUnits[nRenderUnits].mode = BVH_MODE_CPU;
					renderUnits[nRenderUnits].nThreadsCPU = param_nThreadsCPU;
				}
				else if (
					renderUnitString[0] == 'g' &&
					renderUnitString[1] == 'p' &&
					renderUnitString[2] == 'u' &&
					renderUnitString[3] != '\0')
				{
					renderUnits[nRenderUnits].mode = BVH_MODE_GPU;
					renderUnits[nRenderUnits].deviceId = atoi(&renderUnitString[3]);
					renderUnits[nRenderUnits].nThreadsGPU = param_nThreadsGPU;
					renderUnits[nRenderUnits].nThreadsCPU = param_nThreadsCPU;
				}
				else
				{
					cout << "Incorrect render unit" << endl;
					return 0;
				}

				nRenderUnits++;
			}
		}

		if (nRenderUnits == 0)
			return 0;
	}

	cout << "Unsing " << nRenderUnits << " render units:" << endl;

	for (int i = 0; i < nRenderUnits; i++)
	{
		cout << "  Render unit " << (i + 1) << ":" << endl;

		if (renderUnits[i].mode == BVH_MODE_CPU)
		{
			cout << "    Mode: CPU" << endl;
			cout << "    CPU threads: " << renderUnits[i].nThreadsCPU << endl;
		}
		else if (renderUnits[i].mode == BVH_MODE_GPU)
		{
			cout << "    Mode: GPU" << endl;
			cout << "    Device id: " << renderUnits[i].deviceId << endl;
			cout << "    GPU threads: " << renderUnits[i].nThreadsGPU << endl;
			cout << "    CPU threads: " << renderUnits[i].nThreadsCPU << endl;
		}
		else
		{
			cout << "    Incorrect render unit [" << renderUnits[i].mode << "]. Aborting" << endl;
			return 0;
		}
	}


		/* Load scene */

	cout << "Loading scene" << endl;

	clock_gettime(CLOCK_REALTIME, &time0);
	if (!bvh.load(param_scene))
	{
		cout << "Error loading scene" << endl;
		return 0;
	}
	clock_gettime(CLOCK_REALTIME, &time1);
	timeElapsed =
		(double)(time1.tv_sec - time0.tv_sec) +
		1E-9 * (double)(time1.tv_nsec - time0.tv_nsec);
	cout << "Loading scene: " << timeElapsed << " seconds" << endl;


		/* Build tree */

	cout << "Building tree" << endl;

	clock_gettime(CLOCK_REALTIME, &time0);
	bvh.buildTree(param_nThreadsTree);
	clock_gettime(CLOCK_REALTIME, &time1);
	timeElapsed =
		(double)(time1.tv_sec - time0.tv_sec) +
		1E-9 * (double)(time1.tv_nsec - time0.tv_nsec);
	cout << "Building tree: " << timeElapsed << " seconds" << endl;


		/* Init display if necesary */

	if (param_display)
	{
		cout << "Initing display window" << endl;

		if (SDL_Init(SDL_INIT_VIDEO) < 0)
		{
			cout << "Error initing SDL. Continuing without display" << endl;
			param_display = 0;
		}
		else
		{
			// Calculate width and height
			const SDL_VideoInfo *videoInfo = SDL_GetVideoInfo();

			int scaleWidth = 1 + param_width / videoInfo->current_w;
			int scaleHeight = 1 + param_height / videoInfo->current_h;

			if (scaleWidth > scaleHeight)
			{
				sdlWidth = param_width / scaleWidth;
				sdlHeight = param_height / scaleWidth;
			}
			else
			{
				sdlWidth = param_width / scaleHeight;
				sdlHeight = param_height / scaleHeight;
			}

			if (scaleWidth > 1 || scaleHeight > 1)
				cout << "Reescaling display to " << sdlWidth << "x" << sdlHeight << endl;

			sdlScreen = SDL_SetVideoMode(sdlWidth, sdlHeight, 0, 0);

			if (sdlScreen == NULL)
			{
				cout << "Error initing video mode. Continuing without display" << endl;
				param_display = 0;
			}
		}
	}


		/* Rendering */

	cout << "Rendering" << endl;

	renderBuffer = new BVH_gpuRenderBuffer[param_width * param_height];
	imageBuffer = new unsigned char[param_width * param_height * 3];

	if (param_display)
	{
		sdlSample = 0;
		renderBufferAcc = new BVH_gpuRenderBuffer[param_width * param_height];

		// Init accumulation buffer
		for (int i = 0; i < param_width; i++)
		{
			for (int j = 0; j < param_height; j++)
			{
				renderBufferAcc[i + j * param_width].alpha = 0.0f;
				renderBufferAcc[i + j * param_width].rgb[0] = 0.0f;
				renderBufferAcc[i + j * param_width].rgb[1] = 0.0f;
				renderBufferAcc[i + j * param_width].rgb[2] = 0.0f;
			}
		}

		clock_gettime(CLOCK_REALTIME, &time0);

		// Main loop
		while (1)
		{
			// Manage events
			SDL_PollEvent(&sdlEvent);

			if (sdlEvent.type == SDL_QUIT)
				break;

			if (sdlEvent.type == SDL_KEYDOWN && sdlEvent.key.keysym.sym == SDLK_ESCAPE)
				break;

			// Render one sample
			if (sdlSample < param_spp)
			{
				bvh.render(
					param_width, param_height,
					renderBuffer, 1,
					param_lightDepth, param_eyeDepth, param_blurRadius,
					param_widthBucket, param_heightBucket,
					nRenderUnits, renderUnits);
				sdlSample += 1;

				// Update display
				for (int i = 0; i < param_width; i++)
				{
					for (int j = 0; j < param_height; j++)
					{
						int jFlip = param_height - j - 1;

						BVHColor::gammaCorrectRGB(renderBuffer[i + jFlip * param_width].rgb);
						renderBufferAcc[i + j * param_width].alpha += renderBuffer[i + jFlip * param_width].alpha;
						renderBufferAcc[i + j * param_width].rgb[0] += renderBuffer[i + jFlip * param_width].rgb[0];
						renderBufferAcc[i + j * param_width].rgb[1] += renderBuffer[i + jFlip * param_width].rgb[1];
						renderBufferAcc[i + j * param_width].rgb[2] += renderBuffer[i + jFlip * param_width].rgb[2];

						imageBuffer[(i + j * param_width) * 3] =
							(1.0f - renderBufferAcc[i + j * param_width].alpha / sdlSample) * 0 +
							renderBufferAcc[i + j * param_width].alpha * clamp(renderBufferAcc[i + j * param_width].rgb[0] / (sdlSample * sdlSample));
						imageBuffer[(i + j * param_width) * 3 + 1] =
							(1.0f - renderBufferAcc[i + j * param_width].alpha / sdlSample) * 0 +
							renderBufferAcc[i + j * param_width].alpha * clamp(renderBufferAcc[i + j * param_width].rgb[1] / (sdlSample * sdlSample));
						imageBuffer[(i + j * param_width) * 3 + 2] =
							(1.0f - renderBufferAcc[i + j * param_width].alpha / sdlSample) * 0 +
							renderBufferAcc[i + j * param_width].alpha * clamp(renderBufferAcc[i + j * param_width].rgb[2] / (sdlSample * sdlSample));
					}
				}

				SDL_Surface *frame = SDL_CreateRGBSurfaceFrom(
					(void*)imageBuffer, param_width, param_height, 24, param_width * 3, 0x0000FF, 0x00FF00, 0xFF0000, 0x000000);
				SDL_BlitSurface(frame, NULL, sdlScreen, NULL);
				SDL_FreeSurface(frame);
				SDL_UpdateRect(sdlScreen, 0, 0, 0, 0);

				clock_gettime(CLOCK_REALTIME, &time1);
				timeElapsed =
					(double)(time1.tv_sec - time0.tv_sec) +
					1E-9 * (double)(time1.tv_nsec - time0.tv_nsec);
				cout << "Rendered " << sdlSample << " samples in ";
				BVH::printTime(timeElapsed, 1);
				cout << endl;
			}
		}

		// Save render buffer
		for (int i = 0; i < param_width; i++)
		{
			for (int j = 0; j < param_height; j++)
			{
				int jFlip = param_height - j - 1;

				renderBuffer[i + jFlip * param_width].alpha = renderBufferAcc[i + j * param_width].alpha / sdlSample;
				renderBuffer[i + jFlip * param_width].rgb[0] = renderBufferAcc[i + j * param_width].rgb[0] / sdlSample;
				renderBuffer[i + jFlip * param_width].rgb[1] = renderBufferAcc[i + j * param_width].rgb[1] / sdlSample;
				renderBuffer[i + jFlip * param_width].rgb[2] = renderBufferAcc[i + j * param_width].rgb[2] / sdlSample;
			}
		}

		// Destroy render buffer accumulator
		delete [] renderBufferAcc;
	}
	else
	{
		clock_gettime(CLOCK_REALTIME, &time0);
		bvh.render(
			param_width, param_height,
			renderBuffer, param_spp,
			param_lightDepth, param_eyeDepth, param_blurRadius,
			param_widthBucket, param_heightBucket,
			nRenderUnits, renderUnits);
		clock_gettime(CLOCK_REALTIME, &time1);
		timeElapsed =
			(double)(time1.tv_sec - time0.tv_sec) +
			1E-9 * (double)(time1.tv_nsec - time0.tv_nsec);
		cout << "Rendering: ";
		BVH::printTime(timeElapsed, 1);
		cout << endl;
	}


		/* Save image */

	for (int i = 0; i < param_width; i++)
	{
		for (int j = 0; j < param_height; j++)
		{
			if (!param_display)
				BVHColor::gammaCorrectRGB(renderBuffer[i + j * param_width].rgb);

			imageBuffer[(i + j * param_width) * 3] =
				(1.0f - renderBuffer[i + j * param_width].alpha) * 0 +
				renderBuffer[i + j * param_width].alpha * clamp(renderBuffer[i + j * param_width].rgb[0]);
			imageBuffer[(i + j * param_width) * 3 + 1] =
				(1.0f - renderBuffer[i + j * param_width].alpha) * 0 +
				renderBuffer[i + j * param_width].alpha * clamp(renderBuffer[i + j * param_width].rgb[1]);
			imageBuffer[(i + j * param_width) * 3 + 2] =
				(1.0f - renderBuffer[i + j * param_width].alpha) * 0 +
				renderBuffer[i + j * param_width].alpha * clamp(renderBuffer[i + j * param_width].rgb[2]);
		}
	}

	savePNG(param_output, param_width, param_height, imageBuffer);

	// Delete buffers
	delete [] renderBuffer;
	delete [] imageBuffer;


		/* Closing display */

	if (param_display)
		SDL_Quit();


		/* Return ok */

	cout << "Exiting" << endl;
	return 0;
}
