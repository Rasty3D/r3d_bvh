/*
 * bvhgpu.cpp
 *
 *  Created on: 04 Feb 2011
 *      Author: showroom
 */


/*
 * INCLUDES
 */

#include "bvhgpu.h"
#include "bvhscene.h"


/*
 * NAMESPACES
 */

using namespace std;


/*
 * FUNCTIONS
 */

int BVHgpu_primitiveIntersection(BVHgpu_ray *ray, unsigned char *primitive)
{
	BVHgpu_primitiveHeader *header = (BVHgpu_primitiveHeader*)primitive;
	unsigned char *data = primitive + sizeof(BVHgpu_primitiveHeader);

	if (header->type == BVHPRIMITIVE_TYPE_TRIANGLE)
	{
		// Variables
		float t;
		float pi[3];
		BVHPrimitive_triangle *triangle;

		// Get triangle data
		triangle = (BVHPrimitive_triangle*)data;

		// Calculate t
		t = ((triangle->v1[0] - ray->pos[0]) * triangle->normal[0] +
			 (triangle->v1[1] - ray->pos[1]) * triangle->normal[1] +
			 (triangle->v1[2] - ray->pos[2]) * triangle->normal[2]) /
			(ray->dir[0] * triangle->normal[0] +
			 ray->dir[1] * triangle->normal[1] +
			 ray->dir[2] * triangle->normal[2]);

		// Check direction
		if (t < 0.001f)
			return 0;

		if (ray->distance != -1.0f && t > ray->distance)
			return 0;

		// Check intersection
		pi[0] = ray->pos[0] + ray->dir[0] * t;
		pi[1] = ray->pos[1] + ray->dir[1] * t;
		pi[2] = ray->pos[2] + ray->dir[2] * t;

		if (((pi[0] - triangle->v1[0]) * triangle->n12[0] +
			 (pi[1] - triangle->v1[1]) * triangle->n12[1] +
			 (pi[2] - triangle->v1[2]) * triangle->n12[2]) < 0.0f)
			return 0;

		if (((pi[0] - triangle->v2[0]) * triangle->n23[0] +
			 (pi[1] - triangle->v2[1]) * triangle->n23[1] +
			 (pi[2] - triangle->v2[2]) * triangle->n23[2]) < 0.0f)
			return 0;

		if (((pi[0] - triangle->v3[0]) * triangle->n31[0] +
			 (pi[1] - triangle->v3[1]) * triangle->n31[1] +
			 (pi[2] - triangle->v3[2]) * triangle->n31[2]) < 0.0f)
			return 0;

		ray->distance = t;
		return 1;
	}
	else
	{
		return 0;
	}
}

int BVHgpu_aabbIntersection(float *gpu_nodeMin, float *gpu_nodeMax, BVHgpu_ray *ray, float *dirInv)
{
	// Calculate intersection
	float tpos[3];
	float tneg[3];
	float tmax[3];
	float tmin[3];
	float lmin;
	float lmax;

	tpos[0] = (gpu_nodeMax[0] - ray->pos[0]) * dirInv[0];
	tpos[1] = (gpu_nodeMax[1] - ray->pos[1]) * dirInv[1];
	tpos[2] = (gpu_nodeMax[2] - ray->pos[2]) * dirInv[2];
	tneg[0] = (gpu_nodeMin[0] - ray->pos[0]) * dirInv[0];
	tneg[1] = (gpu_nodeMin[1] - ray->pos[1]) * dirInv[1];
	tneg[2] = (gpu_nodeMin[2] - ray->pos[2]) * dirInv[2];

	tmax[0] = (tpos[0] > tneg[0]) ? tpos[0] : tneg[0];
	tmax[1] = (tpos[1] > tneg[1]) ? tpos[1] : tneg[1];
	tmax[2] = (tpos[2] > tneg[2]) ? tpos[2] : tneg[2];
	tmin[0] = (tpos[0] < tneg[0]) ? tpos[0] : tneg[0];
	tmin[1] = (tpos[1] < tneg[1]) ? tpos[1] : tneg[1];
	tmin[2] = (tpos[2] < tneg[2]) ? tpos[2] : tneg[2];

	if (tmin[0] >= tmin[1] && tmin[0] >= tmin[2])
		lmin = tmin[0];
	else if (tmin[1] > tmin[0] && tmin[1] >= tmin[2])
		lmin = tmin[1];
	else
		lmin = tmin[2];

	if (tmax[0] <= tmax[1] && tmax[0] <= tmax[2])
		lmax = tmax[0];
	else if (tmax[1] < tmax[0] && tmax[1] <= tmax[2])
		lmax = tmax[1];
	else
		lmax = tmax[2];

	if (ray->distance == -1.0f)
		return lmax > lmin && lmax > 0.0f;
	else
		return lmax > lmin && lmax > 0.0f && lmin < ray->distance;
}

void BVHgpu_trace(
	unsigned int gpu_nNodes, float *gpu_nodesMin, float *gpu_nodesMax, unsigned int *gpu_nodesSkipLocation,
	BVHgpu_ray *ray, unsigned char *primitives)
{
	if (ray->distance == -2.0f)
		return;

	// Normalize ray
	float rayModule;
	unsigned int nodeIndex;

	rayModule = sqrtf(
		ray->dir[0] * ray->dir[0] +
		ray->dir[1] * ray->dir[1] +
		ray->dir[2] * ray->dir[2]);
	ray->dir[0] /= rayModule;
	ray->dir[1] /= rayModule;
	ray->dir[2] /= rayModule;

	// Inverse ray
	float dirInv[3];
	dirInv[0] = 1.0f / ray->dir[0];
	dirInv[1] = 1.0f / ray->dir[1];
	dirInv[2] = 1.0f / ray->dir[2];

	// Start from first node
	nodeIndex = 0;

	// Tree traversal
	while (nodeIndex < gpu_nNodes)
	{
		if (BVHgpu_aabbIntersection(&gpu_nodesMin[nodeIndex * 4], &gpu_nodesMax[nodeIndex * 4], ray, dirInv))
		{
			if (gpu_nodesSkipLocation[nodeIndex * 2 + 1] != BVHGPU_NOPRIMITIVE)
			{
				if (BVHgpu_primitiveIntersection(ray, &primitives[gpu_nodesSkipLocation[nodeIndex * 2 + 1]]))
					ray->closestNode = nodeIndex;
			}

			nodeIndex++;
		}
		else
		{
			nodeIndex = gpu_nodesSkipLocation[nodeIndex * 2];
		}
	}
}

