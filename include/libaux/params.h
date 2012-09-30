/*
 * params.h
 *
 *  Created on: 23-June-2009
 *      Author: Jesus Ortiz
 *
 *  Istituto Italiano di Tecnologia (IIT)
 *
 */

#ifndef _PARAMS_H_
#define _PARAMS_H_

#include <stdlib.h>
#include <string.h>
#include <iostream>

// Param type
#define PARAM_TYPE_INT		0
#define PARAM_TYPE_FLOAT	1
#define PARAM_TYPE_STRING	2
#define PARAM_TYPE_FLAG		3

// Param lenghts
#define PARAM_NAME_LENGHT			64
#define PARAM_OPTION_SHORT_LENGHT	8
#define PARAM_OPTION_LONG_LENGHT	64
#define PARAM_DESCRIPTION_LENGHT	256
#define PARAM_VALUE_LENGHT			256

typedef struct
{
	int type;

	char value[PARAM_VALUE_LENGHT];

	char name[PARAM_NAME_LENGHT];

	char optionShort[PARAM_OPTION_SHORT_LENGHT];
	char optionLong[PARAM_OPTION_LONG_LENGHT];

	char description[PARAM_DESCRIPTION_LENGHT];

	int mandatory;
}Param;

class ParamList
{
private:
	int nParams;
	Param *params;

public:
	ParamList();
	~ParamList();

private:
	int addParam(Param param);

	int addParam(
		int type,
		const char *defaultValue,
		const char *name,
		const char *optionShort,
		const char *optionLong,
		const char *description,
		int mandatory);

public:
	int addParamFlag(
		const char *name,
		const char *optionShort,
		const char *optionLong,
		const char *description);

	int addParamInt(
		const char *name,
		const char *optionShort,
		const char *optionLong,
		const char *description,
		int defaultValue,
		int mandatory);

	int addParamFloat(
		const char *name,
		const char *optionShort,
		const char *optionLong,
		const char *description,
		float defaultValue,
		int mandatory);

	int addParamString(
		const char *name,
		const char *optionShort,
		const char *optionLong,
		const char *description,
		const char *defaultValue,
		int mandatory);

	int printUsage();

	int fillValues(int argc, char *argv[]);

	int checkMandatoryParams();

	int getValue(const char *name, char *value);
};

#endif // _PARAMS_H_
