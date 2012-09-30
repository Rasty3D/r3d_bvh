/*
 * bvhshader.cpp
 *
 *  Created on: 24 Feb 2011
 *      Author: showroom
 */

/*
 * INCLUDES
 */

#include "bvhshader.h"


/*
 * NAMESPACES
 */

using namespace std;


/*
 * CLASS: BVHShader
 */

	/* Compilation options */

char *BVHShader::compileCommand = BVHSHADER_COMPILE_COMMAND;
char *BVHShader::compileIncPath = BVHSHADER_COMPILE_INCPATH;
char *BVHShader::compileSrcFiles = BVHSHADER_COMPILE_SRCFILES;
char *BVHShader::compileSrcUtils = BVHSHADER_COMPILE_SRCUTILS;
char *BVHShader::compileSrcPath = BVHSHADER_COMPILE_SRCPATH;
char *BVHShader::compileSrcExt = BVHSHADER_COMPILE_SRCEXT;
char *BVHShader::compileObjPath = BVHSHADER_COMPILE_OBJPATH;
char *BVHShader::compileObjExt = BVHSHADER_COMPILE_OBJEXT;


	/* Compile log */

int BVHShader::compileLogSize;
char BVHShader::compileLog[BVHSHADER_COMPILE_LOG_MAX_SIZE];


	/* Auxiliary functions for the shaders */

BVHShader_functions BVHShader::functions;


	/* Constructor and destructor */

BVHShader::BVHShader()
{
	this->handle = NULL;
	this->pGetType = NULL;
	this->pGetParams = NULL;
	this->pSetParam = NULL;
	this->pExecute = NULL;
	this->params = NULL;

	this->pGetSampling = NULL;
	this->pGetNormal = NULL;
	this->samplingList = NULL;
}

BVHShader::~BVHShader()
{
	if (this->handle != NULL)
		dlclose(this->handle);
	this->handle = NULL;

	if (this->samplingList == NULL)
		delete this->samplingList;
}


	/* Compile */

void BVHShader::setCompileCommand(const char *command)
{
	BVHShader::compileCommand = new char[strlen(command) + 1];
	strcpy(BVHShader::compileCommand, command);
}

void BVHShader::setCompileIncPath(const char *path)
{
	BVHShader::compileIncPath = new char[strlen(path) + 1];
	strcpy(BVHShader::compileIncPath, path);
}

void BVHShader::setCompileSrcFiles(const char *files)
{
	BVHShader::compileSrcFiles = new char[strlen(files) + 1];
	strcpy(BVHShader::compileSrcFiles, files);
}

void BVHShader::setCompileSrcUtils(const char *utils)
{
	BVHShader::compileSrcUtils = new char[strlen(utils) + 1];
	strcpy(BVHShader::compileSrcUtils, utils);
}

void BVHShader::setCompileSrcPath(const char *path)
{
	BVHShader::compileSrcPath = new char[strlen(path) + 1];
	strcpy(BVHShader::compileSrcPath, path);
}

void BVHShader::setCompileSrcExt(const char *ext)
{
	BVHShader::compileSrcExt = new char[strlen(ext) + 1];
	strcpy(BVHShader::compileSrcExt, ext);
}

void BVHShader::setCompileObjPath(const char *path)
{
	BVHShader::compileObjPath = new char[strlen(path) + 1];
	strcpy(BVHShader::compileObjPath, path);
}

void BVHShader::setCompileObjExt(const char *ext)
{
	BVHShader::compileObjExt = new char[strlen(ext) + 1];
	strcpy(BVHShader::compileObjExt, ext);
}