BVHPrimitive *getPrimitiveInfo(
	float *pos, float *dir,
	unsigned int primitiveLocation, unsigned char *primitives,
	BVHShader_surface *info)
{
	// Check primitive location
	if (primitiveLocation == BVHGPU_NOPRIMITIVE)
		return NULL;

	// Variables
	BVHPrimitive *primitive = NULL;
	BVHPrimitive_triangle *triangle;
	BVHgpu_primitiveHeader *header;
	unsigned char *data;

	// Get primitive header
	header = (BVHgpu_primitiveHeader*)&primitives[primitiveLocation];

	// Get primitive data
	data = &primitives[primitiveLocation] + sizeof(BVHgpu_primitiveHeader);

	// Get primitive
	primitive = (BVHPrimitive*)header->primitive;

	// Process depending on the type
	if (header->type == BVHPRIMITIVE_TYPE_TRIANGLE)
	{
		// Get triangle data
		triangle = (BVHPrimitive_triangle*)data;

		// Calculate intersection point
		info->pos[0] = pos[0];
		info->pos[1] = pos[1];
		info->pos[2] = pos[2];

		// Calculate normal
		info->normal[0] = triangle->normal[0];
		info->normal[1] = triangle->normal[1];
		info->normal[2] = triangle->normal[2];

		// Invert normal if necessary
		if ((dir != NULL) &&
			(info->normal[0] * dir[0] + info->normal[1] * dir[1] + info->normal[2] * dir[2] > 0.0f))
		{
			info->normal[0] = -info->normal[0];
			info->normal[1] = -info->normal[1];
			info->normal[2] = -info->normal[2];
		}

		// Calculate primitive coordinates
		float v12[3] = {
			triangle->v2[0] - triangle->v1[0],
			triangle->v2[1] - triangle->v1[1],
			triangle->v2[2] - triangle->v1[2]};
		float v13[3] = {
			triangle->v3[0] - triangle->v1[0],
			triangle->v3[1] - triangle->v1[1],
			triangle->v3[2] - triangle->v1[2]};
		float v1p[3] = {
			info->pos[0] - triangle->v1[0],
			info->pos[1] - triangle->v1[1],
			info->pos[2] - triangle->v1[2]};

		info->primCoords[1] =
			(v1p[0] * triangle->n31[0] + v1p[1] * triangle->n31[1] + v1p[2] * triangle->n31[2]) /
			(v12[0] * triangle->n31[0] + v12[1] * triangle->n31[1] + v12[2] * triangle->n31[2]);
		info->primCoords[2] =
			(v1p[0] * triangle->n12[0] + v1p[1] * triangle->n12[1] + v1p[2] * triangle->n12[2]) /
			(v13[0] * triangle->n12[0] + v13[1] * triangle->n12[1] + v13[2] * triangle->n12[2]);
		info->primCoords[0] = 1.0f - info->primCoords[1] - info->primCoords[2];

		// Get attribs information
		info->attribAtlas = primitive->getAttribAtlas();

		// Resize buffer
		if (info->attribBuffer == NULL || info->attribBufferSize == 0)
		{
			info->attribBuffer = new char[info->attribAtlas->size];
			info->attribBufferSize = info->attribAtlas->size;
		}
		else if (info->attribBufferSize < info->attribAtlas->size)
		{
			delete [] info->attribBuffer;
			info->attribBuffer = new char[info->attribAtlas->size];
			info->attribBufferSize = info->attribAtlas->size;
		}

		// Interpolate attributes
		for (int i = 0; i < info->attribAtlas->number; i++)
		{
			if (info->attribAtlas->attrList[i].type == BVHSHADER_TYPE_INT8)
			{
				char *data1 = (char*)&(primitive->getAttribBuffer()[info->attribAtlas->attrList[i].offset]);
				char *data2 = (char*)&(primitive->getAttribBuffer()[info->attribAtlas->attrList[i].offset + info->attribAtlas->size]);
				char *data3 = (char*)&(primitive->getAttribBuffer()[info->attribAtlas->attrList[i].offset + info->attribAtlas->size * 2]);
				char *dataInfo = (char*)&(info->attribBuffer[info->attribAtlas->attrList[i].offset]);

				for (int j = 0; j < info->attribAtlas->attrList[i].length; j++)
				{
					dataInfo[j] =
						info->primCoords[0] * data1[j] +
						info->primCoords[1] * data2[j] +
						info->primCoords[2] * data3[j];
				}
			}
			else if (info->attribAtlas->attrList[i].type == BVHSHADER_TYPE_INT16)
			{
				short *data1 = (short*)&(primitive->getAttribBuffer()[info->attribAtlas->attrList[i].offset]);
				short *data2 = (short*)&(primitive->getAttribBuffer()[info->attribAtlas->attrList[i].offset + info->attribAtlas->size]);
				short *data3 = (short*)&(primitive->getAttribBuffer()[info->attribAtlas->attrList[i].offset + info->attribAtlas->size * 2]);
				short *dataInfo = (short*)&(info->attribBuffer[info->attribAtlas->attrList[i].offset]);

				for (int j = 0; j < info->attribAtlas->attrList[i].length; j++)
				{
					dataInfo[j] =
						info->primCoords[0] * data1[j] +
						info->primCoords[1] * data2[j] +
						info->primCoords[2] * data3[j];
				}
			}
			else if (info->attribAtlas->attrList[i].type == BVHSHADER_TYPE_INT32)
			{
				int *data1 = (int*)&(primitive->getAttribBuffer()[info->attribAtlas->attrList[i].offset]);
				int *data2 = (int*)&(primitive->getAttribBuffer()[info->attribAtlas->attrList[i].offset + info->attribAtlas->size]);
				int *data3 = (int*)&(primitive->getAttribBuffer()[info->attribAtlas->attrList[i].offset + info->attribAtlas->size * 2]);
				int *dataInfo = (int*)&(info->attribBuffer[info->attribAtlas->attrList[i].offset]);

				for (int j = 0; j < info->attribAtlas->attrList[i].length; j++)
				{
					dataInfo[j] =
						info->primCoords[0] * data1[j] +
						info->primCoords[1] * data2[j] +
						info->primCoords[2] * data3[j];
				}
			}
			else if (info->attribAtlas->attrList[i].type == BVHSHADER_TYPE_UINT8)
			{
				unsigned char *data1 = (unsigned char*)&(primitive->getAttribBuffer()[info->attribAtlas->attrList[i].offset]);
				unsigned char *data2 = (unsigned char*)&(primitive->getAttribBuffer()[info->attribAtlas->attrList[i].offset + info->attribAtlas->size]);
				unsigned char *data3 = (unsigned char*)&(primitive->getAttribBuffer()[info->attribAtlas->attrList[i].offset + info->attribAtlas->size * 2]);
				unsigned char *dataInfo = (unsigned char*)&(info->attribBuffer[info->attribAtlas->attrList[i].offset]);

				for (int j = 0; j < info->attribAtlas->attrList[i].length; j++)
				{
					dataInfo[j] =
						info->primCoords[0] * data1[j] +
						info->primCoords[1] * data2[j] +
						info->primCoords[2] * data3[j];
				}
			}
			else if (info->attribAtlas->attrList[i].type == BVHSHADER_TYPE_UINT16)
			{
				unsigned short *data1 = (unsigned short*)&(primitive->getAttribBuffer()[info->attribAtlas->attrList[i].offset]);
				unsigned short *data2 = (unsigned short*)&(primitive->getAttribBuffer()[info->attribAtlas->attrList[i].offset + info->attribAtlas->size]);
				unsigned short *data3 = (unsigned short*)&(primitive->getAttribBuffer()[info->attribAtlas->attrList[i].offset + info->attribAtlas->size * 2]);
				unsigned short *dataInfo = (unsigned short*)&(info->attribBuffer[info->attribAtlas->attrList[i].offset]);

				for (int j = 0; j < info->attribAtlas->attrList[i].length; j++)
				{
					dataInfo[j] =
						info->primCoords[0] * data1[j] +
						info->primCoords[1] * data2[j] +
						info->primCoords[2] * data3[j];
				}
			}
			else if (info->attribAtlas->attrList[i].type == BVHSHADER_TYPE_UINT32)
			{
				unsigned int *data1 = (unsigned int*)&(primitive->getAttribBuffer()[info->attribAtlas->attrList[i].offset]);
				unsigned int *data2 = (unsigned int*)&(primitive->getAttribBuffer()[info->attribAtlas->attrList[i].offset + info->attribAtlas->size]);
				unsigned int *data3 = (unsigned int*)&(primitive->getAttribBuffer()[info->attribAtlas->attrList[i].offset + info->attribAtlas->size * 2]);
				unsigned int *dataInfo = (unsigned int*)&(info->attribBuffer[info->attribAtlas->attrList[i].offset]);

				for (int j = 0; j < info->attribAtlas->attrList[i].length; j++)
				{
					dataInfo[j] =
						info->primCoords[0] * data1[j] +
						info->primCoords[1] * data2[j] +
						info->primCoords[2] * data3[j];
				}
			}
			else if (info->attribAtlas->attrList[i].type == BVHSHADER_TYPE_FLOAT)
			{
				float *data1 = (float*)&(primitive->getAttribBuffer()[info->attribAtlas->attrList[i].offset]);
				float *data2 = (float*)&(primitive->getAttribBuffer()[info->attribAtlas->attrList[i].offset + info->attribAtlas->size]);
				float *data3 = (float*)&(primitive->getAttribBuffer()[info->attribAtlas->attrList[i].offset + info->attribAtlas->size * 2]);
				float *dataInfo = (float*)&(info->attribBuffer[info->attribAtlas->attrList[i].offset]);

				for (int j = 0; j < info->attribAtlas->attrList[i].length; j++)
				{
					dataInfo[j] =
						info->primCoords[0] * data1[j] +
						info->primCoords[1] * data2[j] +
						info->primCoords[2] * data3[j];
				}
			}
		}
	}

	return primitive;
}

