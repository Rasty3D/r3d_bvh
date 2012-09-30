/*
 * path.h
 *
 *   Created on: 03-Sep-2010
 *       Author: Jesus Ortiz
 *
 *  Description: TODO
 *
 */

#ifndef PATH_H_
#define PATH_H_

#include <stdio.h>
#include <string.h>

int getPath(const char *file, char *path);
int addPath(char *line, const char *path);
int fixPath(char *path);
int removePath(char *line, const char *path);

#endif /* PATH_H_ */
