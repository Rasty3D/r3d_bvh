/*
 * bvh.cpp
 *
 *  Created on: 19 Jan 2011
 *      Author: showroom
 */


/*
 * INCLUDES
 */

#include "bvh.h"


/*
 * NAMESPACES
 */

using namespace std;


/*
 * CLASS: BVH
 */

	/* Verbose */

int BVH::verbose = 1;


	/* Constructor and destructor */

BVH::BVH()
{
	this->gpu_nPrimitives = 0;
	this->gpu_primitiveLocations = NULL;

	this->gpu_primitiveSize = 0;
	this->gpu_primitives = NULL;

	this->gpu_nNodes = 0;
	this->gpu_nodesMin = NULL;
	this->gpu_nodesMax = NULL;
	this->gpu_nodesSkipLocation = NULL;
}

BVH::~BVH()
{
	// Clear primitive list
	this->scene.clear();

	// Clear GPU lists
	if (this->gpu_nPrimitives > 0)
		delete [] this->gpu_primitiveLocations;

	if (this->gpu_primitiveSize > 0)
		delete [] this->gpu_primitives;

	if (this->gpu_nNodes > 0)
	{
		delete [] this->gpu_nodesMin;
		delete [] this->gpu_nodesMax;
		delete [] this->gpu_nodesSkipLocation;
	}
}


	/* Init function */

void BVH::init(const char *colorSystem)
{
	// Init color system
	if (colorSystem != NULL)
		BVHColor::setColorSystem(colorSystem);
	else
		BVHColor::setColorSystem("HDTV");

	// Init textures
	BVHTexture::init();

	// Init shader auxiliary functions
	BVHShader::setFunc_rng(randFloat);
	BVHShader::setFunc_getColor(BVHTexture::getColor);
	BVHShader::setFunc_getColorUV(BVHTexture::getColorUV);
	BVHShader::setFunc_getColorUVW(BVHTexture::getColorUVW);
	BVHShader::setFunc_getIntensity(BVHTexture::getIntensity);
	BVHShader::setFunc_getIntensityUV(BVHTexture::getIntensityUV);
	BVHShader::setFunc_getIntensityUVW(BVHTexture::getIntensityUVW);
}


	/* Load scene */

int BVH::load(const char *filename)
{
	return this->scene.load(filename);
}


	/* Build tree */

void BVH::buildTree(int nThreads)
{
	// Clean tree
	BVHNode *root = new BVHNode;

	// Add a node per primitive
	std::list<BVHPrimitive>::iterator primitiveIt;

	for (primitiveIt = this->scene.primitives.begin(); primitiveIt != this->scene.primitives.end(); primitiveIt++)
	{
		BVHNode *node = new BVHNode;
		root->addChild(node);
		node->setPrimitive(&(*primitiveIt));
	}

	// Divide nodes recursively
	root->divide(nThreads);

	// Build static tree
	//if (this->nodeStaticNumber > 0)
	//	delete [] this->nodeStaticList;

	// Traverse all the nodes and put new ids
	unsigned long nodeId = 0;
	int nodeStaticNumber = root->setId(nodeId);
	BVHNodeStatic *nodeStaticList = new BVHNodeStatic[nodeStaticNumber];

	// Traverse (again) all the nodes and fill the static list
	int index = 0;
	root->fillStatic(nodeStaticList, nodeStaticNumber, index, nodeStaticNumber);

	// Delete root node
	delete root;

	// Generate GPU lists
	this->generateGPUlists(nodeStaticNumber, nodeStaticList);

	// Delete static list
	delete [] nodeStaticList;
}

	/* Generate GPU friendly lists */

