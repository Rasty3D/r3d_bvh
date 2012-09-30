/*
 * bvhscene.cpp
 *
 *  Created on: 6 May 2011
 *      Author: showroom
 */


/*
 * INCLUDES
 */

#include "bvhscene.h"


/*
 * NAMESPACES
 */

using namespace std;


/*
 * CLASS: BVHScene
 */

	/* Constructor and destructor */

BVHScene::BVHScene()
{
	this->cameraCurrent = NULL;
}

BVHScene::~BVHScene()
{
	this->clear();
}


	/* Load scene */

int BVHScene::load(const char *filename)
{
		/* Variables */

	NMLParser parser;


		/* Open parser */

	if (!parser.open(filename))
		return 0;


		/* Load general */

	if (!this->loadGeneral(parser))
	{
		// Print the last error
		NMLParser_error *lastError = parser.getLastError();

		cout << "Found error in file " << parser.getFilename() << " in line " << parser.getCurrentLine();

		if (lastError != NULL)
			cout << ": " << lastError->message << endl;
		else
			cout << endl;

		// Close parser and exit with error
		parser.close();
		return 0;
	}

	parser.close();
	this->transformPrimitives();
	return 1;
}

	/* Clear scene */

void BVHScene::clear()
{
	this->textures.clear();
	this->shaderTextures.clear();
	this->shaderMasks.clear();
	this->materials.clear();

	this->lights.clear();
	this->lightRefs.clear();

	this->cameras.clear();
	this->cameraRefs.clear();

	this->objects.clear();
	this->objectRefs.clear();
}


	/* Get camera ray */

void BVHScene::getCameraRay(
	float x, float y,
	int *kernelDim, int *kernelIdx,
	float *rayPos, float *rayDir,
	gsl_rng *rng)
{
	if (this->cameraCurrent == NULL)
		return;

	this->cameraCurrent->camera->getRay(
		x, y, kernelDim, kernelIdx,
		rayPos, rayDir, rng);

	this->cameraCurrent->transform.transformVector(rayPos);
	this->cameraCurrent->transform.rotateVector(rayDir);
}


	/* Get light ray */

// It returns the pointer of the selected light
BVHLightInfo BVHScene::getLightRay(float *rayPos, float *rayDir, float *normal, float &pdf, gsl_rng *rng)
{
	BVHLightInfo lightInfo = {NULL, NULL};
	int lightId;
	int index;
	list<BVHLightRef>::iterator lightIt;
	BVHShader_emitter emitter;

	// Select random light
	lightId = randInt(rng, 0, this->lightRefs.size()) % this->lightRefs.size();
	index = 0;

	for (lightIt = this->lightRefs.begin(); lightIt != this->lightRefs.end(); lightIt++)
	{
		if (index == lightId)
		{
			lightInfo.lightRef = &(*lightIt);
			break;
		}

		index++;
	}

	// Calculate ray
	emitter.rng = (void*)rng;
	lightInfo.material = lightInfo.lightRef->light->getRay(rng, &emitter);

	// Transform ray
	lightInfo.lightRef->transform.transformVector(emitter.rayPos, rayPos);
	lightInfo.lightRef->transform.rotateVector(emitter.rayDir, rayDir);
	lightInfo.lightRef->transform.rotateVector(emitter.normal, normal);

	// Get pdf
	pdf = emitter.pdf;

	// Return selected light
	return lightInfo;
}

void BVHScene::getLightColor(BVHLightInfo &lightInfo, BVHShader_light *lightEnv)
{
	if (lightInfo.material == NULL)
		lightInfo.lightRef->light->getColor(lightEnv);
	else
		lightInfo.material->execute((void*)lightEnv);

	lightEnv->emission *= this->lightRefs.size();
}


	/* Load general from parser */

int BVHScene::loadGeneral(NMLParser &parser)
{
	NMLParser_token token;
	char name[NMLPARSER_TOKEN_LEN];
	char filename[NMLPARSER_TOKEN_LEN];

	while (1)
	{
		if (!parser.getWord(token))
		{
			if (token.type == NMLPARSER_EOF)
				return 1;
			else
				break;
		}

		if (strcmp(token.value, "include") == 0)
		{
			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!parser.getFile(filename))
				break;

			// Load include file
			NMLParser parserInclude;

			if (!parserInclude.open(filename))
				break;

			if (!this->loadGeneral(parserInclude))
			{
				parserInclude.close();
				break;
			}

			parserInclude.close();

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;
		}
		else if (strcmp(token.value, "global") == 0)
		{
			if (!this->loadGlobal(parser))
				break;
		}
		else if (strcmp(token.value, "texture") == 0)
		{
			BVHTexture *texture = new BVHTexture;

			if (!this->loadTexture(parser, texture, name))
			{
				delete texture;
				break;
			}

			// Add texture to the list
			if (!this->textures.add(name, *texture))
				break;
		}
		else if (strcmp(token.value, "shader-texture") == 0)
		{
			BVHShader *shader = new BVHShader;

			if (!this->loadShader(parser, shader, name, 0))
			{
				delete shader;
				break;
			}

			// Add shader texture to the list
			if (!this->shaderTextures.add(name, *shader))
				break;
		}
		else if (strcmp(token.value, "shader-mask") == 0)
		{
			BVHShader *shader = new BVHShader;

			if (!this->loadShader(parser, shader, name, 0))
			{
				delete shader;
				break;
			}

			// Add shader mask to the list
			if (!this->shaderMasks.add(name, *shader))
				break;
		}
		else if (strcmp(token.value, "material") == 0)
		{
			BVHShader *shader = new BVHShader;

			if (!this->loadShader(parser, shader, name, 0))
			{
				delete shader;
				break;
			}

			// Add material to the list
			if (!this->materials.add(name, *shader))
				break;
		}
		else if (strcmp(token.value, "camera") == 0)
		{
			BVHCamera *camera =  new BVHCamera;

			if (!this->loadCamera(parser, camera, name, 0))
			{
				delete camera;
				break;
			}

			// Add camera to the list
			if (!this->cameras.add(name, *camera))
				break;
		}
		else if (strcmp(token.value, "light") == 0)
		{
			BVHLight *light = new BVHLight;

			if (!this->loadLight(parser, light, name, 0))
			{
				delete light;
				break;
			}

			// Add light to the list
			if (!this->lights.add(name, *light))
				break;
		}
		else if (strcmp(token.value, "object") == 0)
		{
			BVHObject *object = new BVHObject;

			if (!this->loadObject(parser, object, name, 0))
			{
				delete object;
				break;
			}

			// Add object to the list
			if (!this->objects.add(name, *object))
				break;
		}
		else if (strcmp(token.value, "transform") == 0)
		{
			BVHTransform transform;
			transform.identity();

			if (!this->loadTransform(parser, transform))
				break;
		}
		else
		{
			break;
		}
	}

	return 0;
}


	/* Load parts */