void getTangentBinormal(float *normal, float *tangent, float *binormal)
{
	normalize(normal);

	if (fabsf(normal[0]) >= fabsf(normal[1]) && fabsf(normal[0]) >= fabsf(normal[2]))
		initVector(tangent, -normal[2], 0.0f, normal[0]);
	else if (fabsf(normal[1]) > fabsf(normal[0]) && fabsf(normal[1]) >= fabsf(normal[2]))
		initVector(tangent, normal[1], -normal[0], 0.0f);
	else
		initVector(tangent, 0.0f, normal[2], -normal[1]);

	cross(tangent, normal, binormal);
	normalize(tangent);
	normalize(binormal);
}

void getRayDirectionDiffuse(
	gsl_rng *rng,
	float *normal, float *rayInput, float *rayOutput,
	float &pdf)
{
	float cosTheta = sqrtf(randFloat(rng, 0.0f, 1.0f));
	float sinTheta = sqrtf(1.0f - cosTheta * cosTheta);
	float phi = 2.0f * PI * randFloat(rng, 0.0f, 1.0f);
	float cosPhi = cosf(phi);
	float sinPhi = sinf(phi);
	float tangent[3];
	float binormal[3];
	float rayLocal[3];

	// Get tangent and binormal
	getTangentBinormal(normal, tangent, binormal);

	// Calculate ray in local coordinates
	rayLocal[0] = sinTheta * cosPhi;
	rayLocal[1] = sinTheta * sinPhi;
	rayLocal[2] = cosTheta;

	// Transform to global coordinates
	rayOutput[0] = tangent[0] * rayLocal[0] + binormal[0] * rayLocal[1] + normal[0] * rayLocal[2];
	rayOutput[1] = tangent[1] * rayLocal[0] + binormal[1] * rayLocal[1] + normal[1] * rayLocal[2];
	rayOutput[2] = tangent[2] * rayLocal[0] + binormal[2] * rayLocal[1] + normal[2] * rayLocal[2];

	// Save pdf
	pdf = 2.0f * cosTheta / PI;
}

void getRayDirectionSpecular(
	gsl_rng *rng,
	float *normal, float *rayInput, float *rayOutput,
	float &pdf, float exponent)
{
	float cosTheta = powf(randFloat(rng, 0.0f, 1.0f), 1.0f / (1.0f + exponent));
	float sinTheta = sqrtf(1.0f - cosTheta * cosTheta);
	float phi = 2.0f * PI * randFloat(rng, 0.0f, 1.0f);
	float cosPhi = cosf(phi);
	float sinPhi = sinf(phi);
	float rayReflect[3];
	float tangent[3];
	float binormal[3];
	float rayLocal[3];

	// Calculate reflected ray
	normalize(rayInput);
	reflect(normal, rayInput, rayReflect);
	invert(rayReflect);

	// Get tangent and binormal
	getTangentBinormal(rayReflect, tangent, binormal);

	// Calculate ray in local coordinates
	rayLocal[0] = sinTheta * cosPhi;
	rayLocal[1] = sinTheta * sinPhi;
	rayLocal[2] = cosTheta;

	// Transform to global coordinates
	rayOutput[0] = tangent[0] * rayLocal[0] + binormal[0] * rayLocal[1] + rayReflect[0] * rayLocal[2];
	rayOutput[1] = tangent[1] * rayLocal[0] + binormal[1] * rayLocal[1] + rayReflect[1] * rayLocal[2];
	rayOutput[2] = tangent[2] * rayLocal[0] + binormal[2] * rayLocal[1] + rayReflect[2] * rayLocal[2];

	// Save pdf
	pdf = powf(cosTheta, exponent) * (1.0f + exponent) / PI;
}

void getRayDirectionReflection(
	gsl_rng *rng,
	float *normal, float *rayInput, float *rayOutput,
	float &pdf)
{
	reflect(normal, rayInput, rayOutput);
	invert(rayOutput);
	pdf = 0.5f;
}

void getRayDirectionRefraction(
	gsl_rng *rng,
	float *normal, float *rayInput, float *rayOutput,
	float &pdf, float indexRel)
{
	refract(normal, rayInput, rayOutput, indexRel);
	invert(rayOutput);
	pdf = 0.5f;
}

void getRayDirectionGlossyReflection(
	gsl_rng *rng,
	float *normal, float *rayInput, float *rayOutput,
	float &pdf, float exponent)
{
	getRayDirectionSpecular(rng, normal, rayInput, rayOutput, pdf, exponent);
}

void getRayDirectionGlossyRefraction(
	gsl_rng *rng,
	float *normal, float *rayInput, float *rayOutput,
	float &pdf, float exponent, float indexRel)
{
	float cosTheta = powf(randFloat(rng, 0.0f, 1.0f), 1.0f / (1.0f + exponent));
	float sinTheta = sqrtf(1.0f - cosTheta * cosTheta);
	float phi = 2.0f * PI * randFloat(rng, 0.0f, 1.0f);
	float cosPhi = cosf(phi);
	float sinPhi = sinf(phi);
	float rayRefract[3];
	float tangent[3];
	float binormal[3];
	float rayLocal[3];

	// Calculate reflected ray
	normalize(rayInput);
	refract(normal, rayInput, rayRefract, indexRel);
	invert(rayRefract);

	// Get tangent and binormal
	getTangentBinormal(rayRefract, tangent, binormal);

	// Calculate ray in local coordinates
	rayLocal[0] = sinTheta * cosPhi;
	rayLocal[1] = sinTheta * sinPhi;
	rayLocal[2] = cosTheta;

	// Transform to global coordinates
	rayOutput[0] = tangent[0] * rayLocal[0] + binormal[0] * rayLocal[1] + rayRefract[0] * rayLocal[2];
	rayOutput[1] = tangent[1] * rayLocal[0] + binormal[1] * rayLocal[1] + rayRefract[1] * rayLocal[2];
	rayOutput[2] = tangent[2] * rayLocal[0] + binormal[2] * rayLocal[1] + rayRefract[2] * rayLocal[2];

	// Save pdf
	pdf = 2.0f * powf(cosTheta, exponent) * (1.0f + exponent) / PI;
}

