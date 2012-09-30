/*
 * params.cpp
 *
 *  Created on: 23-June-2009
 *      Author: Jes�s Ortiz
 *
 *  Istituto Italiano di Tecnologia (IIT)
 *
 */

#include "params.h"

using namespace std;

ParamList::ParamList()
{
	this->nParams = 0;
	this->params = NULL;
}

ParamList::~ParamList()
{
	if (this->nParams > 0)
		delete [] this->params;
}

int ParamList::addParam(Param param)
{
	if (this->nParams == 0)
	{
		this->nParams = 1;
		this->params = new Param[this->nParams];
		memcpy(this->params, &param, sizeof(Param));
	}
	else
	{
		Param *aux = new Param[this->nParams + 1];
		memcpy(aux, this->params, sizeof(Param) * this->nParams);
		memcpy(&aux[this->nParams], &param, sizeof(Param));
		delete [] this->params;
		this->params = aux;
		this->nParams++;
	}

	return 1;
}

int ParamList::addParam(
	int type,
	const char *defaultValue,
	const char *name,
	const char *optionShort,
	const char *optionLong,
	const char *description,
	int mandatory)
{
	Param param;

	// Param type
	param.type = type;

	// Mandatory
	param.mandatory = mandatory ? 1 : 0;

	// Default value
	if (param.type != PARAM_TYPE_FLAG && defaultValue == NULL && !mandatory)
	{
		cout << "Error: No default value of a non mandatory parameter" << endl;
		return 0;
	}

	if (param.type == PARAM_TYPE_FLAG)
	{
		param.value[0] = 0;
		param.mandatory = 0;
	}
	else
	{
		if (defaultValue != NULL)
		{
			switch (param.type)
			{
			case PARAM_TYPE_INT:
				memcpy(param.value, defaultValue, sizeof(int));
				break;
			case PARAM_TYPE_FLOAT:
				memcpy(param.value, defaultValue, sizeof(float));
				break;
			case PARAM_TYPE_STRING:
				if (strlen(defaultValue) > PARAM_VALUE_LENGHT - 1)
				{
					cout << "Error: String value too long" << endl;
					return 0;
				}
				strcpy(param.value, defaultValue);
				break;
			default:
				cout << "Warning: Unknown parameter type" << endl;
				break;
			}
		}
	}

	// Name
	if (name == NULL)
	{
		cout << "Error: No name" << endl;
		return 0;
	}

	if (strlen(name) > PARAM_NAME_LENGHT - 1)
	{
		cout << "Error: Name too long" << endl;
		return 0;
	}

	strcpy(param.name, name);

	// Option short
	if (optionShort == NULL)
		cout << "Warning: No short option" << endl;
	else if (strlen(optionShort) > PARAM_OPTION_SHORT_LENGHT - 1)
		cout << "Error: Option short too long" << endl;
	else
		strcpy(param.optionShort, optionShort);

	// Option long
	if (optionLong == NULL)
		cout << "Warning: No long option" << endl;
	else if (strlen(optionLong) > PARAM_OPTION_LONG_LENGHT - 1)
		cout << "Error: Option long too long" << endl;
	else
		strcpy(param.optionLong, optionLong);

	// Description
	if (description == NULL)
		cout << "Warning: No description" << endl;
	else if (strlen(description) > PARAM_DESCRIPTION_LENGHT - 1)
		cout << "Error: Option long too long" << endl;
	else
		strcpy(param.description, description);

	return this->addParam(param);
}

int ParamList::addParamFlag(
	const char *name,
	const char *optionShort,
	const char *optionLong,
	const char *description)
{
	return this->addParam(
		PARAM_TYPE_FLAG,
		NULL,
		name,
		optionShort,
		optionLong,
		description,
		0);
}

int ParamList::addParamInt(
	const char *name,
	const char *optionShort,
	const char *optionLong,
	const char *description,
	int defaultValue,
	int mandatory)
{
	return this->addParam(
		PARAM_TYPE_INT,
		(char*)&defaultValue,
		name,
		optionShort,
		optionLong,
		description,
		mandatory);
}