int BVHScene::loadGlobal(NMLParser &parser)
{
	NMLParser_token token;
	unsigned char elementsFlag = 0x00;

	if (!parser.getSeparator(NMLPARSER_OPEN_BRACKET, token))
		return 0;

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

		if (strcmp(token.value, "sky") == 0)
		{
			if (elementsFlag & BVHSCENE_GLOBAL_SKY)
				break;

			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!this->loadColor(parser, &this->skyColor))
				break;

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;

			elementsFlag |= BVHSCENE_GLOBAL_SKY;
		}
		else
		{
			break;
		}
	}

	return 0;
}

int BVHScene::loadTexture(NMLParser &parser, BVHTexture *texture, char *name)
{
	NMLParser_token token;
	char filename[NMLPARSER_TOKEN_LEN];
	char path[NMLPARSER_TOKEN_LEN];
	unsigned char elementsFlag = 0x00;

	if (!parser.getSeparator(NMLPARSER_OPEN_BRACKET, token))
		return 0;

	getPath(parser.getFilename(), path);

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
			return ((elementsFlag & BVHSCENE_TEXTURE_MUST) == BVHSCENE_TEXTURE_MUST);

		if (strcmp(token.value, "name") == 0)
		{
			if (elementsFlag & BVHSCENE_TEXTURE_NAME)
				break;

			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!parser.getWord(token))
				break;

			strcpy(name, token.value);

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;

			elementsFlag |= BVHSCENE_TEXTURE_NAME;
		}
		else if (strcmp(token.value, "img") == 0)
		{
			if (elementsFlag & BVHSCENE_TEXTURE_IMG)
				break;

			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!parser.getFile(filename))
				break;

			addPath(filename, path);

			if (!texture->load(filename))
				break;

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;

			elementsFlag |= BVHSCENE_TEXTURE_IMG;
		}
		else if (strcmp(token.value, "wrap-s") == 0)
		{
			if (elementsFlag & BVHSCENE_TEXTURE_WRAPS)
				break;

			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!parser.getWord(token))
				break;

			if (strcmp(token.value, "repeat") == 0)
				texture->setWraps(BVHTEXTURE_WRAP_REPEAT);
			else if (strcmp(token.value, "mirror") == 0)
				texture->setWraps(BVHTEXTURE_WRAP_MIRRORED_REPEAT);
			else if (strcmp(token.value, "clamp") == 0)
				texture->setWraps(BVHTEXTURE_WRAP_CLAMP);
			else
				break;

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;

			elementsFlag |= BVHSCENE_TEXTURE_WRAPS;
		}
		else if (strcmp(token.value, "wrap-t") == 0)
		{
			if (elementsFlag & BVHSCENE_TEXTURE_WRAPT)
				break;

			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!parser.getWord(token))
				break;

			if (strcmp(token.value, "repeat") == 0)
				texture->setWrapt(BVHTEXTURE_WRAP_REPEAT);
			else if (strcmp(token.value, "mirror") == 0)
				texture->setWrapt(BVHTEXTURE_WRAP_MIRRORED_REPEAT);
			else if (strcmp(token.value, "clamp") == 0)
				texture->setWrapt(BVHTEXTURE_WRAP_CLAMP);
			else
				break;

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;

			elementsFlag |= BVHSCENE_TEXTURE_WRAPT;
		}
		else
		{
			break;
		}
	}

	return 0;
}

int BVHScene::loadShader(NMLParser &parser, BVHShader *shader, char *name, int standalone)
{
	NMLParser_token token;
	char filename[NMLPARSER_TOKEN_LEN];
	unsigned char elementsFlag = 0x00;

	if (!parser.getSeparator(NMLPARSER_OPEN_BRACKET, token))
		return 0;

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
			return ((elementsFlag & BVHSCENE_SHADER_MUST) == BVHSCENE_SHADER_MUST);

		if (strcmp(token.value, "name") == 0)
		{
			if (elementsFlag & BVHSCENE_SHADER_NAME)
				break;

			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!parser.getWord(token))
				break;

			strcpy(name, token.value);

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;

			elementsFlag |= BVHSCENE_SHADER_NAME;
		}
		else if (strcmp(token.value, "shader") == 0)
		{
			if (elementsFlag & BVHSCENE_SHADER_SHADER)
				break;

			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!parser.getFile(filename))
				break;

			if (!shader->load(filename))
				break;

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;

			elementsFlag |= BVHSCENE_SHADER_SHADER;
		}
		else if (strcmp(token.value, "param") == 0)
		{
			if (!(elementsFlag & BVHSCENE_SHADER_SHADER))
				break;

			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!this->loadParam(parser, shader))
				break;

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;

			elementsFlag |= BVHSCENE_SHADER_PARAM;
		}
		else
		{
			break;
		}
	}

	return 0;
}

int BVHScene::loadCamera(NMLParser &parser, BVHCamera *camera, char *name, int standalone)
{
	NMLParser_token token;
	unsigned char elementsFlag = 0x00;
	char filename[NMLPARSER_TOKEN_LEN];

	if (!parser.getSeparator(NMLPARSER_OPEN_BRACKET, token))
		return 0;

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
			return ((elementsFlag & BVHSCENE_CAMERA_MUST) == BVHSCENE_CAMERA_MUST);

		if (strcmp(token.value, "name") == 0)
		{
			if (elementsFlag & BVHSCENE_CAMERA_NAME)
				break;

			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!parser.getWord(token))
				break;

			strcpy(name, token.value);

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;

			elementsFlag |= BVHSCENE_CAMERA_NAME;
		}
		else if (strcmp(token.value, "shader") == 0)
		{
			if (elementsFlag & BVHSCENE_CAMERA_SHADER)
				break;

			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!parser.getFile(filename))
				break;

			if (!camera->initShader(filename))
				break;

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;

			elementsFlag |= BVHSCENE_CAMERA_SHADER;
		}
		else if (strcmp(token.value, "jitter") == 0)
		{
			if (elementsFlag & BVHSCENE_CAMERA_JITTER)
				break;

			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!parser.getFile(filename))
				break;

			if (!camera->initJitter(filename))
				break;

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;

			elementsFlag |= BVHSCENE_CAMERA_JITTER;
		}
		else if (strcmp(token.value, "param-shader") == 0)
		{
			if (!(elementsFlag & BVHSCENE_CAMERA_SHADER))
				break;

			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!this->loadParam(parser, camera->getShader()))
				break;

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;

			elementsFlag |= BVHSCENE_CAMERA_PARAM_SHADER;
		}
		else if (strcmp(token.value, "param-jitter") == 0)
		{
			if (!(elementsFlag & BVHSCENE_CAMERA_JITTER))
				break;

			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!this->loadParam(parser, camera->getJitter()))
				break;

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;

			elementsFlag |= BVHSCENE_CAMERA_PARAM_JITTER;
		}
		else
		{
			break;
		}
	}

	return 0;
}

