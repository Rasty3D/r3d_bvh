/*
 * image.cpp
 *
 *  Created on: 15 Feb 2011
 *      Author: showroom
 */


/*
 * INCLUDES
 */

#include "image.h"


/*
 * FUNCTIONS
 */

	/* PNG functions */

int savePNG(const char *filename, int width, int height, unsigned char *buffer)
{
		/* Variables */

	FILE *file;
	png_structp png_ptr;
	png_infop info_ptr;


        /* Create file */

	if (!(file = fopen(filename, "wb")))
		return 0;


		/* Initialize stuff */

	if (!(png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)))
		return 0;

	if (!(info_ptr = png_create_info_struct(png_ptr)))
		return 0;

	png_init_io(png_ptr, file);


		/* Write header */

	png_set_IHDR(
		png_ptr, info_ptr, width, height,
		8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(png_ptr, info_ptr);


		/* Write bytes */

	for (int i = 0; i < height; i++)
		png_write_row(png_ptr, &buffer[(height - i - 1) * width * 3]);


		/* End write */

	png_write_end(png_ptr, NULL);


		/* Close file */

	fclose(file);


		/* Return ok */

	return 1;
}