void BVH::generateGPUlists(int nodeStaticNumber, BVHNodeStatic *nodeStaticList)
{
	BVHgpu_primitiveHeader *header;

	// Clear GPU lists
	if (this->gpu_nPrimitives > 0)
		delete [] this->gpu_primitiveLocations;
	this->gpu_nPrimitives = 0;

	if (this->gpu_primitiveSize > 0)
		delete [] this->gpu_primitives;
	this->gpu_primitiveSize = 0;

	if (this->gpu_nNodes > 0)
	{
		delete [] this->gpu_nodesMin;
		delete [] this->gpu_nodesMax;
		delete [] this->gpu_nodesSkipLocation;
	}
	this->gpu_nNodes = 0;

	// Generate primitive list
	this->gpu_nPrimitives = this->scene.primitives.size();
	this->gpu_primitiveLocations = new unsigned int[this->gpu_nPrimitives];

	// Calculate primitive size
	this->gpu_primitiveSize = 0;
	unsigned int primitiveLocation = 0;
	int primitiveIndex = 0;
	std::list<BVHPrimitive>::iterator primitiveIt;

	for (primitiveIt = this->scene.primitives.begin(); primitiveIt != this->scene.primitives.end(); primitiveIt++)
	{
		this->gpu_primitiveSize += sizeof(BVHgpu_primitiveHeader) + (*primitiveIt).getGeomDataSize();
	}

	// Generate primitive buffer
	gpu_primitives = new unsigned char[this->gpu_primitiveSize];

	// Fill primitive buffer and location list
	for (primitiveIt = this->scene.primitives.begin(); primitiveIt != this->scene.primitives.end(); primitiveIt++)
	{
		header = (BVHgpu_primitiveHeader*)&this->gpu_primitives[primitiveLocation];
		header->type = (*primitiveIt).getType();
		header->primitive = (void*)&(*primitiveIt);
		memcpy(this->gpu_primitives + primitiveLocation + sizeof(BVHgpu_primitiveHeader), (*primitiveIt).getGeomData(), (*primitiveIt).getGeomDataSize());
		this->gpu_primitiveLocations[primitiveIndex] = primitiveLocation;
		(*primitiveIt).setIndex(primitiveIndex);

		primitiveLocation += sizeof(BVHgpu_primitiveHeader) + (*primitiveIt).getGeomDataSize();
		primitiveIndex++;
	}

	// Generate nodes
	this->gpu_nNodes = nodeStaticNumber;
	this->gpu_nodesMin = new float[this->gpu_nNodes * 4];
	this->gpu_nodesMax = new float[this->gpu_nNodes * 4];
	this->gpu_nodesSkipLocation = new unsigned int[this->gpu_nNodes * 2];

	for (unsigned int i = 0; i < this->gpu_nNodes; i++)
	{
		this->gpu_nodesSkipLocation[i * 2] = nodeStaticList[i].skipIndex;

		this->gpu_nodesMin[i * 4] = nodeStaticList[i].aabb.min[0];
		this->gpu_nodesMin[i * 4 + 1] = nodeStaticList[i].aabb.min[1];
		this->gpu_nodesMin[i * 4 + 2] = nodeStaticList[i].aabb.min[2];
		this->gpu_nodesMin[i * 4 + 3] = 0.0f;
		this->gpu_nodesMax[i * 4] = nodeStaticList[i].aabb.max[0];
		this->gpu_nodesMax[i * 4 + 1] = nodeStaticList[i].aabb.max[1];
		this->gpu_nodesMax[i * 4 + 2] = nodeStaticList[i].aabb.max[2];
		this->gpu_nodesMax[i * 4 + 3] = 0.0f;

		if (nodeStaticList[i].primitive != NULL)
			this->gpu_nodesSkipLocation[i * 2 + 1] = this->gpu_primitiveLocations[nodeStaticList[i].primitive->getIndex()];
		else
			this->gpu_nodesSkipLocation[i * 2 + 1] = BVHGPU_NOPRIMITIVE;
	}
}


	/* Render */