int BVHScene::loadLight(NMLParser &parser, BVHLight *light, char *name, int standalone)
{
	NMLParser_token token;
	char filename[NMLPARSER_TOKEN_LEN];
	unsigned char elementsFlag = 0x00;

	if (!parser.getSeparator(NMLPARSER_OPEN_BRACKET, token))
		return 0;

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
			return ((elementsFlag & BVHSCENE_LIGHT_MUST_1) == BVHSCENE_LIGHT_MUST_1 ||
					(elementsFlag & BVHSCENE_LIGHT_MUST_2) == BVHSCENE_LIGHT_MUST_2);

		if (strcmp(token.value, "name") == 0)
		{
			if (elementsFlag & BVHSCENE_LIGHT_NAME)
				break;

			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!parser.getWord(token))
				break;

			strcpy(name, token.value);

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;

			elementsFlag |= BVHSCENE_LIGHT_NAME;
		}
		else if (strcmp(token.value, "shader") == 0)
		{
			if (elementsFlag & BVHSCENE_LIGHT_SHADER)
				break;

			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!parser.getFile(filename))
				break;

			if (!light->initShader(filename))
				break;

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;

			elementsFlag |= BVHSCENE_LIGHT_SHADER;
		}
		else if (strcmp(token.value, "emitter-shader") == 0)
		{
			if (elementsFlag & BVHSCENE_LIGHT_EMITTER_SHADER)
				break;

			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!parser.getFile(filename))
				break;

			if (!light->initEmitter(filename))
				break;

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;

			elementsFlag |= BVHSCENE_LIGHT_EMITTER_SHADER;
		}
		else if (strcmp(token.value, "emitter-object") == 0)
		{
			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!parser.getWord(token))
				break;

			BVHObject *object = this->objects.get(token.value);

			if (object == NULL)
				break;

			light->addPrimitives(&object->primitives);

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;

			elementsFlag |= BVHSCENE_LIGHT_EMITTER_OBJECT;
		}
		else if (strcmp(token.value, "param-shader") == 0)
		{
			if (!(elementsFlag & BVHSCENE_LIGHT_SHADER))
				break;

			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!this->loadParam(parser, light->getShader()))
				break;

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;

			elementsFlag |= BVHSCENE_LIGHT_PARAM_SHADER;
		}
		else if (strcmp(token.value, "param-emitter") == 0)
		{
			if (!(elementsFlag & BVHSCENE_LIGHT_EMITTER_SHADER))
				break;

			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!this->loadParam(parser, light->getEmitter()))
				break;

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;

			elementsFlag |= BVHSCENE_LIGHT_PARAM_EMITTER;
		}
		else
		{
			break;
		}
	}

	return 0;
}

int BVHScene::loadObject(NMLParser &parser, BVHObject *object, char *name, int standalone)
{
	NMLParser_token token;
	char filename[NMLPARSER_TOKEN_LEN];
	char path[NMLPARSER_TOKEN_LEN];
	unsigned char elementsFlag = 0x00;

	if (!parser.getSeparator(NMLPARSER_OPEN_BRACKET, token))
		return 0;

	// Get parser path
	getPath(parser.getFilename(), path);

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
			return ((elementsFlag & BVHSCENE_OBJECT_MUST) == BVHSCENE_OBJECT_MUST);

		if (strcmp(token.value, "name") == 0)
		{
			if (elementsFlag & BVHSCENE_OBJECT_NAME)
				break;

			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!parser.getWord(token))
				break;

			strcpy(name, token.value);

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;

			elementsFlag |= BVHSCENE_OBJECT_NAME;
		}
		else if (strcmp(token.value, "mesh") == 0)
		{
			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!parser.getFile(filename))
				break;

			// Add path
			addPath(filename, path);

			if (!BVHMesh::load(filename, object->primitives))
				break;

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;

			elementsFlag |= BVHSCENE_OBJECT_MESH;
		}
		else if (strcmp(token.value, "shader") == 0)
		{
			if (elementsFlag & BVHSCENE_OBJECT_SHADER)
				break;

			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!parser.getFile(filename))
				break;

			if (!object->shader.load(filename))
				break;

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;

			elementsFlag |= BVHSCENE_OBJECT_SHADER;
		}
		else if (strcmp(token.value, "param") == 0)
		{
			if (!(elementsFlag & BVHSCENE_OBJECT_SHADER))
				break;

			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!this->loadParam(parser, &object->shader))
				break;

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;

			elementsFlag |= BVHSCENE_OBJECT_PARAM;
		}
		else if (strcmp(token.value, "material-ref") == 0)
		{
			if (elementsFlag & BVHSCENE_OBJECT_MATERIAL)
				break;

			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!parser.getWord(token))
				break;

			// Look for the material in the list
			BVHShader *material = this->materials.get(token.value);

			if (material == NULL)
				break;

			// Add reference to the material in the object
			object->material = material;

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;

			elementsFlag |= BVHSCENE_OBJECT_MATERIAL;
		}
		else if (strcmp(token.value, "material-src") == 0)
		{
			if (elementsFlag & BVHSCENE_OBJECT_MATERIAL)
				break;

			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!parser.getFile(filename))
				break;

			// Load material from file
			NMLParser parserMaterial;
			BVHShader *material = new BVHShader;
			char nameMaterial[NMLPARSER_TOKEN_LEN];

			if (!parserMaterial.open(filename))
				break;

			if (!this->loadShader(parserMaterial, material, nameMaterial, 1))
			{
				delete material;
				parserMaterial.close();
				break;
			}

			parserMaterial.close();

			// Add material to the list
			if (!this->materials.add(nameMaterial, *material))
				break;

			// Look for the material in the list
			if ((material = this->materials.get(nameMaterial)) == NULL)
				break;

			// Add reference to the material in the object
			object->material = material;

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;

			elementsFlag |= BVHSCENE_OBJECT_MATERIAL;
		}
		else if (strcmp(token.value, "material") == 0)
		{
			if (elementsFlag & BVHSCENE_OBJECT_MATERIAL)
				break;

			BVHShader *material = new BVHShader;
			char nameMaterial[NMLPARSER_TOKEN_LEN];

			if (!this->loadShader(parser, material, nameMaterial, 0))
			{
				delete material;
				break;
			}

			// Add material to the list
			if (!this->materials.add(nameMaterial, *material))
				break;

			// Look for the material in the list
			if ((material = this->materials.get(nameMaterial)) == NULL)
				break;

			// Add reference to the material in the object
			object->material = material;

			elementsFlag |= BVHSCENE_OBJECT_MATERIAL;
		}
		else
		{
			break;
		}
	}

	return 0;
}

