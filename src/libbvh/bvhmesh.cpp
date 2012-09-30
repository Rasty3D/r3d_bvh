/*
 * bvhmesh.cpp
 *
 *  Created on: 10 May 2011
 *      Author: showroom
 */


/*
 * INCLUDES
 */

#include "bvhmesh.h"


/*
 * NAMESPACES
 */

using namespace std;


/*
 * CLASS: BVHMesh
 */

	/* Load a mesh from a file */

int BVHMesh::load(const char *filename, std::list<BVHPrimitive> &primitives, BVHShader *material)
{
	int filenameLen = strlen(filename);

	if (filenameLen < 4)
		return 0;

	if (filename[filenameLen - 4] == '.' &&
		(filename[filenameLen - 3] == 'o' || filename[filenameLen - 3] == 'O') &&
		(filename[filenameLen - 2] == 'b' || filename[filenameLen - 2] == 'B') &&
		(filename[filenameLen - 1] == 'j' || filename[filenameLen - 1] == 'J'))
		return BVHMesh::loadOBJ(filename, primitives, material);
	else if (filename[filenameLen - 4] == '.' &&
		(filename[filenameLen - 3] == 'r' || filename[filenameLen - 3] == 'R') &&
		(filename[filenameLen - 2] == '3') &&
		(filename[filenameLen - 1] == 'd' || filename[filenameLen - 1] == 'D'))
		return BVHMesh::loadR3D(filename, primitives, material);
	else if (filename[filenameLen - 4] == '.' &&
		(filename[filenameLen - 3] == 'v' || filename[filenameLen - 3] == 'V') &&
		(filename[filenameLen - 2] == 'b' || filename[filenameLen - 2] == 'B') &&
		(filename[filenameLen - 1] == 'o' || filename[filenameLen - 1] == 'O'))
		return BVHMesh::loadVBO(filename, primitives, material);
	else
		return 0;
}


	/* Load in different formats */