void getRayDirection(
	gsl_rng *rng, BVHShader_samplingList *samplingList,
	float *normal, float *rayInput, float *rayOutput,
	float &pdf)
{
	normalize(normal);
	normalize(rayInput);

	if (samplingList == NULL)
	{
		getRayDirectionDiffuse(rng, normal, rayInput, rayOutput, pdf);
	}
	else
	{
		int samplingIndex = 0;
		float pdfRandom = randFloat(rng, 0.0f, 1.0f);

		for (int i = 0; i < samplingList->samplingNumber; i++)
		{
			if (pdfRandom < samplingList->samplings[i].pdfAcc)
			{
				samplingIndex = i;
				break;
			}
		}

		if (samplingList->samplings[samplingIndex].type == BVHSHADER_SAMPLING_DIFFUSE)
		{
			getRayDirectionDiffuse(
				rng, normal, rayInput, rayOutput, pdf);
		}
		else if (samplingList->samplings[samplingIndex].type == BVHSHADER_SAMPLING_SPECULAR)
		{
			getRayDirectionSpecular(
				rng, normal, rayInput, rayOutput, pdf,
				samplingList->samplings[samplingIndex].exponent);
		}
		else if (samplingList->samplings[samplingIndex].type == BVHSHADER_SAMPLING_REFLECTION)
		{
			getRayDirectionReflection(
				rng, normal, rayInput, rayOutput, pdf);
		}
		else if (samplingList->samplings[samplingIndex].type == BVHSHADER_SAMPLING_REFRACTION)
		{
			getRayDirectionRefraction(
				rng, normal, rayInput, rayOutput, pdf,
				samplingList->samplings[samplingIndex].indexRel);
		}
		else if (samplingList->samplings[samplingIndex].type == BVHSHADER_SAMPLING_GLOSSY_REFLECTION)
		{
			getRayDirectionGlossyReflection(
				rng, normal, rayInput, rayOutput, pdf,
				samplingList->samplings[samplingIndex].exponent);
		}
		else if (samplingList->samplings[samplingIndex].type == BVHSHADER_SAMPLING_GLOSSY_REFRACTION)
		{
			getRayDirectionGlossyRefraction(
				rng, normal, rayInput, rayOutput, pdf,
				samplingList->samplings[samplingIndex].exponent,
				samplingList->samplings[samplingIndex].indexRel);
		}
		else
		{
			getRayDirectionDiffuse(rng, normal, rayInput, rayOutput, pdf);
		}

		pdf *= samplingList->samplings[samplingIndex].pdf;
	}

	normalize(rayOutput);
}