int BVHScene::loadTransform(NMLParser &parser, BVHTransform transform)
{
	NMLParser_token token;
	char filename[NMLPARSER_TOKEN_LEN];

	if (!parser.getSeparator(NMLPARSER_OPEN_BRACKET, token))
		return 0;

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

		if (strcmp(token.value, "translate") == 0)
		{
			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			BVHTransform *translate = new BVHTransform;

			if (!this->loadTranslate(parser, translate))
			{
				delete translate;
				break;
			}

			// Apply translation
			transform.transform(*translate);

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;
		}
		else if (strcmp(token.value, "rotate") == 0)
		{
			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			BVHTransform *rotate = new BVHTransform;

			if (!this->loadRotate(parser, rotate))
			{
				delete rotate;
				break;
			}

			// Apply rotation
			transform.transform(*rotate);

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;
		}
		else if (strcmp(token.value, "scale") == 0)
		{
			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			BVHTransform *scale = new BVHTransform;

			if (!this->loadScale(parser, scale))
			{
				delete scale;
				break;
			}

			// Apply scale
			transform.transform(*scale);

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;
		}
		else if (strcmp(token.value, "lookat-obj") == 0)
		{
			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			BVHTransform *lookat = new BVHTransform;

			if (!this->loadLookAtObj(parser, lookat))
			{
				delete lookat;
				break;
			}

			// Apply transform
			transform.transform(*lookat);

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;
		}
		else if (strcmp(token.value, "lookat-dir") == 0)
		{
			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			BVHTransform *lookat = new BVHTransform;

			if (!this->loadLookAtDir(parser, lookat))
			{
				delete lookat;
				break;
			}

			// Apply transform
			transform.transform(*lookat);

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;
		}
		else if (strcmp(token.value, "object-ref") == 0)
		{
			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!parser.getWord(token))
				break;

			// Look for the object in the list
			BVHObject *object = this->objects.get(token.value);

			if (object == NULL)
				break;

			// Add object reference
			BVHObjectRef *objectRef = new BVHObjectRef;
			objectRef->object = object;
			objectRef->transform = transform;
			this->objectRefs.push_back(*objectRef);

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;
		}
		else if (strcmp(token.value, "object-src") == 0)
		{
			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!parser.getFile(filename))
				break;

			// Load object from file
			NMLParser parserObject;
			BVHObject *object = new BVHObject;
			char nameObject[NMLPARSER_TOKEN_LEN];

			if (!parserObject.open(filename))
				break;

			if (!this->loadObject(parserObject, object, nameObject, 1))
			{
				delete object;
				parserObject.close();
				break;
			}

			parserObject.close();

			// Add object to the list
			if (!this->objects.add(nameObject, *object))
				break;

			// Look for the object in the list
			if ((object = this->objects.get(nameObject)) == NULL)
				break;

			// Add object reference
			BVHObjectRef *objectRef = new BVHObjectRef;
			objectRef->object = object;
			objectRef->transform = transform;
			this->objectRefs.push_back(*objectRef);

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;
		}
		else if (strcmp(token.value, "object") == 0)
		{
			BVHObject *object = new BVHObject;
			char nameObject[NMLPARSER_TOKEN_LEN];

			if (!this->loadObject(parser, object, nameObject, 0))
			{
				delete object;
				break;
			}

			// Add object to the list
			if (!this->objects.add(nameObject, *object))
				break;

			// Look for the object in the list
			if ((object = this->objects.get(nameObject)) == NULL)
				break;

			// Add object reference
			BVHObjectRef *objectRef = new BVHObjectRef;
			objectRef->object = object;
			objectRef->transform = transform;
			this->objectRefs.push_back(*objectRef);
		}
		else if (strcmp(token.value, "light-ref") == 0)
		{
			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!parser.getWord(token))
				break;

			// Look for the light in the list
			BVHLight *light = this->lights.get(token.value);

			if (light == NULL)
				break;

			// Add light reference
			BVHLightRef *lightRef = new BVHLightRef;
			lightRef->light = light;
			lightRef->transform = transform;
			this->lightRefs.push_back(*lightRef);

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;
		}
		else if (strcmp(token.value, "light-src") == 0)
		{
			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!parser.getFile(filename))
				break;

			// Load light from file
			NMLParser parserLight;
			BVHLight *light = new BVHLight;
			char nameLight[NMLPARSER_TOKEN_LEN];

			if (!parserLight.open(filename))
				break;

			if (!this->loadLight(parserLight, light, nameLight, 1))
			{
				delete light;
				parserLight.close();
				break;
			}

			parserLight.close();

			// Add light to the list
			if (!this->lights.add(nameLight, *light))
				break;

			// Look for the light in the list
			if ((light = this->lights.get(nameLight)) == NULL)
				break;

			// Add light reference
			BVHLightRef *lightRef = new BVHLightRef;
			lightRef->light = light;
			lightRef->transform = transform;
			this->lightRefs.push_back(*lightRef);

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;
		}
		else if (strcmp(token.value, "light") == 0)
		{
			BVHLight *light = new BVHLight;
			char nameLight[NMLPARSER_TOKEN_LEN];

			if (!this->loadLight(parser, light, nameLight, 0))
			{
				delete light;
				break;
			}

			// Add light to the list
			if (!this->lights.add(nameLight, *light))
				break;

			// Look for the light in the list
			if ((light = this->lights.get(nameLight)) == NULL)
				break;

			// Add light reference
			BVHLightRef *lightRef = new BVHLightRef;
			lightRef->light = light;
			lightRef->transform = transform;
			this->lightRefs.push_back(*lightRef);
		}
		else if (strcmp(token.value, "camera-ref") == 0)
		{
			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!parser.getWord(token))
				break;

			// Look for the camera in the list
			BVHCamera *camera = this->cameras.get(token.value);

			if (camera == NULL)
				break;

			// Add camera reference
			BVHCameraRef *cameraRef = new BVHCameraRef;
			cameraRef->camera = camera;
			cameraRef->transform = transform;
			this->cameraRefs.push_back(*cameraRef);
			cameraCurrent = &this->cameraRefs.back();

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;
		}
		else if (strcmp(token.value, "camera-src") == 0)
		{
			if (!parser.getSeparator(NMLPARSER_COLON, token))
				break;

			if (!parser.getFile(filename))
				break;

			// Load camera from file
			NMLParser parserCamera;
			BVHCamera *camera = new BVHCamera;
			char nameCamera[NMLPARSER_TOKEN_LEN];

			if (!parserCamera.open(filename))
				break;

			if (!this->loadCamera(parserCamera, camera, nameCamera, 1))
			{
				delete camera;
				parserCamera.close();
				break;
			}

			parserCamera.close();

			// Add camera to the list
			if (!this->cameras.add(nameCamera, *camera))
				break;

			// Look for the camera in the list
			if ((camera = this->cameras.get(nameCamera)) == NULL)
				break;

			// Add camera reference
			BVHCameraRef *cameraRef = new BVHCameraRef;
			cameraRef->camera = camera;
			cameraRef->transform = transform;
			this->cameraRefs.push_back(*cameraRef);

			if (!parser.getSeparator(NMLPARSER_SEMICOLON, token))
				break;
		}
		else if (strcmp(token.value, "camera") == 0)
		{
			BVHCamera *camera = new BVHCamera;
			char nameCamera[NMLPARSER_TOKEN_LEN];

			if (!this->loadCamera(parser, camera, nameCamera, 0))
			{
				delete camera;
				break;
			}

			// Add camera to the list
			if (!this->cameras.add(nameCamera, *camera))
				break;

			// Look for the camera in the list
			if ((camera = this->cameras.get(nameCamera)) == NULL)
				break;

			// Add camera reference
			BVHCameraRef *cameraRef = new BVHCameraRef;
			cameraRef->camera = camera;
			cameraRef->transform = transform;
			this->cameraRefs.push_back(*cameraRef);
		}
		else if (strcmp(token.value, "transform") == 0)
		{
			if (!this->loadTransform(parser, transform))
				break;
		}
		else
		{
			break;
		}
	}

	return 0;
}


	/* Load params */

