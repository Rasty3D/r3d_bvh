/*
 * path.cpp
 *
 *   Created on: 03-Sep-2010
 *       Author: Jesus Ortiz
 *
 *  Description: TODO
 *
 */

#include "path.h"

int getPath(const char *input, char *path)
{
	int lenghtInput = strlen(input);

	if (lenghtInput <= 0)
		return 0;

	int position = 0;
	for (int i = lenghtInput - 1; i >= 0; i--)
	{
		if (input[i] == '\\' || input[i] == '/')
		{
			position = i;
			break;
		}
	}

	if (position == 0)
	{
		path[0] = '.';
		path[1] = '\0';
	}
	else
	{
		for (int i = 0; i < position; i++)
			path[i] = input[i];
		path[position] = '\0';
	}

	return 1;
}

int addPath(char *line, const char *path)
{
	int lineLenght = strlen(line);

	if (lineLenght <= 0)
		return 0;

	char *auxLine = new char[lineLenght + 1];
	strcpy(auxLine, line);

	sprintf(line, "%s/%s", path, auxLine);

	delete [] auxLine;

	fixPath(line);

	return 1;
}

int fixPath(char *path)
{
	// Variables
	char *tokens[32];
	int nTokens = 0;
	int basepos = -1;
	char *auxPath;
	int pathLen = strlen(path);

	if (pathLen <= 0)
		return 0;

	auxPath = new char[pathLen + 1];
	strcpy(auxPath, path);

	for (int i = 0; i < pathLen; i++)
	{
		if ((path[i] == '/' && i == pathLen - 1) ||
			(path[i] == '/' && path[i + 1] != '/'))
		{
			auxPath[i] = '\0';

			if (basepos != -1 && nTokens > 0)
			{
				tokens[nTokens - 1] = &auxPath[basepos];
			}

			basepos = -1;
		}
		else
		{
			if (basepos == -1)
			{
				basepos = i;
				nTokens++;
			}

			auxPath[i] = path[i];
		}
	}

	if (path[pathLen - 1] != '/' && basepos != -1 && nTokens > 0)
		tokens[nTokens - 1] = &auxPath[basepos];

	// Remove .
	for (int i = 0; i < nTokens; i++)
	{
		if (strcmp(tokens[i], ".") == 0)
		{
			for (int j = i + 1; j < nTokens; j++)
			{
				tokens[j - 1] = tokens[j];
			}
			nTokens--;
		}
	}

	// Remove ..
	for (int i = 1; i < nTokens; i++)
	{
		if (strcmp(tokens[i], "..") == 0)
		{
			for (int j = i - 1; j < nTokens - 2; j++)
			{
				tokens[j] = tokens[j + 2];
			}
			nTokens -= 2;
		}
	}

	// Copy auxPath to path
	path[0] = '\0';
	for (int i = 0; i < nTokens; i++)
	{
		strcat(path, tokens[i]);

		if (i < nTokens - 1)
			strcat(path, "/");
	}

	return 1;
}

int removePath(char *line, const char *path)
{
	char aux[256];
	int lenLine = strlen(line);
	int lenPath = strlen(path);
	int lenMin;
	int lenAux;
	int startingPoint;

	if (lenLine < lenPath)
		lenMin = lenLine;
	else
		lenMin = lenPath;

	startingPoint = lenMin;

	for (int i = 0; i < lenMin; i++)
	{
		if (line[i] != path[i])
		{
			startingPoint = i;
			break;
		}
	}

	if (startingPoint != 0 &&
		(line[startingPoint] == '\\' || line[startingPoint] == '/'))
	{
		startingPoint++;
	}

	lenAux = lenLine - startingPoint;

	for (int i = 0; i < lenAux; i++)
	{
		aux[i] = line[startingPoint + i];
	}

	aux[lenAux] = '\0';

	strcpy(line, aux);

	return 1;
}