void *BVHgpu_pathtracing_threadFunc(void *data)
{
	BVHgpu_threadData *threadData = (BVHgpu_threadData*)data;
	BVHgpu_renderState *state = threadData->renderState;
	gsl_rng *rng = threadData->rng;


		/* Variables */

	int pos;
	int x, y;
	int kernelDim[2] = {1, 1};
	int kernelIdx[2] = {0, 0};
	BVHColor nodeEyeColor;
	BVHColor nodeLightColor;
	BVHColor mediaColor;
	BVHShader_surface surfaceInfo;
	BVHShader_surface surfaceInfo2;
	BVHPrimitive *primitive;
	BVHShader *material;
	unsigned int primitiveLocation;
	unsigned int primitiveLocation2;
	float primPos[3];
	float posEyePrev[3];
	float posLightPrev[3];
	float posEye[3];
	float posLight[3];
	float dir[3];
	float distance2;
	float normal[3];
	BVHColor auxColor;
	float rgbAux[3];
	BVHLightInfo *lightInfo = new BVHLightInfo[state->bucketSize];
	BVHShader_light lightEnv;
	int subpathId;
	float pdf = 1.0f;
	BVHShader_samplingList *samplingList;

	surfaceInfo.attribBuffer = NULL;
	surfaceInfo.attribBufferSize = 0;
	surfaceInfo2.attribBuffer = NULL;
	surfaceInfo2.attribBufferSize = 0;

	while (1)
	{
		while (threadData->threadState == BVHGPU_THREAD_WAIT)
		{
			usleep(100);
		}

		if (threadData->threadState == BVHGPU_THREAD_STOP)
		{
			delete [] lightInfo;
			return NULL;
		}

		threadData->ret = 1;

		if (state->phase == PHASE_EYE_RAY && state->eyeDepth == 0)
		{
			// Generate camera rays
			for (pos = threadData->threadId; pos < state->bucketSize; pos += threadData->threadNumber)
			{
				x = (pos / state->bucket.size[1]) + state->bucket.pos[0];
				y = (pos % state->bucket.size[1]) + state->bucket.pos[1];

				// Get camera ray
				((BVHScene*)(state->scene))->getCameraRay(
					((float)(x + randFloat(rng, -state->blurRadius, state->blurRadius)) * 2.0f - (float)state->width) / (float)state->width,
					((float)(y + randFloat(rng, -state->blurRadius, state->blurRadius)) * 2.0f - (float)state->height) / (float)state->width,
					kernelDim, kernelIdx,
					state->rays[state->rayBuffer][pos].pos, state->rays[state->rayBuffer][pos].dir,
					rng);

				// Reset distance
				state->rays[state->rayBuffer][pos].distance = -1.0f;

				// Save data in the path
				state->paths[pos].setNodePosition(BVHBIDI_EYE, 0, state->rays[state->rayBuffer][pos].pos);
			}

			// Configure kernel params
			if (threadData->threadId == 0)
				state->nRays[state->rayBuffer] = state->bucketSize;
		}
		else if (state->phase == PHASE_LIGHT_RAY && state->lightDepth == 0)
		{
			// Generate light rays
			for (pos = threadData->threadId; pos < state->bucketSize; pos += threadData->threadNumber)
			{
				// Select a light
				lightInfo[pos] = ((BVHScene*)state->scene)->getLightRay(
					state->rays[state->rayBuffer][pos].pos, state->rays[state->rayBuffer][pos].dir, normal, pdf, rng);

				// Reset distance
				state->rays[state->rayBuffer][pos].distance = -1.0f;

				// Save data in the path
				state->paths[pos].setNodePosition(BVHBIDI_LIGHT, 0, state->rays[state->rayBuffer][pos].pos);
				state->paths[pos].setNodeNormal(BVHBIDI_LIGHT, 0, normal);
			}

			// Configure kernel params
			if (threadData->threadId == 0)
				state->nRays[state->rayBuffer] = state->bucketSize;
		}
		else if (state->phase == PHASE_EYE_RAY && state->eyeDepth < state->eyeSegmentLength - 1)
		{
			// Generate reflection, refraction or diffuse
			// Calculate node color
			// Calculate media color
			for (pos = threadData->threadId; pos < state->bucketSize; pos += threadData->threadNumber)
			{
				// Check intersection
				if (state->rays[state->rayBuffer][pos].distance < 0.0f)
				{
					// Generate fake ray
					state->rays[state->rayBuffer][pos].distance = -2.0f;

					// Calculate media color (for the moment just white)
					mediaColor.init(1.0f);

					// Set path colors
					state->paths[pos].setNodeMedia(BVHBIDI_EYE, state->eyeDepth, mediaColor);
				}
				else
				{
					// Save primitive location in the path node
					primitiveLocation = state->nodesSkipLocation[state->rays[state->rayBuffer][pos].closestNode * 2 + 1];
					state->paths[pos].setNodePrimitiveLocation(BVHBIDI_EYE, state->eyeDepth, primitiveLocation);

					// Get primitive info
					primPos[0] =
						state->rays[state->rayBuffer][pos].pos[0] +
						state->rays[state->rayBuffer][pos].dir[0] * state->rays[state->rayBuffer][pos].distance;
					primPos[1] =
						state->rays[state->rayBuffer][pos].pos[1] +
						state->rays[state->rayBuffer][pos].dir[1] * state->rays[state->rayBuffer][pos].distance;
					primPos[2] =
						state->rays[state->rayBuffer][pos].pos[2] +
						state->rays[state->rayBuffer][pos].dir[2] * state->rays[state->rayBuffer][pos].distance;

					primitive = getPrimitiveInfo(
						primPos, state->rays[state->rayBuffer][pos].dir,
						primitiveLocation, state->primitives, &surfaceInfo);

					// Save node position in the path
					state->paths[pos].setNodePosition(BVHBIDI_EYE, state->eyeDepth, surfaceInfo.pos);

					// Get previous node position
					state->paths[pos].getNodePosition(BVHBIDI_EYE, state->eyeDepth - 1, posEyePrev);

					// Calculate direction
					surfaceInfo.rayInput[0] = posEyePrev[0] - surfaceInfo.pos[0];
					surfaceInfo.rayInput[1] = posEyePrev[1] - surfaceInfo.pos[1];
					surfaceInfo.rayInput[2] = posEyePrev[2] - surfaceInfo.pos[2];

					// Get material
					material = primitive->getMaterial();

					// Modify normal with the shader
					material->getNormal(&surfaceInfo);

					// Get sampling
					samplingList = material->getSampling();

					// Generate new ray direction
					getRayDirection(
						rng, samplingList,
						surfaceInfo.normal, surfaceInfo.rayInput, surfaceInfo.rayOutput, pdf);

					// Calculate node color with the shader
					material->execute(&surfaceInfo);

					// Calculate media color (for the moment just white)
					mediaColor.init(1.0f);

					// Set node normal
					state->paths[pos].setNodeNormal(BVHBIDI_EYE, state->eyeDepth, surfaceInfo.normal);

					// Set path colors
					state->paths[pos].setNodeAttenuation(
						BVHBIDI_EYE, state->eyeDepth,
						mediaColor, surfaceInfo.color, surfaceInfo.emission,
						pdf, surfaceInfo.rayOutput);

					// Generate light rays
					state->rays[state->rayBuffer][pos].pos[0] = surfaceInfo.pos[0];
					state->rays[state->rayBuffer][pos].pos[1] = surfaceInfo.pos[1];
					state->rays[state->rayBuffer][pos].pos[2] = surfaceInfo.pos[2];

					// In a random direction
					state->rays[state->rayBuffer][pos].dir[0] = surfaceInfo.rayOutput[0];
					state->rays[state->rayBuffer][pos].dir[1] = surfaceInfo.rayOutput[1];
					state->rays[state->rayBuffer][pos].dir[2] = surfaceInfo.rayOutput[2];

					// Reset distance
					state->rays[state->rayBuffer][pos].distance = -1.0f;
				}
			}

			// Configure kernel params
			if (threadData->threadId == 0)
				state->nRays[state->rayBuffer] = state->bucketSize;
		}
		else if (state->phase == PHASE_LIGHT_RAY && state->lightDepth < state->lightSegmentLength - 1)
		{
			// Generate diffuse, specular or caustic
			// Calculate node color
			// Calculate media color
			for (pos = threadData->threadId; pos < state->bucketSize; pos += threadData->threadNumber)
			{
				// Check intersection
				if (state->rays[state->rayBuffer][pos].distance < 0.0f)
				{
					// Generate fake ray
					state->rays[state->rayBuffer][pos].distance = -2.0f;

					// Calculate media color (for the moment just white)
					mediaColor.init(1.0f);

					// Set path colors
					state->paths[pos].setNodeMedia(BVHBIDI_LIGHT, state->lightDepth, mediaColor);
				}
				else
				{
					// Save primitive location in the path node
					primitiveLocation = state->nodesSkipLocation[state->rays[state->rayBuffer][pos].closestNode * 2 + 1];
					state->paths[pos].setNodePrimitiveLocation(BVHBIDI_LIGHT, state->lightDepth, primitiveLocation);

					// Get primitive info
					primPos[0] =
						state->rays[state->rayBuffer][pos].pos[0] +
						state->rays[state->rayBuffer][pos].dir[0] * state->rays[state->rayBuffer][pos].distance;
					primPos[1] =
						state->rays[state->rayBuffer][pos].pos[1] +
						state->rays[state->rayBuffer][pos].dir[1] * state->rays[state->rayBuffer][pos].distance;
					primPos[2] =
						state->rays[state->rayBuffer][pos].pos[2] +
						state->rays[state->rayBuffer][pos].dir[2] * state->rays[state->rayBuffer][pos].distance;

					primitive = getPrimitiveInfo(
						primPos, state->rays[state->rayBuffer][pos].dir,
						primitiveLocation, state->primitives, &surfaceInfo);

					// Save node position in the path
					state->paths[pos].setNodePosition(BVHBIDI_LIGHT, state->lightDepth, surfaceInfo.pos);

					// Get previous node position
					state->paths[pos].getNodePosition(BVHBIDI_LIGHT, state->lightDepth - 1, posLightPrev);

					// Calculate direction
					surfaceInfo.rayOutput[0] = posLightPrev[0] - surfaceInfo.pos[0];
					surfaceInfo.rayOutput[1] = posLightPrev[1] - surfaceInfo.pos[1];
					surfaceInfo.rayOutput[2] = posLightPrev[2] - surfaceInfo.pos[2];

					// Get material
					material = primitive->getMaterial();

					// Modify normal with the shader
					material->getNormal(&surfaceInfo);

					// Get sampling
					samplingList = material->getSampling();

					// Generate new ray direction
					getRayDirection(
						rng, samplingList,
						surfaceInfo.normal, surfaceInfo.rayOutput, surfaceInfo.rayInput, pdf);

					// Calculate node color with the shader
					material->execute(&surfaceInfo);

					// Calculate media color (for the moment just white)
					mediaColor.init(1.0f);

					// Save node normal
					state->paths[pos].setNodeNormal(BVHBIDI_LIGHT, state->lightDepth, surfaceInfo.normal);

					// Set path colors
					state->paths[pos].setNodeAttenuation(
						BVHBIDI_LIGHT, state->lightDepth,
						mediaColor, surfaceInfo.color, surfaceInfo.emission,
						pdf, surfaceInfo.rayInput);

					// Generate light rays
					state->rays[state->rayBuffer][pos].pos[0] = surfaceInfo.pos[0];
					state->rays[state->rayBuffer][pos].pos[1] = surfaceInfo.pos[1];
					state->rays[state->rayBuffer][pos].pos[2] = surfaceInfo.pos[2];

					// In a random direction
					state->rays[state->rayBuffer][pos].dir[0] = surfaceInfo.rayInput[0];
					state->rays[state->rayBuffer][pos].dir[1] = surfaceInfo.rayInput[1];
					state->rays[state->rayBuffer][pos].dir[2] = surfaceInfo.rayInput[2];

					// Reset distance
					state->rays[state->rayBuffer][pos].distance = -1.0f;
				}
			}

			// Configure kernel params
			if (threadData->threadId == 0)
				state->nRays[state->rayBuffer] = state->bucketSize;
		}
		else if (state->phase == PHASE_EYE_RAY && state->eyeDepth < state->eyeSegmentLength)
		{
			for (pos = threadData->threadId; pos < state->bucketSize; pos += threadData->threadNumber)
			{
				// Check intersection
				if (state->rays[state->rayBuffer][pos].distance < 0.0f)
				{
					// Do nothing
				}
				else
				{
					// Save primitive location in the path node
					primitiveLocation = state->nodesSkipLocation[state->rays[state->rayBuffer][pos].closestNode * 2 + 1];
					state->paths[pos].setNodePrimitiveLocation(BVHBIDI_EYE, state->eyeDepth, primitiveLocation);

					// Get primitive info
					primPos[0] =
						state->rays[state->rayBuffer][pos].pos[0] +
						state->rays[state->rayBuffer][pos].dir[0] * state->rays[state->rayBuffer][pos].distance;
					primPos[1] =
						state->rays[state->rayBuffer][pos].pos[1] +
						state->rays[state->rayBuffer][pos].dir[1] * state->rays[state->rayBuffer][pos].distance;
					primPos[2] =
						state->rays[state->rayBuffer][pos].pos[2] +
						state->rays[state->rayBuffer][pos].dir[2] * state->rays[state->rayBuffer][pos].distance;

					primitive = getPrimitiveInfo(
						primPos, state->rays[state->rayBuffer][pos].dir,
						primitiveLocation, state->primitives, &surfaceInfo);

					// Save node position in the path
					state->paths[pos].setNodePosition(BVHBIDI_EYE, state->eyeDepth, surfaceInfo.pos);

					// Get material
					material = primitive->getMaterial();

					// Modify normal with the shader
					material->getNormal(&surfaceInfo);

					// Set node normal
					state->paths[pos].setNodeNormal(BVHBIDI_EYE, state->eyeDepth, surfaceInfo.normal);

					// Calculate media color (for the moment just white)
					mediaColor.init(1.0f);

					// Set path colors
					state->paths[pos].setNodeMedia(BVHBIDI_EYE, state->eyeDepth, mediaColor);
				}
			}

			// Configure kernel params
			if (threadData->threadId == 0)
				state->nRays[state->rayBuffer] = 0;
		}
		else if (state->phase == PHASE_LIGHT_RAY && state->lightDepth < state->lightSegmentLength)
		{
			for (pos = threadData->threadId; pos < state->bucketSize; pos += threadData->threadNumber)
			{
				// Check intersection
				if (state->rays[state->rayBuffer][pos].distance < 0.0f)
				{
					//Do nothing
				}
				else
				{
					// Save primitive location in the path node
					primitiveLocation = state->nodesSkipLocation[state->rays[state->rayBuffer][pos].closestNode * 2 + 1];
					state->paths[pos].setNodePrimitiveLocation(BVHBIDI_LIGHT, state->lightDepth, primitiveLocation);

					// Get primitive info
					primPos[0] =
						state->rays[state->rayBuffer][pos].pos[0] +
						state->rays[state->rayBuffer][pos].dir[0] * state->rays[state->rayBuffer][pos].distance;
					primPos[1] =
						state->rays[state->rayBuffer][pos].pos[1] +
						state->rays[state->rayBuffer][pos].dir[1] * state->rays[state->rayBuffer][pos].distance;
					primPos[2] =
						state->rays[state->rayBuffer][pos].pos[2] +
						state->rays[state->rayBuffer][pos].dir[2] * state->rays[state->rayBuffer][pos].distance;

					primitive = getPrimitiveInfo(
						primPos, state->rays[state->rayBuffer][pos].dir,
						primitiveLocation, state->primitives, &surfaceInfo);

					// Save node position in the path
					state->paths[pos].setNodePosition(BVHBIDI_LIGHT, state->lightDepth, surfaceInfo.pos);

					// Get material
					material = primitive->getMaterial();

					// Modify normal with the shader
					material->getNormal(&surfaceInfo);

					// Save node normal
					state->paths[pos].setNodeNormal(BVHBIDI_LIGHT, state->lightDepth, surfaceInfo.normal);

					// Calculate media color (for the moment just white)
					mediaColor.init(1.0f);

					// Set path colors
					state->paths[pos].setNodeMedia(BVHBIDI_LIGHT, state->lightDepth, mediaColor);
				}
			}

			// Configure kernel params
			if (threadData->threadId == 0)
				state->nRays[state->rayBuffer] = 0;
		}
		else if (state->phase == PHASE_EYE_RAY)
		{
			if (threadData->threadId == 0)
				state->nRays[state->rayBuffer] = 0;
		}
		else if (state->phase == PHASE_LIGHT_RAY)
		{
			if (threadData->threadId == 0)
				state->nRays[state->rayBuffer] = 0;
		}
		else if (state->phase == PHASE_SUBPATH)
		{
			// Generate the rays to calculate the subpaths color and the next subpaths
			for (pos = threadData->threadId; pos < state->bucketSize; pos += threadData->threadNumber)
			{
				for (int i = 0; i < state->subpathStep; i++)
				{
					if (state->subpathId > state->subpathStep)
					{
						subpathId = state->subpathId + i - state->subpathStep * 2;

						if (state->paths[pos].isSubpathEye(subpathId))
							continue;

						// Get subpath vertex positions
						if (state->paths[pos].getSubpathVertices(subpathId, posEye, posLight))
						{
							// Calculate media color
							dir[0] = posLight[0] - posEye[0];
							dir[1] = posLight[1] - posEye[1];
							dir[2] = posLight[2] - posEye[2];
							distance2 = dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2];

							if (state->rays[state->rayBuffer][pos * state->subpathStep + i].distance < 0.0f ||
								state->rays[state->rayBuffer][pos * state->subpathStep + i].distance *
								state->rays[state->rayBuffer][pos * state->subpathStep + i].distance > distance2)
							{
								// Set media color
								mediaColor.init(1.0f);

								// Get subpath info
								state->paths[pos].getSubpathInfo(
									subpathId,
									surfaceInfo.normal, surfaceInfo2.normal,
									surfaceInfo.rayInput, surfaceInfo2.rayOutput,
									primitiveLocation, primitiveLocation2);

								primitive = getPrimitiveInfo(
									posEye, NULL,
									primitiveLocation, state->primitives, &surfaceInfo);

								// Set ray input
								surfaceInfo.rayOutput[0] = dir[0];
								surfaceInfo.rayOutput[1] = dir[1];
								surfaceInfo.rayOutput[2] = dir[2];

								// Calculate node eye color with the shader
								material = primitive->getMaterial();
								material->execute(&surfaceInfo);

								nodeEyeColor = surfaceInfo.color;

								if (state->paths[pos].isSubpathLight(subpathId))
								{
									// Calculate node light color
									lightEnv.rayOutput[0] = -dir[0];
									lightEnv.rayOutput[1] = -dir[1];
									lightEnv.rayOutput[2] = -dir[2];
									((BVHScene*)state->scene)->getLightColor(lightInfo[pos], &lightEnv);
									nodeLightColor = lightEnv.emission;
								}
								else
								{
									// Get primitive info
									primitive = getPrimitiveInfo(
										posLight, NULL,
										primitiveLocation2, state->primitives, &surfaceInfo2);

									// Set ray input
									surfaceInfo2.rayInput[0] = -dir[0];
									surfaceInfo2.rayInput[1] = -dir[1];
									surfaceInfo2.rayInput[2] = -dir[2];

									// Calculate node light color with the shader
									material = primitive->getMaterial();
									material->execute(&surfaceInfo2);

									nodeLightColor = surfaceInfo2.color;
								}

								// Set subpath colors
								state->paths[pos].setSubpathColors(
									subpathId, 1,
									mediaColor, nodeEyeColor, nodeLightColor);
							}
							else
							{
								// Set subpath colors
								state->paths[pos].setSubpathColors(
									subpathId, 0,
									mediaColor, nodeEyeColor, nodeLightColor);
							}
						}
					}

					if (state->subpathId < state->subpathMax)
					{
						// Get node positions
						if (state->paths[pos].getSubpathVertices(state->subpathId + i, posEye, posLight))
						{
							// Generate ray from posEye to posLight
							state->rays[state->rayBuffer][pos * state->subpathStep + i].pos[0] = posEye[0];
							state->rays[state->rayBuffer][pos * state->subpathStep + i].pos[1] = posEye[1];
							state->rays[state->rayBuffer][pos * state->subpathStep + i].pos[2] = posEye[2];

							// In a random direction
							state->rays[state->rayBuffer][pos * state->subpathStep + i].dir[0] = posLight[0] - posEye[0];
							state->rays[state->rayBuffer][pos * state->subpathStep + i].dir[1] = posLight[1] - posEye[1];
							state->rays[state->rayBuffer][pos * state->subpathStep + i].dir[2] = posLight[2] - posEye[2];

							// Reset distance
							state->rays[state->rayBuffer][pos * state->subpathStep + i].distance = -1.0f;
						}
						else
						{
							// Send fake ray
							state->rays[state->rayBuffer][pos * state->subpathStep + i].distance = -2.0f;
						}
					}
				}
			}

			// Configure kernel params
			if (threadData->threadId == 0)
			{
				if (state->subpathId < state->subpathMax)
					state->nRays[state->rayBuffer] = state->bucketSize * state->subpathStep;
				else
					state->nRays[state->rayBuffer] = 0;
			}
		}
		else if (state->phase == PHASE_INTERSECTION)
		{
			for (pos = threadData->threadId; pos < state->nRays[!state->rayBuffer]; pos += threadData->threadNumber)
			{
				BVHgpu_trace(
					state->nNodes, state->nodesMin, state->nodesMax, state->nodesSkipLocation,
					&state->rays[!state->rayBuffer][pos], state->primitives);
			}
		}
		else if (state->phase == PHASE_FILL_BUFFER)
		{
			for (pos = threadData->threadId; pos < state->bucketSize; pos += threadData->threadNumber)
			{
				x = (pos / state->bucket.size[1]) + state->bucket.pos[0];
				y = (pos % state->bucket.size[1]) + state->bucket.pos[1];

				auxColor = state->paths[pos].getColor();
				auxColor.toRGB(rgbAux);
				state->renderBuffer[x + y * state->width].rgb[0] += rgbAux[0] / state->samplesPerPixel;
				state->renderBuffer[x + y * state->width].rgb[1] += rgbAux[1] / state->samplesPerPixel;
				state->renderBuffer[x + y * state->width].rgb[2] += rgbAux[2] / state->samplesPerPixel;

				if (state->paths[pos].getPathLengthCut(BVHBIDI_EYE) > 1)
					state->renderBuffer[x + y * state->width].alpha += 1.0f / state->samplesPerPixel;
			}
		}
		else
		{
			threadData->ret = 0;
		}

		threadData->threadState = BVHGPU_THREAD_WAIT;
	}

	return NULL;
}