int BVHScene::loadColor(NMLParser &parser, BVHColor *color)
{
	NMLParser_token token;
	NMLParser_token tokenKey;

	if (!parser.getWord(tokenKey))
		return 0;

	if (!parser.getSeparator(NMLPARSER_OPEN_PARANTHESES, token))
		return 0;

	if (strcmp(tokenKey.value, "color-intensity") == 0)
	{
		if (this->loadColorIntensity(parser, color))
			return 0;
	}
	else if (strcmp(tokenKey.value, "color-rgb") == 0)
	{
		if (this->loadColorRGB(parser, color))
			return 0;
	}
	else if (strcmp(tokenKey.value, "color-temp") == 0)
	{
		if (this->loadColorTemp(parser, color))
			return 0;
	}
	else if (strcmp(tokenKey.value, "color-spectrum") == 0)
	{
		if (this->loadColorSpectrum(parser, color))
			return 0;
	}
	else
	{
		return 0;
	}

	if (!parser.getSeparator(NMLPARSER_CLOSE_PARANTHESES, token))
		return 0;

	return 1;
}

int BVHScene::loadColorIntensity(NMLParser &parser, BVHColor *color)
{
	NMLParser_token token;
	float value;

	if (!parser.getWord(token))
		return 0;

	value = atof(token.value);
	color->init(BVHCOLOR_INIT_INTENSITY, &value);

	return 1;
}

int BVHScene::loadColorRGB(NMLParser &parser, BVHColor *color)
{
	NMLParser_token tokenValues[3];
	float values[3];

	if (!parser.getWord(tokenValues[0]) ||
		!parser.getWord(tokenValues[1]) ||
		!parser.getWord(tokenValues[2]))
		return 0;

	values[0] = atof(tokenValues[0].value);
	values[1] = atof(tokenValues[1].value);
	values[2] = atof(tokenValues[2].value);
	color->init(BVHCOLOR_INIT_RGB, values);

	return 1;
}

int BVHScene::loadColorTemp(NMLParser &parser, BVHColor *color)
{
	NMLParser_token tokenValues[2];
	float values[2];

	if (!parser.getWord(tokenValues[0]) ||
		!parser.getWord(tokenValues[1]))
		return 0;

	values[0] = atof(tokenValues[0].value);
	values[1] = atof(tokenValues[1].value);
	color->init(BVHCOLOR_INIT_TEMP, values);
	*color = *color * (values[1] / color->getIntensity());

	return 1;
}

int BVHScene::loadColorSpectrum(NMLParser &parser, BVHColor *color)
{
	NMLParser_token tokenValues[2];
	BVHColor_spectrum spectrum;

	if (!parser.getWord(tokenValues[0]))
		return 0;

	spectrum.nComponents = atoi(tokenValues[0].value);
	spectrum.components = new BVHColor_component[spectrum.nComponents];

	for (int i = 0; i < spectrum.nComponents; i++)
	{
		if (!parser.getWord(tokenValues[0]) ||
			!parser.getWord(tokenValues[1]))
		{
			delete [] spectrum.components;
			return 0;
		}

		spectrum.components[i].freq = atof(tokenValues[0].value) * 1E-9;
		spectrum.components[i].value = atof(tokenValues[1].value);

		// Check frequency
		if (i > 0 && spectrum.components[i].freq <= spectrum.components[i - 1].freq)
		{
			delete [] spectrum.components;
			return 0;
		}
	}

	color->init(BVHCOLOR_INIT_SPECTRUM, (void*)&spectrum);
	delete [] spectrum.components;
	return 1;
}

