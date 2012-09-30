/*
 * utils.h
 *
 *  Created on: 22 Apr 2011
 *      Author: Jesus Ortiz
 *
 */

#ifndef _UTILS_H_
#define _UTILS_H_

/*
 * INCLUDES
 */

#include <stdlib.h>
#include <gsl/gsl_rng.h>


/*
 * RANDOM NUMBERS
 */

float randFloat(gsl_rng *rng, float min, float max);
float randFloat(void *rng, float min, float max);
int randInt(gsl_rng *rng, int min, int max);


/*
 * OTHER FUNCTIONS
 */

int iDivUp(int a, int b);

#endif // _UTILS_H_