void BVHgpu_pathtracing_launchThreads(
	BVHgpu_renderState *state, int nThreads,
	BVHgpu_threadData *threadData, pthread_t *threads)
{
	for (int i = 0; i < nThreads; i++)
	{
		threadData[i].renderState = state;
		threadData[i].threadState = BVHGPU_THREAD_WAIT;
		threadData[i].threadNumber = nThreads;
		threadData[i].threadId = i;
		threadData[i].rng = gsl_rng_alloc(gsl_rng_taus);
		if (i == 0)
			gsl_rng_set(threadData[i].rng, time(NULL));
		else
			gsl_rng_set(threadData[i].rng, gsl_rng_get(threadData[i - 1].rng));
		pthread_create(&threads[i], NULL, BVHgpu_pathtracing_threadFunc, (void*)&threadData[i]);
	}

	/*
	for (int i = 0; i < nThreads; i++)
	{
		threadData[i].rng = gsl_rng_alloc(gsl_rng_taus);
		if (i == 0)
			gsl_rng_set(threadData[i].rng, time(NULL));
		else
			gsl_rng_set(threadData[i].rng, gsl_rng_get(threadData[i - 1].rng));
	}*/
}

void BVHgpu_pathtracing_stopThreads(
	BVHgpu_renderState *state, int nThreads,
	BVHgpu_threadData *threadData, pthread_t *threads)
{
	for (int i = 0; i < nThreads; i++)
	{
		while (threadData[i].threadState == BVHGPU_THREAD_RUN)
		{
			usleep(100);
		}

		threadData[i].threadState = BVHGPU_THREAD_STOP;
	}

	for (int i = 0; i < nThreads; i++)
	{
		pthread_join(threads[i], NULL);
		gsl_rng_free(threadData[i].rng);
	}
}