int BVHScene::loadTranslate(NMLParser &parser, BVHTransform *transform)
{
	NMLParser_token token;
	NMLParser_token tokenKey;
	NMLParser_token tokenValues[3];
	float position[3];

	transform->identity();

	if (!parser.getWord(tokenKey))
		return 0;

	if (strcmp(tokenKey.value, "vector") != 0)
		return 0;

	if (!parser.getSeparator(NMLPARSER_OPEN_PARANTHESES, token))
		return 0;

	if (!parser.getWord(tokenValues[0]) ||
		!parser.getWord(tokenValues[1]) ||
		!parser.getWord(tokenValues[2]))
		return 0;

	position[0] = atof(tokenValues[0].value);
	position[1] = atof(tokenValues[1].value);
	position[2] = atof(tokenValues[2].value);
	transform->translate(position);

	if (!parser.getSeparator(NMLPARSER_CLOSE_PARANTHESES, token))
		return 0;

	return 1;
}

int BVHScene::loadRotate(NMLParser &parser, BVHTransform *transform)
{
	NMLParser_token token;
	NMLParser_token tokenKey;
	NMLParser_token tokenValues[9];
	float values[9];

	transform->identity();

	if (!parser.getWord(tokenKey))
		return 0;

	if (!parser.getSeparator(NMLPARSER_OPEN_PARANTHESES, token))
		return 0;

	if (strcmp(tokenKey.value, "eulerxyz") == 0)
	{
		if (!parser.getWord(tokenValues[0]) ||
			!parser.getWord(tokenValues[1]) ||
			!parser.getWord(tokenValues[2]))
			return 0;

		values[0] = atof(tokenValues[0].value) * DEG2RAD;
		values[1] = atof(tokenValues[1].value) * DEG2RAD;
		values[2] = atof(tokenValues[2].value) * DEG2RAD;
		transform->rotate_euler(values, MATH_EULER_XYZ);
	}
	else if (strcmp(tokenKey.value, "eulerxzy") == 0)
	{
		if (!parser.getWord(tokenValues[0]) ||
			!parser.getWord(tokenValues[1]) ||
			!parser.getWord(tokenValues[2]))
			return 0;

		values[0] = atof(tokenValues[0].value) * DEG2RAD;
		values[1] = atof(tokenValues[1].value) * DEG2RAD;
		values[2] = atof(tokenValues[2].value) * DEG2RAD;
		transform->rotate_euler(values, MATH_EULER_XZY);
	}
	else if (strcmp(tokenKey.value, "euleryxz") == 0)
	{
		if (!parser.getWord(tokenValues[0]) ||
			!parser.getWord(tokenValues[1]) ||
			!parser.getWord(tokenValues[2]))
			return 0;

		values[0] = atof(tokenValues[0].value) * DEG2RAD;
		values[1] = atof(tokenValues[1].value) * DEG2RAD;
		values[2] = atof(tokenValues[2].value) * DEG2RAD;
		transform->rotate_euler(values, MATH_EULER_YXZ);
	}
	else if (strcmp(tokenKey.value, "euleryzx") == 0)
	{
		if (!parser.getWord(tokenValues[0]) ||
			!parser.getWord(tokenValues[1]) ||
			!parser.getWord(tokenValues[2]))
			return 0;

		values[0] = atof(tokenValues[0].value) * DEG2RAD;
		values[1] = atof(tokenValues[1].value) * DEG2RAD;
		values[2] = atof(tokenValues[2].value) * DEG2RAD;
		transform->rotate_euler(values, MATH_EULER_YZX);
	}
	else if (strcmp(tokenKey.value, "eulerzxy") == 0)
	{
		if (!parser.getWord(tokenValues[0]) ||
			!parser.getWord(tokenValues[1]) ||
			!parser.getWord(tokenValues[2]))
			return 0;

		values[0] = atof(tokenValues[0].value) * DEG2RAD;
		values[1] = atof(tokenValues[1].value) * DEG2RAD;
		values[2] = atof(tokenValues[2].value) * DEG2RAD;
		transform->rotate_euler(values, MATH_EULER_ZXY);
	}
	else if (strcmp(tokenKey.value, "eulerzyx") == 0)
	{
		if (!parser.getWord(tokenValues[0]) ||
			!parser.getWord(tokenValues[1]) ||
			!parser.getWord(tokenValues[2]))
			return 0;

		values[0] = atof(tokenValues[0].value) * DEG2RAD;
		values[1] = atof(tokenValues[1].value) * DEG2RAD;
		values[2] = atof(tokenValues[2].value) * DEG2RAD;
		transform->rotate_euler(values, MATH_EULER_ZYX);
	}
	else if (strcmp(tokenKey.value, "angle-axis") == 0)
	{
		if (!parser.getWord(tokenValues[0]) ||
			!parser.getWord(tokenValues[1]) ||
			!parser.getWord(tokenValues[2]) ||
			!parser.getWord(tokenValues[3]))
			return 0;

		values[0] = atof(tokenValues[0].value) * DEG2RAD;
		values[1] = atof(tokenValues[1].value);
		values[2] = atof(tokenValues[2].value);
		values[3] = atof(tokenValues[3].value);
		transform->rotate_angleAxis(values);
	}
	else if (strcmp(tokenKey.value, "quat") == 0)
	{
		if (!parser.getWord(tokenValues[0]) ||
			!parser.getWord(tokenValues[1]) ||
			!parser.getWord(tokenValues[2]) ||
			!parser.getWord(tokenValues[3]))
			return 0;

		values[0] = atof(tokenValues[0].value);
		values[1] = atof(tokenValues[1].value);
		values[2] = atof(tokenValues[2].value);
		values[3] = atof(tokenValues[3].value);
		transform->rotate_quat(values);
	}
	else if (strcmp(tokenKey.value, "matrix") == 0)
	{
		if (!parser.getWord(tokenValues[0]) ||
			!parser.getWord(tokenValues[1]) ||
			!parser.getWord(tokenValues[2]) ||
			!parser.getWord(tokenValues[3]) ||
			!parser.getWord(tokenValues[4]) ||
			!parser.getWord(tokenValues[5]) ||
			!parser.getWord(tokenValues[6]) ||
			!parser.getWord(tokenValues[7]) ||
			!parser.getWord(tokenValues[8]))
			return 0;

		values[0] = atof(tokenValues[0].value);
		values[1] = atof(tokenValues[1].value);
		values[2] = atof(tokenValues[2].value);
		values[3] = atof(tokenValues[3].value);
		values[4] = atof(tokenValues[4].value);
		values[5] = atof(tokenValues[5].value);
		values[6] = atof(tokenValues[6].value);
		values[7] = atof(tokenValues[7].value);
		values[8] = atof(tokenValues[8].value);
		transform->rotate_matrix(values);
	}
	else
	{
		return 0;
	}

	if (!parser.getSeparator(NMLPARSER_CLOSE_PARANTHESES, token))
		return 0;

	return 1;
}