int ParamList::addParamFloat(
	const char *name,
	const char *optionShort,
	const char *optionLong,
	const char *description,
	float defaultValue,
	int mandatory)
{
	return this->addParam(
		PARAM_TYPE_FLOAT,
		(char*)&defaultValue,
		name,
		optionShort,
		optionLong,
		description,
		mandatory);
}

int ParamList::addParamString(
	const char *name,
	const char *optionShort,
	const char *optionLong,
	const char *description,
	const char *defaultValue,
	int mandatory)
{
	return this->addParam(
		PARAM_TYPE_STRING,
		defaultValue,
		name,
		optionShort,
		optionLong,
		description,
		mandatory);
}

int ParamList::printUsage()
{
#ifdef linux
	cout << "       List of arguments:" << endl;

	for (int i = 0; i < this->nParams; i++)
	{
		cout << endl;
		cout << "       \033[1m" << this->params[i].optionShort << "\033[0m, \033[1m" << this->params[i].optionLong << "\033[0m" << endl;
		cout << "              " << this->params[i].description << endl;
	}
#else
	cout << "       List of arguments:" << endl;

	for (int i = 0; i < this->nParams; i++)
	{
		cout << endl;
		cout << "       " << this->params[i].optionShort << ", " << this->params[i].optionLong << endl;
		cout << "              " << this->params[i].description << endl;
	}
#endif

	return 1;
}

int ParamList::fillValues(int argc, char *argv[])
{
	int counter = 1;
	int paramId;

	while (counter < argc)
	{
		paramId = -1;

		for (int i = 0; i < this->nParams; i++)
		{
			if (strcmp(argv[counter], this->params[i].optionShort) == 0 ||
				strcmp(argv[counter], this->params[i].optionLong) == 0)
			{
				paramId = i;
				break;
			}
		}

		if (paramId == -1)
		{
			cout << "Warning: Param don't found" << endl;
			counter++;
			continue;
		}

		if (params[paramId].type == PARAM_TYPE_FLAG)
		{
			params[paramId].value[0] = 1;
			counter++;
		}
		else
		{
			if (counter >= argc - 1)
			{
				cout << "Error: The parameter [" << params[paramId].name << "] needs some value" << endl;
				return 0;
			}

			int valueInt;
			float valueFloat;

			switch (params[paramId].type)
			{
			case PARAM_TYPE_INT:
				valueInt = atoi(argv[counter + 1]);
				memcpy(params[paramId].value, &valueInt, sizeof(int));
				break;
			case PARAM_TYPE_FLOAT:
				valueFloat = (float)atof(argv[counter + 1]);
				memcpy(params[paramId].value, &valueFloat, sizeof(float));
				break;
			case PARAM_TYPE_STRING:
				if (strlen(argv[counter + 1]) >= PARAM_VALUE_LENGHT)
				{
					cout << "Error: Parameter value too long" << endl;
					return 0;
				}
				strcpy(params[paramId].value, argv[counter + 1]);
				break;
			default:
				cout << "Error: Wrong parameter type" << endl;
				return 0;
				break;
			}

			counter += 2;

			if (this->params[paramId].mandatory)
				this->params[paramId].mandatory = 2;
		}
	}

	return 1;
}

int ParamList::checkMandatoryParams()
{
	for (int i = 0; i < this->nParams; i++)
	{
		if (this->params[i].mandatory == 1)
		{
			cout << "Missing mandatory param [" << this->params[i].name << "]" << endl;
			return 0;
		}
	}

	return 1;
}

int ParamList::getValue(const char *name, char *value)
{
	for (int i = 0; i < this->nParams; i++)
	{
		if (strcmp(name, this->params[i].name) == 0)
		{
			switch (this->params[i].type)
			{
			case PARAM_TYPE_INT:
				memcpy(value, this->params[i].value, sizeof(int));
				break;
			case PARAM_TYPE_FLOAT:
				memcpy(value, this->params[i].value, sizeof(float));
				break;
			case PARAM_TYPE_STRING:
				strcpy(value, this->params[i].value);
				break;
			case PARAM_TYPE_FLAG:
				value[0] = this->params[i].value[0];
				break;
			default:
				cout << "Wrong type" << endl;
				return 0;
				break;
			}

			return 1;
		}
	}

	return 0;
}