int BVHShader::compile(const char *name, int verbose)
{
	char command[256];
	FILE *fpipe;
	char line[256];

	// Delete previous file
	sprintf(command, "rm -f %s/%s%s",
		BVHShader::compileObjPath, name, BVHShader::compileObjExt);
	if (verbose)
		cout << command << endl;
	system(command);

	// Compile
	sprintf(command, "%s %s ", BVHShader::compileCommand, BVHShader::compileIncPath);
	if (this->checkClass(name, "BVHColor") || this->checkClass(name, "\"bvhcolor.h\""))
		sprintf(command + strlen(command), "%s/bvhcolor.cpp ", BVHShader::compileSrcFiles);
	if (this->checkClass(name, "BVHShader"))
		sprintf(command + strlen(command), "%s/bvhshader.cpp ", BVHShader::compileSrcFiles);
	if (this->checkClass(name, "BVHTransform") || this->checkClass(name, "\"bvhtransform.h\""))
		sprintf(command + strlen(command), "%s/bvhtransform.cpp %s/mathaux.cpp ", BVHShader::compileSrcFiles, BVHShader::compileSrcUtils);
	sprintf(command + strlen(command), "%s/%s%s -o %s/%s%s",
		BVHShader::compileSrcPath, name, BVHShader::compileSrcExt,
		BVHShader::compileObjPath, name, BVHShader::compileObjExt);

	if (verbose)
		cout << command << endl;
	strcat(command, " 2>&1");

	if (!(fpipe = (FILE*)popen(command, "r")))
		return 0;

	BVHShader::compileLogSize = 0;
	BVHShader::compileLog[0] = '\0';

	if (verbose)
	{
		while (fgets(line, 256, fpipe))
		{
			cout << line;
		}
	}
	else
	{
		while (fgets(BVHShader::compileLog + BVHShader::compileLogSize, BVHSHADER_COMPILE_LOG_MAX_SIZE - BVHShader::compileLogSize, fpipe) &&
			BVHShader::compileLogSize < BVHSHADER_COMPILE_LOG_MAX_SIZE - 1)
		{
			BVHShader::compileLogSize = strlen(BVHShader::compileLog);
		}

		while (fgets(line, 256, fpipe))
		{
			// Do nothing
		}
	}
	pclose(fpipe);

	// Check if the file is there
	sprintf(command, "%s/%s%s",
		BVHShader::compileObjPath, name, BVHShader::compileObjExt);
	ifstream file;
	file.open(command, ios::binary);
	if (!file)
		return 0;
	file.close();

	return 1;
}

int BVHShader::getCompileLogSize()
{
	return BVHShader::compileLogSize;
}

void BVHShader::getCompileLog(char *info)
{
	if (BVHShader::compileLogSize > 0)
		strcpy(info, BVHShader::compileLog);
	else
		info[0] = '\0';
}

void BVHShader::printCompileLog()
{
	if (BVHShader::compileLogSize > 0)
		cout << BVHShader::compileLog;
}


	/* Load and unload dynamic library */

int BVHShader::load(const char *name)
{
	char filename[256];
	char *error;

	sprintf(filename, "%s/%s%s",
		BVHShader::compileObjPath, name, BVHShader::compileObjExt);
	this->handle = dlopen(filename, RTLD_NOW | RTLD_LOCAL);

	if (this->handle == NULL)
	{
		cout << "Error opening shader [" << dlerror() << "]" << endl;
		return 0;
	}

	*(void**)(&this->pGetType) = dlsym(this->handle, "getType");

	if ((error = dlerror()) != NULL)
	{
		cout << "Error loading 'getType' function [" << error << "]" << endl;
		this->unload();
		return 0;
	}

	*(void**)(&this->pGetParams) = dlsym(this->handle, "getParams");

	if ((error = dlerror()) != NULL)
	{
		cout << "Error loading 'getParams' function [" << error << "]" << endl;
		this->unload();
		return 0;
	}

	*(void**)(&this->pSetParam) = dlsym(this->handle, "setParam");

	if ((error = dlerror()) != NULL)
	{
		cout << "Error loading 'setParam' function [" << error << "]" << endl;
		this->unload();
		return 0;
	}

	*(void**)(&this->pExecute) = dlsym(this->handle, "execute");

	if ((error = dlerror()) != NULL)
	{
		cout << "Error loading 'execute' function [" << error << "]" << endl;
		this->unload();
		return 0;
	}

	// Get params
	this->params = (*this->pGetParams)();

	// Only for surface shader (optional functions)
	*(void**)(&this->pGetSampling) = dlsym(this->handle, "getSampling");

	if ((error = dlerror()) != NULL)
		this->pGetSampling = NULL;

	*(void**)(&this->pGetNormal) = dlsym(this->handle, "getNormal");

	if ((error = dlerror()) != NULL)
		this->pGetNormal = NULL;

	// Return ok
	return 1;
}

int BVHShader::compileAndLoad(const char *name, int verbose)
{
	if (!this->compile(name, verbose))
		return 0;

	return this->load(name);
}

void BVHShader::unload()
{
	if (this->handle != NULL)
		dlclose(this->handle);

	this->handle = NULL;
}


	/* Call library functions */