int BVHScene::loadScale(NMLParser &parser, BVHTransform *transform)
{
	NMLParser_token token;
	NMLParser_token tokenKey;
	NMLParser_token tokenValues[3];
	float values[3];

	transform->identity();

	if (!parser.getWord(tokenKey))
		return 0;

	if (!parser.getSeparator(NMLPARSER_OPEN_PARANTHESES, token))
		return 0;

	if (strcmp(tokenKey.value, "vector") == 0)
	{
		if (!parser.getWord(tokenValues[0]) ||
			!parser.getWord(tokenValues[1]) ||
			!parser.getWord(tokenValues[2]))
			return 0;

		values[0] = atof(tokenValues[0].value);
		values[1] = atof(tokenValues[1].value);
		values[2] = atof(tokenValues[2].value);
		transform->scale(values);
	}
	else if (strcmp(tokenKey.value, "float") == 0)
	{
		if (!parser.getWord(tokenValues[0]))
			return 0;

		values[0] = atof(tokenValues[0].value);
		values[1] = atof(tokenValues[0].value);
		values[2] = atof(tokenValues[0].value);
		transform->scale(values);
	}
	else
	{
		return 0;
	}

	if (!parser.getSeparator(NMLPARSER_CLOSE_PARANTHESES, token))
		return 0;

	return 1;
}

int BVHScene::loadLookAtObj(NMLParser &parser, BVHTransform *transform)
{
	NMLParser_token token;
	NMLParser_token tokenKey;
	NMLParser_token tokenValues[3];
	float obj[3];
	float up[3];

	transform->identity();

	if (!parser.getWord(tokenKey))
		return 0;

	if (strcmp(tokenKey.value, "vector") != 0)
		return 0;

	if (!parser.getSeparator(NMLPARSER_OPEN_PARANTHESES, token))
		return 0;

	if (!parser.getWord(tokenValues[0]) ||
		!parser.getWord(tokenValues[1]) ||
		!parser.getWord(tokenValues[2]))
		return 0;

	obj[0] = atof(tokenValues[0].value);
	obj[1] = atof(tokenValues[1].value);
	obj[2] = atof(tokenValues[2].value);

	if (!parser.getSeparator(NMLPARSER_CLOSE_PARANTHESES, token))
		return 0;

	if (!parser.getWord(tokenKey))
		return 0;

	if (strcmp(tokenKey.value, "vector") != 0)
		return 0;

	if (!parser.getSeparator(NMLPARSER_OPEN_PARANTHESES, token))
		return 0;

	if (!parser.getWord(tokenValues[0]) ||
		!parser.getWord(tokenValues[1]) ||
		!parser.getWord(tokenValues[2]))
		return 0;

	up[0] = atof(tokenValues[0].value);
	up[1] = atof(tokenValues[1].value);
	up[2] = atof(tokenValues[2].value);

	if (!parser.getSeparator(NMLPARSER_CLOSE_PARANTHESES, token))
		return 0;

	transform->lookat_obj(obj, up);

	return 1;
}

int BVHScene::loadLookAtDir(NMLParser &parser, BVHTransform *transform)
{
	NMLParser_token token;
	NMLParser_token tokenKey;
	NMLParser_token tokenValues[3];
	float dir[3];
	float up[3];

	transform->identity();

	if (!parser.getWord(tokenKey))
		return 0;

	if (strcmp(tokenKey.value, "vector") != 0)
		return 0;

	if (!parser.getSeparator(NMLPARSER_OPEN_PARANTHESES, token))
		return 0;

	if (!parser.getWord(tokenValues[0]) ||
		!parser.getWord(tokenValues[1]) ||
		!parser.getWord(tokenValues[2]))
		return 0;

	dir[0] = atof(tokenValues[0].value);
	dir[1] = atof(tokenValues[1].value);
	dir[2] = atof(tokenValues[2].value);

	if (!parser.getSeparator(NMLPARSER_CLOSE_PARANTHESES, token))
		return 0;

	if (!parser.getWord(tokenKey))
		return 0;

	if (strcmp(tokenKey.value, "vector") != 0)
		return 0;

	if (!parser.getSeparator(NMLPARSER_OPEN_PARANTHESES, token))
		return 0;

	if (!parser.getWord(tokenValues[0]) ||
		!parser.getWord(tokenValues[1]) ||
		!parser.getWord(tokenValues[2]))
		return 0;

	up[0] = atof(tokenValues[0].value);
	up[1] = atof(tokenValues[1].value);
	up[2] = atof(tokenValues[2].value);

	if (!parser.getSeparator(NMLPARSER_CLOSE_PARANTHESES, token))
		return 0;

	transform->lookat_dir(dir, up);

	return 1;
}