void BVH::render(
	unsigned int width, unsigned int height,
	BVH_gpuRenderBuffer *renderBuffer, int samplesPerPixel,
	int lightDepth, int eyeDepth, float blurRadius,
	int widthBucket, int heightBucket,
	int nRenderUnits, BVHRenderUnit *renderUnits)
{
	pthread_t *threads = new pthread_t[nRenderUnits];
	BVHRenderUnitInfo *threadData = new BVHRenderUnitInfo[nRenderUnits];
	BVHBucketList bucketList;

	// Calculate bucket list
	int bucketSizeX = iDivUp(width, widthBucket);
	int bucketSizeY = iDivUp(height, heightBucket);
	bucketList.bucketSize = bucketSizeX * bucketSizeY;
	bucketList.buckets = new BVHgpu_bucket[bucketList.bucketSize];
	bucketList.bucketCurrent = 0;
	pthread_mutex_init(&bucketList.bucketMutex, NULL);

	// Set path lenghts
	BVHBidi::setPathLengthMax(eyeDepth, lightDepth);

	for (int i = 0; i < bucketSizeX; i++)
	{
		for (int j = 0; j < bucketSizeY; j++)
		{
			bucketList.buckets[i + j * bucketSizeX].pos[0] = i * widthBucket;
			bucketList.buckets[i + j * bucketSizeX].pos[1] = j * heightBucket;

			if (i < bucketSizeX - 1)
				bucketList.buckets[i + j * bucketSizeX].size[0] = widthBucket;
			else
				bucketList.buckets[i + j * bucketSizeX].size[0] = width - i * widthBucket;

			if (j < bucketSizeY - 1)
				bucketList.buckets[i + j * bucketSizeX].size[1] = heightBucket;
			else
				bucketList.buckets[i + j * bucketSizeX].size[1] = height - j * heightBucket;
		}
	}

	// Save time
	clock_gettime(CLOCK_REALTIME, &this->timeInitRendering);

	// Launch threads
	for (int i = 0; i < nRenderUnits; i++)
	{
		renderUnits[i].id = i + 1;
		threadData[i].renderUnit = &renderUnits[i];

		threadData[i].bucketList = &bucketList;

		threadData[i].nPrimitives = this->gpu_nPrimitives;
		threadData[i].primitiveLocations = this->gpu_primitiveLocations;

		threadData[i].primitiveSize = this->gpu_primitiveSize;
		threadData[i].primitives = this->gpu_primitives;

		threadData[i].nNodes = this->gpu_nNodes;
		threadData[i].nodesMin = this->gpu_nodesMin;
		threadData[i].nodesMax = this->gpu_nodesMax;
		threadData[i].nodesSkipLocation = this->gpu_nodesSkipLocation;

		threadData[i].width = width;
		threadData[i].height = height;
		threadData[i].renderBuffer = renderBuffer;
		threadData[i].scene = &this->scene;
		threadData[i].samplesPerPixel = samplesPerPixel;
		threadData[i].lightDepth = lightDepth;
		threadData[i].eyeDepth = eyeDepth;
		threadData[i].blurRadius = blurRadius;

		threadData[i].timeInitRendering = &this->timeInitRendering;

		pthread_create(&threads[i], NULL, renderThread, (void*)&threadData[i]);
	}

	// Wait for threads
	for (int i = 0; i < nRenderUnits; i++)
	{
		pthread_join(threads[i], NULL);
	}

	// Destroy everything
	pthread_mutex_destroy(&bucketList.bucketMutex);
	delete [] bucketList.buckets;
	delete [] threads;
	delete [] threadData;
}