int BVHShader::getType()
{
	if (this->handle != NULL)
		return (*this->pGetType)();
	else
		return BVHSHADER_TYPE_UNDEFINED;
}

void BVHShader::setParam(const char *name, void *value)
{
	if (this->handle != NULL)
		(*this->pSetParam)(this->params, name, value);
}

void BVHShader::execute(void *env)
{
	if (this->handle != NULL)
		(*this->pExecute)(this->params, &BVHShader::functions, env);
}

// Only for surface shader (optional functions)
BVHShader_samplingList *BVHShader::getSampling()
{
	if (this->samplingList == NULL)
	{
		this->samplingList = new BVHShader_samplingList;

		if (this->pGetSampling == NULL)
		{
			this->samplingList->samplingNumber = 1;
			this->samplingList->samplings[0].type = BVHSHADER_SAMPLING_DIFFUSE;
			this->samplingList->samplings[0].pdf = 1.0f;
		}
		else
		{
			(*this->pGetSampling)(this->params, this->samplingList);

			// Fix sampling
			float pdfAcc = 0.0f;

			for (int i = 0; i < this->samplingList->samplingNumber; i++)
			{
				pdfAcc += this->samplingList->samplings[i].pdf;
				this->samplingList->samplings[i].pdfAcc = pdfAcc;
			}

			for (int i = 0; i < this->samplingList->samplingNumber; i++)
			{
				this->samplingList->samplings[i].pdf /= pdfAcc;
				this->samplingList->samplings[i].pdfAcc /= pdfAcc;
			}
		}
	}

	return this->samplingList;
}

void BVHShader::getNormal(void *env)
{
	if (this->handle != NULL && this->pGetNormal != NULL)
		(*this->pGetNormal)(this->params, &BVHShader::functions, env);
}


	/* Set auxiliary functions */

void BVHShader::setFunc_rng(float(*func)(void*, float, float))
{
	BVHShader::functions.rng = func;
}

void BVHShader::setFunc_getColor(BVHColor(*func)(const void*, float*))
{
	BVHShader::functions.getColor = func;
}

void BVHShader::setFunc_getColorUV(BVHColor(*func)(const void*, float, float))
{
	BVHShader::functions.getColorUV = func;
}

void BVHShader::setFunc_getColorUVW(BVHColor(*func)(const void*, float, float, float))
{
	BVHShader::functions.getColorUVW = func;
}

void BVHShader::setFunc_getIntensity(float(*func)(const void*, float*))
{
	BVHShader::functions.getIntensity = func;
}

void BVHShader::setFunc_getIntensityUV(float(*func)(const void*, float, float))
{
	BVHShader::functions.getIntensityUV = func;
}

void BVHShader::setFunc_getIntensityUVW(float(*func)(const void*, float, float, float))
{
	BVHShader::functions.getIntensityUVW = func;
}


	/* Look for required classes */

int BVHShader::checkClass(const char *name, const char *className)
{
	char filename[256];
	char line[256];
	char *str;
	int len = strlen(className);

	sprintf(filename, "%s/%s%s", BVHShader::compileSrcPath, name, BVHShader::compileSrcExt);

	ifstream file;
	file.open(filename);

	if (!file)
		return 0;

	while (!file.eof())
	{
		file.getline(line, 256);

		if ((str = strstr(line, className)) != NULL)
		{
			if (line == str &&
				(str[len] == ' ' || str[len] == '*' ||
				 str[len] == '&' || str[len] == ')' ||
				 str[len] == ':' || str[len] == '.' ||
				 str[len] == ',' || str[len] == '\t' ||
				 str[len] == '\r' || str[len] == '\n' ||
				 str[len] == '\0'))
			{
				file.close();
				return 1;
			}
			else if (line != str &&
				(str[-1] == ' ' || str[-1] == '*' ||
				 str[-1] == '&' || str[-1] == '(' ||
				 str[-1] == ':' || str[-1] == '.' ||
				 str[-1] == ',' || str[-1] == '\t' ||
				 str[-1] == '\r' || str[-1] == '\n') &&
				(str[len] == ' ' || str[len] == '*' ||
				 str[len] == '&' || str[len] == ')' ||
				 str[len] == ':' || str[len] == '.' ||
				 str[len] == ',' || str[len] == '\t' ||
				 str[len] == '\r' || str[len] == '\n' ||
				 str[len] == '\0'))
			{
				file.close();
				return 1;
			}
		}
	}

	file.close();
	return 0;
}