int BVHgpu_pathtracing(
	BVHgpu_renderState *state, int nThreads,
	BVHgpu_threadData *threadData, pthread_t *threads)
{
	int ret = 1;
	int allFinished;

	for (int i = 0; i < nThreads; i++)
	{
		threadData[i].threadState = BVHGPU_THREAD_RUN;
	}

	do
	{
		allFinished = 1;
		usleep(100);

		for (int i = 0; i < nThreads; i++)
		{
			if (threadData[i].threadState == BVHGPU_THREAD_RUN)
				allFinished = 0;
		}
	}while (!allFinished);

	for (int i = 0; i < nThreads; i++)
	{
		ret = ret && threadData[i].ret;
	}

	return ret;

	/*
	double aux;

	if (nThreads <= 1)
	{
		BVHgpu_threadData threadData = {state, 0, 1, 0};
		BVHgpu_pathtracing_threadFunc(&threadData);
		return threadData.ret;
	}
	else
	{
		int ret = 1;

		for (int i = 0; i < nThreads; i++)
		{
			threadData[i].renderState = state;
			threadData[i].threadState = 1;
			threadData[i].threadNumber = nThreads;
			threadData[i].threadId = i;
			pthread_create(&threads[i], NULL, BVHgpu_pathtracing_threadFunc, (void*)&threadData[i]);
		}

		for (int i = 0; i < nThreads; i++)
		{
			pthread_join(threads[i], NULL);
			ret = ret && threadData[i].ret;
		}

		return ret;
	}*/
}

