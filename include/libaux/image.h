/*
 * image.h
 *
 *  Created on: 15 Feb 2011
 *      Author: showroom
 */

#ifndef IMAGE_H_
#define IMAGE_H_

/*
 * INCLUDES
 */

#include <stdlib.h>
#include <stdio.h>
#include <png.h>


/*
 * FUNCTIONS
 */

	/* PNG */
int savePNG(const char *filename, int width, int height, unsigned char *buffer);

#endif /* IMAGE_H_ */