void *BVH::renderThread(void *renderUnitInfo)
{
	BVHRenderUnitInfo *info = (BVHRenderUnitInfo*)renderUnitInfo;
	BVHRenderUnit *renderUnit = info->renderUnit;
	BVHBucketList *bucketList = info->bucketList;
	int localBucket;
	BVHgpu_stats statsBucket;
	BVHgpu_stats stats = {
		0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0,
		0.0, 0.0};
	struct timespec timeBucket;
	double timeDelta;

	if (renderUnit->mode == BVH_MODE_GPU)
	{
		// Init CUDA
		BVHcuda_init(renderUnit->deviceId);
	}

	while (1)
	{
		// Select the bucket to render
		pthread_mutex_lock(&bucketList->bucketMutex);
		localBucket = bucketList->bucketCurrent;
		bucketList->bucketCurrent++;
		pthread_mutex_unlock(&bucketList->bucketMutex);

		if (localBucket >= bucketList->bucketSize)
			break;

		// Do the render. Yeah, baby, yeah!
		if (renderUnit->mode == BVH_MODE_CPU)
		{
			BVHgpu_render(
				info->nPrimitives, info->primitiveLocations,
				info->primitiveSize, info->primitives,
				info->nNodes, info->nodesMin, info->nodesMax, info->nodesSkipLocation,
				info->width, info->height, info->renderBuffer, info->scene, info->samplesPerPixel,
				info->lightDepth, info->eyeDepth, info->blurRadius,
				renderUnit->nThreadsCPU, &bucketList->buckets[localBucket],
				&statsBucket);

			stats.timeTotal += statsBucket.timeTotal;
			stats.timeInit += statsBucket.timeInit;
			stats.timeGenRays += statsBucket.timeGenRays;
			stats.timeIntersection += statsBucket.timeIntersection;
			stats.timeFillBuffer += statsBucket.timeFillBuffer;
			stats.timeOthers += statsBucket.timeOthers;
		}
		else if (renderUnit->mode == BVH_MODE_GPU)
		{
			cu_BVHgpu_render(
				info->nPrimitives, info->primitiveLocations,
				info->primitiveSize, info->primitives,
				info->nNodes, info->nodesMin, info->nodesMax, info->nodesSkipLocation,
				info->width, info->height, info->renderBuffer, info->scene, info->samplesPerPixel,
				info->lightDepth, info->eyeDepth, info->blurRadius,
				renderUnit->nThreadsGPU, renderUnit->nThreadsCPU, &bucketList->buckets[localBucket],
				&statsBucket);

			stats.timeTotal += statsBucket.timeTotal;
			stats.timeCPU += statsBucket.timeCPU;
			stats.timeInit += statsBucket.timeInit;
			stats.timeGenRays += statsBucket.timeGenRays;
			stats.timeFillBuffer += statsBucket.timeFillBuffer;
			stats.timeOthers += statsBucket.timeOthers;
			stats.timeMemory += statsBucket.timeMemory;
			stats.timeAllocation += statsBucket.timeAllocation;
			stats.timeHostToDevice += statsBucket.timeHostToDevice;
			stats.timeBindTextures += statsBucket.timeBindTextures;
			stats.timeRaysToDevice += statsBucket.timeRaysToDevice;
			stats.timeRaysToHost += statsBucket.timeRaysToHost;
			stats.timeKernel += statsBucket.timeKernel;
		}

		if (BVH::verbose)
			cout << "Finished bucket " << (localBucket + 1) << " of " << bucketList->bucketSize << " by unit " << renderUnit->id << endl;

		clock_gettime(CLOCK_REALTIME, &timeBucket);
		timeDelta =
			(double)(timeBucket.tv_sec - info->timeInitRendering->tv_sec) +
			1E-9 * (double)(timeBucket.tv_nsec - info->timeInitRendering->tv_nsec);

		if (BVH::verbose)
		{
			cout << "Elapsed time: ";
			BVH::printTime(timeDelta);
			cout << ". Estimated remaining time: ";
			BVH::printTime(timeDelta * (bucketList->bucketSize - localBucket - 1) / (localBucket + 1));
			cout << endl;
		}
	}

	if (renderUnit->mode == BVH_MODE_GPU)
	{
		// Finish CUDA
		BVHcuda_finish();
	}

	// Print stats
	if (BVH::verbose)
	{
		cout << "Render unit " << renderUnit->id << " statistics:" << endl;

		if (renderUnit->mode == BVH_MODE_GPU)
		{
			cout << "Time total             : " << stats.timeTotal << endl;
			cout << "  Time CPU             : " << stats.timeCPU << "\t(" << (100.0 * stats.timeCPU / stats.timeTotal) << "%)" << endl;
			cout << "    Time init          : " << stats.timeInit << "\t(" << (100.0 * stats.timeInit / stats.timeTotal) << "%)" << endl;
			cout << "    Time gen rays      : " << stats.timeGenRays << "\t(" << (100.0 * stats.timeGenRays / stats.timeTotal) << "%)" << endl;
			cout << "    Time fill buffer   : " << stats.timeFillBuffer << "\t(" << (100.0 * stats.timeFillBuffer / stats.timeTotal) << "%)" << endl;
			cout << "    Time others        : " << stats.timeOthers << "\t(" << (100.0 * stats.timeOthers / stats.timeTotal) << "%)" << endl;
			cout << "  Time memory          : " << stats.timeMemory << "\t(" << (100.0 * stats.timeMemory / stats.timeTotal) << "%)" << endl;
			cout << "    Time allocation    : " << stats.timeAllocation << "\t(" << (100.0 * stats.timeAllocation / stats.timeTotal) << "%)" << endl;
			cout << "    Time host to device: " << stats.timeHostToDevice << "\t(" << (100.0 * stats.timeHostToDevice / stats.timeTotal) << "%)" << endl;
			cout << "    Time bind textures : " << stats.timeBindTextures  << "\t(" << (100.0 * stats.timeBindTextures / stats.timeTotal) << "%)" << endl;
			cout << "    Time rays to device: " << stats.timeRaysToDevice << "\t(" << (100.0 * stats.timeRaysToDevice / stats.timeTotal) << "%)" << endl;
			cout << "    Time rays to host  : " << stats.timeRaysToHost << "\t(" << (100.0 * stats.timeRaysToHost / stats.timeTotal) << "%)" << endl;
			cout << "  Time Kernel          : " << stats.timeKernel << "\t(" << (100.0 * stats.timeKernel / stats.timeTotal) << "%)" << endl;
			cout << "    Time parallel      : " << (stats.timeKernel + stats.timeGenRays) << "\t(" << (100.0 * (stats.timeKernel + stats.timeGenRays) / stats.timeTotal) << "%)" << endl;
		}
		else
		{
			cout << "Time total             : " << stats.timeTotal << endl;
			cout << "  Time init            : " << stats.timeInit << "\t(" << (100.0 * stats.timeInit / stats.timeTotal) << "%)" << endl;
			cout << "  Time gen rays        : " << stats.timeGenRays << "\t(" << (100.0 * stats.timeGenRays / stats.timeTotal) << "%)" << endl;
			cout << "  Time intersection    : " << stats.timeIntersection << "\t(" << (100.0 * stats.timeIntersection / stats.timeTotal) << "%)" << endl;
			cout << "  Time fill buffer     : " << stats.timeFillBuffer << "\t(" << (100.0 * stats.timeFillBuffer / stats.timeTotal) << "%)" << endl;
			cout << "  Time others          : " << stats.timeOthers << "\t(" << (100.0 * stats.timeOthers / stats.timeTotal) << "%)" << endl;
		}
	}

	return NULL;
}


	/* Print time */

void BVH::printTime(double time, int miliseconds)
{
	const double min2secs = 60.0;
	const double hour2secs = min2secs * 60.0;
	const double day2secs = hour2secs * 24.0;
	int days = time / day2secs;
	int hours = (time - days * day2secs) / hour2secs;
	int mins = (time - days * day2secs - hours * hour2secs) / min2secs;
	int secs = time - days * day2secs - hours * hour2secs - mins * min2secs;
	double secsMili = time - days * day2secs - hours * hour2secs - mins * min2secs;

	if (days > 0)
		cout << days << " days ";

	if (hours > 0)
		cout << hours << " hours ";

	if (mins > 0)
		cout << mins << " minutes and ";

	if (miliseconds)
		cout << secsMili << " seconds";
	else
		cout << secs << " seconds";
}


	/* Set verbose */

void BVH::setVerbose(int verbose)
{
	BVH::verbose = verbose;
}