int BVHMesh::loadOBJ(const char *filename, std::list<BVHPrimitive> &primitives, BVHShader *material)
{
		/* Variables */

	// Vertex list
	std::list<BVHMesh_vertex> vertexList;
	std::list<BVHMesh_vertex>::iterator vertexIt;
	BVHMesh_vertex vertex;
	int vertexListSize = 0;

	// Vector version of the list
	int nVertex = 0;
	int vertexIndex;
	BVHMesh_vertex *vertices = NULL;

	// Texcoord list
	std::list<BVHMesh_texcoord> texcoordList;
	std::list<BVHMesh_texcoord>::iterator texcoordIt;
	BVHMesh_texcoord texcoord;
	int texcoordListSize = 0;

	// Vector version of the list
	int nTexcoord = 0;
	int texcoordIndex;
	BVHMesh_texcoord *texcoords = NULL;

	// Normal list
	std::list<BVHMesh_normal> normalList;
	std::list<BVHMesh_normal>::iterator normalIt;
	BVHMesh_normal normal;
	int normalListSize = 0;

	// Vector version of the list
	int nNormal = 0;
	int normalIndex;
	BVHMesh_normal *normals = NULL;

	// Face information
	int nIndices;
	int verticeIndices[BVHMESH_MAXINDICES];
	int texcoordIndices[BVHMESH_MAXINDICES];
	int normalIndices[BVHMESH_MAXINDICES];
	int nTriangles = 0;

	// File
	pktifstream file;
	char line[BVHMESH_LINESIZE];
	int numSpaces;
	int numBars;
	int lineLen;
	int doubleBars;
	int inSpace;

	// Primitive
	BVHPrimitive *primitive;
	int atlasType;
	float attribBuffer[(3 + 3 + 3) * 3];	// (3 pos, 3 texcoord, 3 normal) x 3 vertices


		/* Generate attrib atlas */

	BVHAttribAtlas atlas;
	BVHAttribAtlas *atlasVertex;
	BVHAttribAtlas *atlasVertexNormal;
	BVHAttribAtlas *atlasVertexTexcoord;
	BVHAttribAtlas *atlasVertexNormalTexcoord;

	// Only vertex
	atlas.number = 1;
	atlas.attrList = new BVHAttrib[atlas.number];
	atlas.attrList[0].type = BVHSHADER_TYPE_FLOAT;
	atlas.attrList[0].length = 3;
	atlas.attrList[0].offset = 0;
	atlas.attrList[0].size = sizeof(float) * atlas.attrList[0].length;
	atlas.attrList[0].name[0] = 'P';
	atlas.attrList[0].name[1] = '\0';
	atlas.attrList[0].name[2] = '\0';
	atlas.size = atlas.attrList[0].size;
	atlasVertex = BVHPrimitive::addAttribAtlas(atlas);

	// Vertex & normal
	atlas.number = 2;
	atlas.attrList = new BVHAttrib[atlas.number];
	atlas.attrList[0].type = BVHSHADER_TYPE_FLOAT;
	atlas.attrList[0].length = 3;
	atlas.attrList[0].offset = 0;
	atlas.attrList[0].size = sizeof(float) * atlas.attrList[0].length;
	atlas.attrList[0].name[0] = 'P';
	atlas.attrList[0].name[1] = '\0';
	atlas.attrList[0].name[2] = '\0';
	atlas.attrList[1].type = BVHSHADER_TYPE_FLOAT;
	atlas.attrList[1].length = 3;
	atlas.attrList[1].offset = atlas.attrList[0].offset + atlas.attrList[0].size;
	atlas.attrList[1].size = sizeof(float) * atlas.attrList[1].length;
	atlas.attrList[1].name[0] = 'N';
	atlas.attrList[1].name[1] = '\0';
	atlas.attrList[1].name[2] = '\0';
	atlas.size = atlas.attrList[0].size + atlas.attrList[1].size;
	atlasVertexNormal = BVHPrimitive::addAttribAtlas(atlas);

	// Vertex and texcoord
	atlas.number = 2;
	atlas.attrList = new BVHAttrib[atlas.number];
	atlas.attrList[0].type = BVHSHADER_TYPE_FLOAT;
	atlas.attrList[0].length = 3;
	atlas.attrList[0].offset = 0;
	atlas.attrList[0].size = sizeof(float) * atlas.attrList[0].length;
	atlas.attrList[0].name[0] = 'P';
	atlas.attrList[0].name[1] = '\0';
	atlas.attrList[0].name[2] = '\0';
	atlas.attrList[1].type = BVHSHADER_TYPE_FLOAT;
	atlas.attrList[1].length = 3;
	atlas.attrList[1].offset = atlas.attrList[0].offset + atlas.attrList[0].size;
	atlas.attrList[1].size = sizeof(float) * atlas.attrList[1].length;
	atlas.attrList[1].name[0] = 'T';
	atlas.attrList[1].name[1] = '\0';
	atlas.attrList[1].name[2] = '\0';
	atlas.size = atlas.attrList[0].size + atlas.attrList[1].size;
	atlasVertexTexcoord = BVHPrimitive::addAttribAtlas(atlas);

	// Vertex, normal & texcoord
	atlas.number = 3;
	atlas.attrList = new BVHAttrib[atlas.number];
	atlas.attrList[0].type = BVHSHADER_TYPE_FLOAT;
	atlas.attrList[0].length = 3;
	atlas.attrList[0].offset = 0;
	atlas.attrList[0].size = sizeof(float) * atlas.attrList[0].length;
	atlas.attrList[0].name[0] = 'P';
	atlas.attrList[0].name[1] = '\0';
	atlas.attrList[0].name[2] = '\0';
	atlas.attrList[1].type = BVHSHADER_TYPE_FLOAT;
	atlas.attrList[1].length = 3;
	atlas.attrList[1].offset = atlas.attrList[0].offset + atlas.attrList[0].size;
	atlas.attrList[1].size = sizeof(float) * atlas.attrList[1].length;
	atlas.attrList[1].name[0] = 'N';
	atlas.attrList[1].name[1] = '\0';
	atlas.attrList[1].name[2] = '\0';
	atlas.attrList[2].type = BVHSHADER_TYPE_FLOAT;
	atlas.attrList[2].length = 3;
	atlas.attrList[2].offset = atlas.attrList[1].offset + atlas.attrList[1].size;
	atlas.attrList[2].size = sizeof(float) * atlas.attrList[2].length;
	atlas.attrList[2].name[0] = 'T';
	atlas.attrList[2].name[1] = '\0';
	atlas.attrList[2].name[2] = '\0';
	atlas.size = atlas.attrList[0].size + atlas.attrList[1].size + atlas.attrList[2].size;
	atlasVertexNormalTexcoord = BVHPrimitive::addAttribAtlas(atlas);


		/* Open file */

	file.open(filename);

	if (!file)
		return 0;


		/* Parse file */

	while (!file.eof())
	{
		file.getline(line, BVHMESH_LINESIZE);

		// Check if it is a comment
		if (line[0] == '#')
			continue;

		// Check the type
		if (line[0] == 'v' && line[1] == ' ')
		{
			sscanf(&line[1], " %f %f %f", &vertex.x, &vertex.y, &vertex.z);
			vertexList.push_back(vertex);
			vertexListSize++;
		}
		else if (line[0] == 'v' && line[1] == 't' && line[2] == ' ')
		{
			sscanf(&line[2], " %f %f", &texcoord.x, &texcoord.y);
			texcoordList.push_back(texcoord);
			texcoordListSize++;
		}
		else if (line[0] == 'v' && line[1] == 'n' && line[2] == ' ')
		{
			sscanf(&line[2], " %f %f %f", &normal.x, &normal.y, &normal.z);
			normalList.push_back(normal);
			normalListSize++;
		}
		else if (line[0] == 'f' && line[1] == ' ')
		{
			// Remade vertex list if necessary
			if (nVertex != vertexListSize)
			{
				if (nVertex > 0)
					delete [] vertices;

				nVertex = vertexListSize;
				vertices = new BVHMesh_vertex[nVertex];
				vertexIndex = 0;

				for (vertexIt = vertexList.begin(); vertexIt != vertexList.end(); vertexIt++)
				{
					vertices[vertexIndex] = (*vertexIt);
					vertexIndex++;
				}
			}

			// Remade texcoord list if necessary
			if (nTexcoord != texcoordListSize)
			{
				if (nTexcoord > 0)
					delete [] texcoords;

				nTexcoord = texcoordListSize;
				texcoords = new BVHMesh_texcoord[nTexcoord];
				texcoordIndex = 0;

				for (texcoordIt = texcoordList.begin(); texcoordIt != texcoordList.end(); texcoordIt++)
				{
					texcoords[texcoordIndex] = (*texcoordIt);
					texcoordIndex++;
				}
			}

			// Remade normal list if necessary
			if (nNormal != normalListSize)
			{
				if (nNormal > 0)
					delete [] normals;

				nNormal = normalListSize;
				normals = new BVHMesh_normal[nNormal];
				normalIndex = 0;

				for (normalIt = normalList.begin(); normalIt != normalList.end(); normalIt++)
				{
					normals[normalIndex] = (*normalIt);
					normalIndex++;
				}
			}

			// Read list of vertices
			nIndices = 0;
			lineLen = strlen(line);
			doubleBars = 0;
			inSpace = 0;
			numSpaces = 0;
			numBars = 0;

			for (int i = 1; i < lineLen; i++)
			{
				// Count spaces
				if (line[i] == ' ' || line[i] == '\t')
				{
					if (!inSpace)
					{
						numSpaces++;
						inSpace = 1;
					}
				}
				else
				{
					inSpace = 0;
				}

				// Count bars
				if (line[i] == '/')
					numBars++;

				// Check if there are double bars
				if (i < lineLen && line[i] == '/' && line[i + 1] == '/')
					doubleBars = 1;
			}

			// Ignore faces with more than 4 vertices
			if (numSpaces != 3 && numSpaces != 4)
				continue;

			nIndices = numSpaces;

			if (numBars == 0)									// Only vertex
			{
				if (numSpaces == 3)
				{
					sscanf(
						&line[1], " %d %d %d",
						&verticeIndices[0],
						&verticeIndices[1],
						&verticeIndices[2]);
				}
				else if (numSpaces == 4)
				{
					sscanf(
						&line[1], " %d %d %d %d",
						&verticeIndices[0],
						&verticeIndices[1],
						&verticeIndices[2],
						&verticeIndices[3]);
				}

				atlasType = BVHMESH_ATLAS_VERTEX;
			}
			else if (numBars == numSpaces && !doubleBars)		// Vertex and texcoords
			{
				if (numSpaces == 3)
				{
					sscanf(
						&line[1], " %d/%d %d/%d %d/%d",
						&verticeIndices[0], &texcoordIndices[0],
						&verticeIndices[1], &texcoordIndices[1],
						&verticeIndices[2], &texcoordIndices[2]);
				}
				else if (numSpaces == 4)
				{
					sscanf(
						&line[1], " %d/%d %d/%d %d/%d %d/%d",
						&verticeIndices[0], &texcoordIndices[0],
						&verticeIndices[1], &texcoordIndices[1],
						&verticeIndices[2], &texcoordIndices[2],
						&verticeIndices[3], &texcoordIndices[3]);
				}

				atlasType = BVHMESH_ATLAS_VERTEX_TEXCOORD;
			}
			else if (numBars == 2 * numSpaces && !doubleBars)	// Vertex, texcoord, normal
			{
				if (numSpaces == 3)
				{
					sscanf(
						&line[1], " %d/%d/%d %d/%d/%d %d/%d/%d",
						&verticeIndices[0], &texcoordIndices[0], &normalIndices[0],
						&verticeIndices[1], &texcoordIndices[1], &normalIndices[1],
						&verticeIndices[2], &texcoordIndices[2], &normalIndices[2]);
				}
				else if (numSpaces == 4)
				{
					sscanf(
						&line[1], " %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
						&verticeIndices[0], &texcoordIndices[0], &normalIndices[0],
						&verticeIndices[1], &texcoordIndices[1], &normalIndices[1],
						&verticeIndices[2], &texcoordIndices[2], &normalIndices[2],
						&verticeIndices[3], &texcoordIndices[3], &normalIndices[3]);
				}

				atlasType = BVHMESH_ATLAS_VERTEX_TEXCOORD_NORMAL;
			}
			else if (numBars == 2 * numSpaces && doubleBars)	// Vertex, normal
			{
				if (numSpaces == 3)
				{
					sscanf(
						&line[1], " %d//%d %d//%d %d//%d",
						&verticeIndices[0], &normalIndices[0],
						&verticeIndices[1], &normalIndices[1],
						&verticeIndices[2], &normalIndices[2]);
				}
				else if (numSpaces == 4)
				{
					sscanf(
						&line[1], " %d//%d %d//%d %d//%d %d//%d",
						&verticeIndices[0], &normalIndices[0],
						&verticeIndices[1], &normalIndices[1],
						&verticeIndices[2], &normalIndices[2],
						&verticeIndices[3], &normalIndices[3]);
				}

				atlasType = BVHMESH_ATLAS_VERTEX_NORMAL;
			}
			else
			{
				// Ignoring line
				continue;
			}

			verticeIndices[0]--;
			verticeIndices[1]--;
			verticeIndices[2]--;
			verticeIndices[3]--;
			texcoordIndices[0]--;
			texcoordIndices[1]--;
			texcoordIndices[2]--;
			texcoordIndices[3]--;
			normalIndices[0]--;
			normalIndices[1]--;
			normalIndices[2]--;
			normalIndices[3]--;

			// Generate primitives
			if (nIndices == 3)
			{
				if (atlasType == BVHMESH_ATLAS_VERTEX)
				{
					attribBuffer[0] = vertices[verticeIndices[0]].x;
					attribBuffer[1] = vertices[verticeIndices[0]].y;
					attribBuffer[2] = vertices[verticeIndices[0]].z;
					attribBuffer[3] = vertices[verticeIndices[1]].x;
					attribBuffer[4] = vertices[verticeIndices[1]].y;
					attribBuffer[5] = vertices[verticeIndices[1]].z;
					attribBuffer[6] = vertices[verticeIndices[2]].x;
					attribBuffer[7] = vertices[verticeIndices[2]].y;
					attribBuffer[8] = vertices[verticeIndices[2]].z;

					primitive = new BVHPrimitive;
					primitive->init(BVHPRIMITIVE_TYPE_TRIANGLE, atlasVertex, 3, (char*)attribBuffer);
					primitive->setMaterial(material);
					primitives.push_back(*primitive);
				}
				else if (atlasType == BVHMESH_ATLAS_VERTEX_NORMAL)
				{
					attribBuffer[0] = vertices[verticeIndices[0]].x;
					attribBuffer[1] = vertices[verticeIndices[0]].y;
					attribBuffer[2] = vertices[verticeIndices[0]].z;
					attribBuffer[3] = normals[normalIndices[0]].x;
					attribBuffer[4] = normals[normalIndices[0]].y;
					attribBuffer[5] = normals[normalIndices[0]].z;
					attribBuffer[6] = vertices[verticeIndices[1]].x;
					attribBuffer[7] = vertices[verticeIndices[1]].y;
					attribBuffer[8] = vertices[verticeIndices[1]].z;
					attribBuffer[9] = normals[normalIndices[1]].x;
					attribBuffer[10] = normals[normalIndices[1]].y;
					attribBuffer[11] = normals[normalIndices[1]].z;
					attribBuffer[12] = vertices[verticeIndices[2]].x;
					attribBuffer[13] = vertices[verticeIndices[2]].y;
					attribBuffer[14] = vertices[verticeIndices[2]].z;
					attribBuffer[15] = normals[normalIndices[2]].x;
					attribBuffer[16] = normals[normalIndices[2]].y;
					attribBuffer[17] = normals[normalIndices[2]].z;

					primitive = new BVHPrimitive;
					primitive->init(BVHPRIMITIVE_TYPE_TRIANGLE, atlasVertexNormal, 3, (char*)attribBuffer);
					primitive->setMaterial(material);
					primitives.push_back(*primitive);
				}
				else if (atlasType == BVHMESH_ATLAS_VERTEX_TEXCOORD)
				{
					attribBuffer[0] = vertices[verticeIndices[0]].x;
					attribBuffer[1] = vertices[verticeIndices[0]].y;
					attribBuffer[2] = vertices[verticeIndices[0]].z;
					attribBuffer[3] = texcoords[texcoordIndices[0]].x;
					attribBuffer[4] = texcoords[texcoordIndices[0]].y;
					attribBuffer[5] = 0.0f;
					attribBuffer[6] = vertices[verticeIndices[1]].x;
					attribBuffer[7] = vertices[verticeIndices[1]].y;
					attribBuffer[8] = vertices[verticeIndices[1]].z;
					attribBuffer[9] = texcoords[texcoordIndices[1]].x;
					attribBuffer[10] = texcoords[texcoordIndices[1]].y;
					attribBuffer[11] = 0.0f;
					attribBuffer[12] = vertices[verticeIndices[2]].x;
					attribBuffer[13] = vertices[verticeIndices[2]].y;
					attribBuffer[14] = vertices[verticeIndices[2]].z;
					attribBuffer[15] = texcoords[texcoordIndices[2]].x;
					attribBuffer[16] = texcoords[texcoordIndices[2]].y;
					attribBuffer[17] = 0.0f;

					primitive = new BVHPrimitive;
					primitive->init(BVHPRIMITIVE_TYPE_TRIANGLE, atlasVertexTexcoord, 3, (char*)attribBuffer);
					primitive->setMaterial(material);
					primitives.push_back(*primitive);
				}
				else if (atlasType == BVHMESH_ATLAS_VERTEX_TEXCOORD_NORMAL)
				{
					attribBuffer[0] = vertices[verticeIndices[0]].x;
					attribBuffer[1] = vertices[verticeIndices[0]].y;
					attribBuffer[2] = vertices[verticeIndices[0]].z;
					attribBuffer[3] = normals[normalIndices[0]].x;
					attribBuffer[4] = normals[normalIndices[0]].y;
					attribBuffer[5] = normals[normalIndices[0]].z;
					attribBuffer[6] = texcoords[texcoordIndices[0]].x;
					attribBuffer[7] = texcoords[texcoordIndices[0]].y;
					attribBuffer[8] = 0.0f;
					attribBuffer[9] = vertices[verticeIndices[1]].x;
					attribBuffer[10] = vertices[verticeIndices[1]].y;
					attribBuffer[11] = vertices[verticeIndices[1]].z;
					attribBuffer[12] = normals[normalIndices[1]].x;
					attribBuffer[13] = normals[normalIndices[1]].y;
					attribBuffer[14] = normals[normalIndices[1]].z;
					attribBuffer[15] = texcoords[texcoordIndices[1]].x;
					attribBuffer[16] = texcoords[texcoordIndices[1]].y;
					attribBuffer[17] = 0.0f;
					attribBuffer[18] = vertices[verticeIndices[2]].x;
					attribBuffer[19] = vertices[verticeIndices[2]].y;
					attribBuffer[20] = vertices[verticeIndices[2]].z;
					attribBuffer[21] = normals[normalIndices[2]].x;
					attribBuffer[22] = normals[normalIndices[2]].y;
					attribBuffer[23] = normals[normalIndices[2]].z;
					attribBuffer[24] = texcoords[texcoordIndices[2]].x;
					attribBuffer[25] = texcoords[texcoordIndices[2]].y;
					attribBuffer[26] = 0.0f;

					primitive = new BVHPrimitive;
					primitive->init(BVHPRIMITIVE_TYPE_TRIANGLE, atlasVertexNormalTexcoord, 3, (char*)attribBuffer);
					primitive->setMaterial(material);
					primitives.push_back(*primitive);
				}

				nTriangles++;
			}
			else if (nIndices == 4)
			{
				if (atlasType == BVHMESH_ATLAS_VERTEX)
				{
					attribBuffer[0] = vertices[verticeIndices[0]].x;
					attribBuffer[1] = vertices[verticeIndices[0]].y;
					attribBuffer[2] = vertices[verticeIndices[0]].z;
					attribBuffer[3] = vertices[verticeIndices[1]].x;
					attribBuffer[4] = vertices[verticeIndices[1]].y;
					attribBuffer[5] = vertices[verticeIndices[1]].z;
					attribBuffer[6] = vertices[verticeIndices[2]].x;
					attribBuffer[7] = vertices[verticeIndices[2]].y;
					attribBuffer[8] = vertices[verticeIndices[2]].z;

					primitive = new BVHPrimitive;
					primitive->init(BVHPRIMITIVE_TYPE_TRIANGLE, atlasVertex, 3, (char*)attribBuffer);
					primitive->setMaterial(material);
					primitives.push_back(*primitive);
				}
				else if (atlasType == BVHMESH_ATLAS_VERTEX_NORMAL)
				{
					attribBuffer[0] = vertices[verticeIndices[0]].x;
					attribBuffer[1] = vertices[verticeIndices[0]].y;
					attribBuffer[2] = vertices[verticeIndices[0]].z;
					attribBuffer[3] = normals[normalIndices[0]].x;
					attribBuffer[4] = normals[normalIndices[0]].y;
					attribBuffer[5] = normals[normalIndices[0]].z;
					attribBuffer[6] = vertices[verticeIndices[1]].x;
					attribBuffer[7] = vertices[verticeIndices[1]].y;
					attribBuffer[8] = vertices[verticeIndices[1]].z;
					attribBuffer[9] = normals[normalIndices[1]].x;
					attribBuffer[10] = normals[normalIndices[1]].y;
					attribBuffer[11] = normals[normalIndices[1]].z;
					attribBuffer[12] = vertices[verticeIndices[2]].x;
					attribBuffer[13] = vertices[verticeIndices[2]].y;
					attribBuffer[14] = vertices[verticeIndices[2]].z;
					attribBuffer[15] = normals[normalIndices[2]].x;
					attribBuffer[16] = normals[normalIndices[2]].y;
					attribBuffer[17] = normals[normalIndices[2]].z;

					primitive = new BVHPrimitive;
					primitive->init(BVHPRIMITIVE_TYPE_TRIANGLE, atlasVertexNormal, 3, (char*)attribBuffer);
					primitive->setMaterial(material);
					primitives.push_back(*primitive);
				}
				else if (atlasType == BVHMESH_ATLAS_VERTEX_TEXCOORD)
				{
					attribBuffer[0] = vertices[verticeIndices[0]].x;
					attribBuffer[1] = vertices[verticeIndices[0]].y;
					attribBuffer[2] = vertices[verticeIndices[0]].z;
					attribBuffer[3] = texcoords[texcoordIndices[0]].x;
					attribBuffer[4] = texcoords[texcoordIndices[0]].y;
					attribBuffer[5] = 0.0f;
					attribBuffer[6] = vertices[verticeIndices[1]].x;
					attribBuffer[7] = vertices[verticeIndices[1]].y;
					attribBuffer[8] = vertices[verticeIndices[1]].z;
					attribBuffer[9] = texcoords[texcoordIndices[1]].x;
					attribBuffer[10] = texcoords[texcoordIndices[1]].y;
					attribBuffer[11] = 0.0f;
					attribBuffer[12] = vertices[verticeIndices[2]].x;
					attribBuffer[13] = vertices[verticeIndices[2]].y;
					attribBuffer[14] = vertices[verticeIndices[2]].z;
					attribBuffer[15] = texcoords[texcoordIndices[2]].x;
					attribBuffer[16] = texcoords[texcoordIndices[2]].y;
					attribBuffer[17] = 0.0f;

					primitive = new BVHPrimitive;
					primitive->init(BVHPRIMITIVE_TYPE_TRIANGLE, atlasVertexTexcoord, 3, (char*)attribBuffer);
					primitive->setMaterial(material);
					primitives.push_back(*primitive);
				}
				else if (atlasType == BVHMESH_ATLAS_VERTEX_TEXCOORD_NORMAL)
				{
					attribBuffer[0] = vertices[verticeIndices[0]].x;
					attribBuffer[1] = vertices[verticeIndices[0]].y;
					attribBuffer[2] = vertices[verticeIndices[0]].z;
					attribBuffer[3] = normals[normalIndices[0]].x;
					attribBuffer[4] = normals[normalIndices[0]].y;
					attribBuffer[5] = normals[normalIndices[0]].z;
					attribBuffer[6] = texcoords[texcoordIndices[0]].x;
					attribBuffer[7] = texcoords[texcoordIndices[0]].y;
					attribBuffer[8] = 0.0f;
					attribBuffer[9] = vertices[verticeIndices[1]].x;
					attribBuffer[10] = vertices[verticeIndices[1]].y;
					attribBuffer[11] = vertices[verticeIndices[1]].z;
					attribBuffer[12] = normals[normalIndices[1]].x;
					attribBuffer[13] = normals[normalIndices[1]].y;
					attribBuffer[14] = normals[normalIndices[1]].z;
					attribBuffer[15] = texcoords[texcoordIndices[1]].x;
					attribBuffer[16] = texcoords[texcoordIndices[1]].y;
					attribBuffer[17] = 0.0f;
					attribBuffer[18] = vertices[verticeIndices[2]].x;
					attribBuffer[19] = vertices[verticeIndices[2]].y;
					attribBuffer[20] = vertices[verticeIndices[2]].z;
					attribBuffer[21] = normals[normalIndices[2]].x;
					attribBuffer[22] = normals[normalIndices[2]].y;
					attribBuffer[23] = normals[normalIndices[2]].z;
					attribBuffer[24] = texcoords[texcoordIndices[2]].x;
					attribBuffer[25] = texcoords[texcoordIndices[2]].y;
					attribBuffer[26] = 0.0f;

					primitive = new BVHPrimitive;
					primitive->init(BVHPRIMITIVE_TYPE_TRIANGLE, atlasVertexNormalTexcoord, 3, (char*)attribBuffer);
					primitive->setMaterial(material);
					primitives.push_back(*primitive);
				}

				if (atlasType == BVHMESH_ATLAS_VERTEX)
				{
					attribBuffer[0] = vertices[verticeIndices[0]].x;
					attribBuffer[1] = vertices[verticeIndices[0]].y;
					attribBuffer[2] = vertices[verticeIndices[0]].z;
					attribBuffer[3] = vertices[verticeIndices[2]].x;
					attribBuffer[4] = vertices[verticeIndices[2]].y;
					attribBuffer[5] = vertices[verticeIndices[2]].z;
					attribBuffer[6] = vertices[verticeIndices[3]].x;
					attribBuffer[7] = vertices[verticeIndices[3]].y;
					attribBuffer[8] = vertices[verticeIndices[3]].z;

					primitive = new BVHPrimitive;
					primitive->init(BVHPRIMITIVE_TYPE_TRIANGLE, atlasVertex, 3, (char*)attribBuffer);
					primitive->setMaterial(material);
					primitives.push_back(*primitive);
				}
				else if (atlasType == BVHMESH_ATLAS_VERTEX_NORMAL)
				{
					attribBuffer[0] = vertices[verticeIndices[0]].x;
					attribBuffer[1] = vertices[verticeIndices[0]].y;
					attribBuffer[2] = vertices[verticeIndices[0]].z;
					attribBuffer[3] = normals[normalIndices[0]].x;
					attribBuffer[4] = normals[normalIndices[0]].y;
					attribBuffer[5] = normals[normalIndices[0]].z;
					attribBuffer[6] = vertices[verticeIndices[2]].x;
					attribBuffer[7] = vertices[verticeIndices[2]].y;
					attribBuffer[8] = vertices[verticeIndices[2]].z;
					attribBuffer[9] = normals[normalIndices[2]].x;
					attribBuffer[10] = normals[normalIndices[2]].y;
					attribBuffer[11] = normals[normalIndices[2]].z;
					attribBuffer[12] = vertices[verticeIndices[3]].x;
					attribBuffer[13] = vertices[verticeIndices[3]].y;
					attribBuffer[14] = vertices[verticeIndices[3]].z;
					attribBuffer[15] = normals[normalIndices[3]].x;
					attribBuffer[16] = normals[normalIndices[3]].y;
					attribBuffer[17] = normals[normalIndices[3]].z;

					primitive = new BVHPrimitive;
					primitive->init(BVHPRIMITIVE_TYPE_TRIANGLE, atlasVertexNormal, 3, (char*)attribBuffer);
					primitive->setMaterial(material);
					primitives.push_back(*primitive);
				}
				else if (atlasType == BVHMESH_ATLAS_VERTEX_TEXCOORD)
				{
					attribBuffer[0] = vertices[verticeIndices[0]].x;
					attribBuffer[1] = vertices[verticeIndices[0]].y;
					attribBuffer[2] = vertices[verticeIndices[0]].z;
					attribBuffer[3] = texcoords[texcoordIndices[0]].x;
					attribBuffer[4] = texcoords[texcoordIndices[0]].y;
					attribBuffer[5] = 0.0f;
					attribBuffer[6] = vertices[verticeIndices[2]].x;
					attribBuffer[7] = vertices[verticeIndices[2]].y;
					attribBuffer[8] = vertices[verticeIndices[2]].z;
					attribBuffer[9] = texcoords[texcoordIndices[2]].x;
					attribBuffer[10] = texcoords[texcoordIndices[2]].y;
					attribBuffer[11] = 0.0f;
					attribBuffer[12] = vertices[verticeIndices[3]].x;
					attribBuffer[13] = vertices[verticeIndices[3]].y;
					attribBuffer[14] = vertices[verticeIndices[3]].z;
					attribBuffer[15] = texcoords[texcoordIndices[3]].x;
					attribBuffer[16] = texcoords[texcoordIndices[3]].y;
					attribBuffer[17] = 0.0f;

					primitive = new BVHPrimitive;
					primitive->init(BVHPRIMITIVE_TYPE_TRIANGLE, atlasVertexTexcoord, 3, (char*)attribBuffer);
					primitive->setMaterial(material);
					primitives.push_back(*primitive);
				}
				else if (atlasType == BVHMESH_ATLAS_VERTEX_TEXCOORD_NORMAL)
				{
					attribBuffer[0] = vertices[verticeIndices[0]].x;
					attribBuffer[1] = vertices[verticeIndices[0]].y;
					attribBuffer[2] = vertices[verticeIndices[0]].z;
					attribBuffer[3] = normals[normalIndices[0]].x;
					attribBuffer[4] = normals[normalIndices[0]].y;
					attribBuffer[5] = normals[normalIndices[0]].z;
					attribBuffer[6] = texcoords[texcoordIndices[0]].x;
					attribBuffer[7] = texcoords[texcoordIndices[0]].y;
					attribBuffer[8] = 0.0f;
					attribBuffer[9] = vertices[verticeIndices[2]].x;
					attribBuffer[10] = vertices[verticeIndices[2]].y;
					attribBuffer[11] = vertices[verticeIndices[2]].z;
					attribBuffer[12] = normals[normalIndices[2]].x;
					attribBuffer[13] = normals[normalIndices[2]].y;
					attribBuffer[14] = normals[normalIndices[2]].z;
					attribBuffer[15] = texcoords[texcoordIndices[2]].x;
					attribBuffer[16] = texcoords[texcoordIndices[2]].y;
					attribBuffer[17] = 0.0f;
					attribBuffer[18] = vertices[verticeIndices[3]].x;
					attribBuffer[19] = vertices[verticeIndices[3]].y;
					attribBuffer[20] = vertices[verticeIndices[3]].z;
					attribBuffer[21] = normals[normalIndices[3]].x;
					attribBuffer[22] = normals[normalIndices[3]].y;
					attribBuffer[23] = normals[normalIndices[3]].z;
					attribBuffer[24] = texcoords[texcoordIndices[3]].x;
					attribBuffer[25] = texcoords[texcoordIndices[3]].y;
					attribBuffer[26] = 0.0f;

					primitive = new BVHPrimitive;
					primitive->init(BVHPRIMITIVE_TYPE_TRIANGLE, atlasVertexNormalTexcoord, 3, (char*)attribBuffer);
					primitive->setMaterial(material);
					primitives.push_back(*primitive);
				}

				nTriangles += 2;
			}
			else
			{
				// Ignoring
			}
		}
		else
		{
			// Ignoring
		}
	}


		/* Close file */

	file.close();


		/* Destroy things */

	if (nVertex > 0)
		delete [] vertices;


		/* Return ok */

	return 1;
}

int BVHMesh::loadR3D(const char *filename, std::list<BVHPrimitive> &primitives, BVHShader *material)
{
	// TODO
	return 0;
}

int BVHMesh::loadVBO(const char *filename, std::list<BVHPrimitive> &primitives, BVHShader *material)
{
	// TODO
	return 0;
}