int BVHScene::loadParam(NMLParser &parser, BVHShader *shader)
{
	NMLParser_token token;
	NMLParser_token tokenName;
	NMLParser_token tokenKey;
	int type = -1;	// 0 int, 1 float
	int size = 0;

	if (!parser.getWord(tokenName))
		return 0;

	if (!parser.getWord(tokenKey))
		return 0;

	if (!parser.getSeparator(NMLPARSER_OPEN_PARANTHESES, token))
		return 0;

	if (strcmp(tokenKey.value, "material") == 0)
	{
		if (!parser.getWord(token))
			return 0;

		BVHShader *material = this->materials.get(token.value);

		if (material == NULL)
			return 0;

		shader->setParam(tokenName.value, (void*)material);
	}
	else if (strcmp(tokenKey.value, "texture") == 0)
	{
		if (!parser.getWord(token))
			return 0;

		BVHTexture *texture = this->textures.get(token.value);

		if (texture == NULL)
			return 0;

		shader->setParam(tokenName.value, (void*)texture);
	}
	else if (strcmp(tokenKey.value, "shader-texture") == 0)
	{
		if (!parser.getWord(token))
			return 0;

		BVHShader *shaderTexture = this->shaderTextures.get(token.value);

		if (shaderTexture == NULL)
			return 0;

		shader->setParam(tokenName.value, (void*)shaderTexture);
	}
	else if (strcmp(tokenKey.value, "shader-mask") == 0)
	{
		if (!parser.getWord(token))
			return 0;

		BVHShader *shaderMask = this->shaderMasks.get(token.value);

		if (shaderMask == NULL)
			return 0;

		shader->setParam(tokenName.value, (void*)shaderMask);
	}
	else if (strcmp(tokenKey.value, "color-intensity") == 0)
	{
		BVHColor color;

		if (!this->loadColorIntensity(parser, &color))
			return 0;

		shader->setParam(tokenName.value, (void*)&color);
	}
	else if (strcmp(tokenKey.value, "color-rgb") == 0)
	{
		BVHColor color;

		if (!this->loadColorRGB(parser, &color))
			return 0;

		shader->setParam(tokenName.value, (void*)&color);
	}
	else if (strcmp(tokenKey.value, "color-temp") == 0)
	{
		BVHColor color;

		if (!this->loadColorTemp(parser, &color))
			return 0;

		shader->setParam(tokenName.value, (void*)&color);
	}
	else if (strcmp(tokenKey.value, "color-spectrum") == 0)
	{
		BVHColor color;

		if (!this->loadColorSpectrum(parser, &color))
			return 0;

		shader->setParam(tokenName.value, (void*)&color);
	}
	else
	{
		if (strcmp(tokenKey.value, "int") == 0)			{ type = 0; size = 1;  }
		else if (strcmp(tokenKey.value, "ivec2") == 0)	{ type = 0; size = 2;  }
		else if (strcmp(tokenKey.value, "ivec3") == 0)	{ type = 0; size = 3;  }
		else if (strcmp(tokenKey.value, "ivec4") == 0)	{ type = 0; size = 4;  }
		else if (strcmp(tokenKey.value, "float") == 0)	{ type = 1; size = 1;  }
		else if (strcmp(tokenKey.value, "vec2") == 0)	{ type = 1; size = 2;  }
		else if (strcmp(tokenKey.value, "vec3") == 0)	{ type = 1; size = 3;  }
		else if (strcmp(tokenKey.value, "vec4") == 0)	{ type = 1; size = 4;  }
		else if (strcmp(tokenKey.value, "mat2") == 0)	{ type = 1; size = 4;  }
		else if (strcmp(tokenKey.value, "mat2x2") == 0)	{ type = 1; size = 4;  }
		else if (strcmp(tokenKey.value, "mat2x3") == 0)	{ type = 1; size = 6;  }
		else if (strcmp(tokenKey.value, "mat2x4") == 0)	{ type = 1; size = 8;  }
		else if (strcmp(tokenKey.value, "mat3x2") == 0)	{ type = 1; size = 6;  }
		else if (strcmp(tokenKey.value, "mat3") == 0)	{ type = 1; size = 9;  }
		else if (strcmp(tokenKey.value, "mat3x3") == 0)	{ type = 1; size = 9;  }
		else if (strcmp(tokenKey.value, "mat3x4") == 0)	{ type = 1; size = 12; }
		else if (strcmp(tokenKey.value, "mat4x2") == 0)	{ type = 1; size = 8;  }
		else if (strcmp(tokenKey.value, "mat4x3") == 0)	{ type = 1; size = 12; }
		else if (strcmp(tokenKey.value, "mat4") == 0)	{ type = 1; size = 16; }
		else if (strcmp(tokenKey.value, "mat4x4") == 0)	{ type = 1; size = 16; }
		else											{ return 0;            }

		if (type == 0)
		{
			int *values = new int[size];

			for (int i = 0; i < size; i++)
			{
				if (!parser.getWord(token))
				{
					delete [] values;
					return 0;
				}

				values[i] = atoi(token.value);
			}

			shader->setParam(tokenName.value, (void*)values);
			delete [] values;
		}
		else if (type == 1)
		{
			float *values = new float[size];

			for (int i = 0; i < size; i++)
			{
				if (!parser.getWord(token))
				{
					delete [] values;
					return 0;
				}

				values[i] = atof(token.value);
			}

			shader->setParam(tokenName.value, (void*)values);
			delete [] values;
		}
	}

	if (!parser.getSeparator(NMLPARSER_CLOSE_PARANTHESES, token))
		return 0;

	return 1;
}


	/* Generate final primitive list */

void BVHScene::transformPrimitives()
{
	// Variables
	std::list<BVHObjectRef>::iterator objectRefIt;
	std::list<BVHPrimitive>::iterator primitiveIt;
	BVHPrimitive *primitive;
	BVHShader_vertex vertex;
	char *newAttribBuffer;

	// Iterate in the object referencies
	for (objectRefIt = this->objectRefs.begin(); objectRefIt != this->objectRefs.end(); objectRefIt++)
	{
		// Iterate in the object primitives
		for (primitiveIt = (*objectRefIt).object->primitives.begin(); primitiveIt != (*objectRefIt).object->primitives.end(); primitiveIt++)
		{
			// Transform attributes with the vertex shader
			vertex.transform = (*objectRefIt).transform;
			vertex.attribAtlas = (*primitiveIt).getAttribAtlas();
			newAttribBuffer = new char[(*primitiveIt).getAttribAtlas()->size * (*primitiveIt).getAttribNumber()];

			for (int i = 0; i < (*primitiveIt).getAttribNumber(); i++)
			{
				vertex.attribBuffer = (*primitiveIt).getAttribBuffer() + i * (*primitiveIt).getAttribAtlas()->size;
				vertex.newAttribBuffer = newAttribBuffer + i * (*primitiveIt).getAttribAtlas()->size;
				(*objectRefIt).object->shader.execute(&vertex);

				// Asign material (for the light references)
				// TODO: Fix it
				(*primitiveIt).setMaterial((*objectRefIt).object->material);
			}

			// Create new primitive
			primitive = new BVHPrimitive;
			primitive->init(
				(*primitiveIt).getType(),
				(*primitiveIt).getAttribAtlas(),
				(*primitiveIt).getAttribNumber(),
				newAttribBuffer);
			primitive->setMaterial((*objectRefIt).object->material);

			// Delete buffer
			delete [] newAttribBuffer;

			// Add primitive to list
			this->primitives.push_back(*primitive);
		}
	}
}
