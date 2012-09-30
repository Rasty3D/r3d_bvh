/*
 * main.cpp
 *
 *  Created on: 18 Apr 2011
 *      Author: showroom
 */


/*
 * INCLUDES
 */

	/* General */
#include <iostream>
#include <fstream>
#include <signal.h>
#include <math.h>

	/* BVH */
#include "bvhcolor.h"
#include "bvhshader.h"


/*
 * NAMESPACES
 */

using namespace std;


/*
 * PRINT USAGE
 */

void printUsage()
{
#ifdef linux
	cout << endl;
	cout << "\033[1mNAME\033[0m" << endl;
	cout << "       BVH shader compiler" << endl;
	cout << endl;
	cout << "\033[1mSYNOPSIS\033[0m" << endl;
	cout << "       \033[1mbvhc\033[0m [\033[4mOPTIONS\033[0m] shader..." << endl;
	cout << endl;
	cout << "\033[1mDESCRIPTION\033[0m" << endl;
	cout << "       BVH shader compiler" << endl;
	cout << endl;
	cout << "       For more information refer to the authors" << endl;
	cout << endl;
	cout << "       List of arguments:" << endl;
	cout << endl;
	cout << "       \033[1m-h, --help\033[0m" << endl;
 	cout << "             Shows this message" << endl;
 	cout << endl;
	cout << "       \033[1m-v, --verbose\033[0m" << endl;
	cout << "              Shows compiling information" << endl;
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
	cout << "       BVH shader compiler" << endl;
	cout << endl;
	cout << "SYNOPSIS" << endl;
	cout << "       bvhc [OPTIONS] shader..." << endl;
	cout << endl;
	cout << "DESCRIPTION" << endl;
	cout << "       BVH shader compiler" << endl;
	cout << endl;
	cout << "       For more information refer to the authors" << endl;
	cout << endl;
	cout << "       List of arguments:" << endl;
	cout << endl;
	cout << "       -h, --help" << endl;
 	cout << "             Shows this message" << endl;
 	cout << endl;
	cout << "       -v, --verbose" << endl;
	cout << "              Shows compiling information" << endl;
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

	BVHColor::setColorSystem("HDTV");


		/* Variables */

	BVHShader shader;


		/* Params */

	// Params variables
	char param_help = 0;
	char param_verbose = 0;
	char *param_shader = NULL;

	// Check params
	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-h") == 0 ||
			strcmp(argv[i], "--help") == 0)
		{
			param_help = 1;
		}
		else if (strcmp(argv[i], "-v") == 0 ||
			strcmp(argv[i], "--verbose") == 0)
		{
			param_verbose = 1;
		}
		else
		{
			param_shader = argv[i];
		}
	}

	if (param_help)
	{
		printUsage();
		return 0;
	}

	if (param_shader == NULL)
	{
		cout << "The shader name parameter is missing" << endl;
		printUsage();
		return 0;
	}


		/* Compile shader */

	if (!shader.compile(param_shader, param_verbose))
	{
		shader.printCompileLog();
		cout << "Error compiling shader" << endl;
		return 0;
	}
	else
	{
		if (param_verbose)
			cout << "Shader compiled succesfully" << endl;
	}


		/* Try to load to check if it's correct */

	if (!shader.load(param_shader))
	{
		cout << "Error loading shader" << endl;
		return 0;
	}
	else
	{
		if (param_verbose)
			cout << "Shader loaded succesfully" << endl;
	}


		/* Unload the shader */

	shader.unload();


		/* Return ok */

	return 0;
}
