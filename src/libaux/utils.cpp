/*
 * utils.cpp
 *
 *  Created on: 22 Apr 2011
 *      Author: Jesus Ortiz
 *
 */

#include "utils.h"


/*
 * RANDOM NUMBERS
 */

float randFloat(gsl_rng *rng, float min, float max)
{
	return min + (max - min) * (float)gsl_rng_uniform_pos(rng);
}

float randFloat(void *rng, float min, float max)
{
	return min + (max - min) * (float)gsl_rng_uniform_pos((gsl_rng*)rng);
}

int randInt(gsl_rng *rng, int min, int max)
{
	return min + (int)((max - min) * (float)gsl_rng_uniform_pos((gsl_rng*)rng));
}


/*
 * OTHER FUNCTIONS
 */

int iDivUp(int a, int b)
{
    return ((a % b) != 0) ? (a / b + 1) : (a / b);
}