void BVHgpu_render(
	// Scene definition
	unsigned int gpu_nPrimitives, unsigned int *gpu_primitiveLocations,
	unsigned int gpu_primitiveSize, unsigned char *gpu_primitives,
	unsigned int gpu_nNodes, float *gpu_nodesMin, float *gpu_nodesMax, unsigned int *gpu_nodesSkipLocation,
	// Render buffer
	unsigned int width, unsigned int height, BVH_gpuRenderBuffer *renderBuffer, void *scene, int samplesPerPixel,
	int lightDepth, int eyeDepth, float blurRadius,
	// Kernel control
	int nThreadsCPU, BVHgpu_bucket *bucket,
	// Stats
	BVHgpu_stats *stats)
{
		/* Variables */

	BVHgpu_renderState state;
	int phaseAux;

	int pos;

	struct timespec time0, time1;

	BVHgpu_threadData *threadDataCPU = new BVHgpu_threadData[nThreadsCPU];
	pthread_t *threadsCPU = new pthread_t[nThreadsCPU];


		/* Init stats */

	stats->timeInit = 0.0;
	stats->timeOthers = 0.0;
	stats->timeGenRays = 0.0;
	stats->timeFillBuffer = 0.0;
	stats->timeIntersection = 0.0;


		/* Init things */

	clock_gettime(CLOCK_REALTIME, &time0);

	// Bucket
	state.bucket = *bucket;
	state.bucketSize = state.bucket.size[0] * state.bucket.size[1];

	// Paths
	state.eyeSegmentLength = eyeDepth;
	state.lightSegmentLength = lightDepth;
	state.nRaysMax = state.bucketSize * state.eyeSegmentLength;
	state.paths = new BVHBidi[state.bucketSize];

	// Rays
	state.rays[0] = new BVHgpu_ray[state.nRaysMax];
	state.rays[1] = new BVHgpu_ray[state.nRaysMax];
	state.nRays[0] = 0;
	state.nRays[1] = 0;

	// Primitives
	state.nNodes = gpu_nNodes;
	state.nodesMin = gpu_nodesMin;
	state.nodesMax = gpu_nodesMax;
	state.primitives = gpu_primitives;
	state.nodesSkipLocation = gpu_nodesSkipLocation;

	// Dimensions
	state.width = width;
	state.height = height;

	// Scene
	state.scene = scene;

	// Render buffer
	state.renderBuffer = renderBuffer;

	// Samples per pixel
	state.samplesPerPixel = samplesPerPixel;

	// Blur radius
	state.blurRadius = blurRadius;

	// Clear render buffer
	for (int x = state.bucket.pos[0]; x < state.bucket.pos[0] + state.bucket.size[0]; x++)
	{
		for (int y = state.bucket.pos[1]; y < state.bucket.pos[1] + state.bucket.size[1]; y++)
		{
			pos = x + y * width;

			// Reset buffer
			renderBuffer[pos].rgb[0] = 0.0f;
			renderBuffer[pos].rgb[1] = 0.0f;
			renderBuffer[pos].rgb[2] = 0.0f;
			renderBuffer[pos].alpha = 0.0f;
		}
	}

	clock_gettime(CLOCK_REALTIME, &time1);
	stats->timeInit += (double)(time1.tv_sec - time0.tv_sec) + 1E-9 * (double)(time1.tv_nsec - time0.tv_nsec);


		/* Launch threads */

	BVHgpu_pathtracing_launchThreads(&state, nThreadsCPU, threadDataCPU, threadsCPU);


		/* Render bucket */

	for (int s = 0; s < samplesPerPixel; s++)
	{
		// Init phase
		state.phase = PHASE_EYE_RAY;
		state.eyeDepth = 0;
		state.lightDepth = 0;
		state.subpathId = 0;
		state.subpathMax = state.paths[0].getSubpathNumber();
		state.subpathStep = state.paths[0].getSubpathStep();
		state.rayBuffer = 0;

		// Reset paths
		for (pos = 0; pos < state.bucketSize; pos++)
			state.paths[pos].reset();

		while (1)
		{
			// No in the first step (there is no rays in the buffer)
			if (state.nRays[!state.rayBuffer] > 0)
			{
				clock_gettime(CLOCK_REALTIME, &time0);
				phaseAux = state.phase;
				state.phase = PHASE_INTERSECTION;
				if (!BVHgpu_pathtracing(&state, nThreadsCPU, threadDataCPU, threadsCPU))
					break;
				state.phase = phaseAux;
				clock_gettime(CLOCK_REALTIME, &time1);
				stats->timeIntersection += (double)(time1.tv_sec - time0.tv_sec) + 1E-9 * (double)(time1.tv_nsec - time0.tv_nsec);
			}

			clock_gettime(CLOCK_REALTIME, &time0);
			if (!BVHgpu_pathtracing(&state, nThreadsCPU, threadDataCPU, threadsCPU))
				break;
			clock_gettime(CLOCK_REALTIME, &time1);
			stats->timeGenRays += (double)(time1.tv_sec - time0.tv_sec) + 1E-9 * (double)(time1.tv_nsec - time0.tv_nsec);

			// Swap buffers
			state.rayBuffer = !state.rayBuffer;

			// Next phase
			clock_gettime(CLOCK_REALTIME, &time0);
			if (state.phase == PHASE_EYE_RAY)
			{
				state.phase = PHASE_LIGHT_RAY;
				state.eyeDepth++;

				if (state.eyeDepth >= state.eyeSegmentLength && state.lightDepth >= state.lightSegmentLength)
					state.phase = PHASE_SUBPATH;
			}
			else if (state.phase == PHASE_LIGHT_RAY)
			{
				state.phase = PHASE_EYE_RAY;
				state.lightDepth++;

				if (state.eyeDepth >= state.eyeSegmentLength && state.lightDepth >= state.lightSegmentLength)
					state.phase = PHASE_SUBPATH;
			}
			else if (state.phase == PHASE_SUBPATH)
			{
				state.subpathId += state.subpathStep;

				if (state.subpathId > state.subpathMax + state.subpathStep)
				{
					clock_gettime(CLOCK_REALTIME, &time1);
					stats->timeOthers += (double)(time1.tv_sec - time0.tv_sec) + 1E-9 * (double)(time1.tv_nsec - time0.tv_nsec);
					break;
				}
			}
			else
			{
				break;
			}
			clock_gettime(CLOCK_REALTIME, &time1);
			stats->timeOthers += (double)(time1.tv_sec - time0.tv_sec) + 1E-9 * (double)(time1.tv_nsec - time0.tv_nsec);
		}

		// Fill buffer
		clock_gettime(CLOCK_REALTIME, &time0);
		state.phase = PHASE_FILL_BUFFER;
		if (!BVHgpu_pathtracing(&state, nThreadsCPU, threadDataCPU, threadsCPU))
			break;
		clock_gettime(CLOCK_REALTIME, &time1);
		stats->timeFillBuffer += (double)(time1.tv_sec - time0.tv_sec) + 1E-9 * (double)(time1.tv_nsec - time0.tv_nsec);
	}


		/* Stop threads */

	BVHgpu_pathtracing_stopThreads(&state, nThreadsCPU, threadDataCPU, threadsCPU);


		/* Calculate stats */

    stats->timeTotal = stats->timeIntersection + stats->timeInit + stats->timeGenRays + stats->timeFillBuffer + stats->timeOthers;


    	/* Free memory */

	delete [] state.paths;
	delete [] state.rays[0];
	delete [] state.rays[1];
	delete [] threadDataCPU;
	delete [] threadsCPU;
}
