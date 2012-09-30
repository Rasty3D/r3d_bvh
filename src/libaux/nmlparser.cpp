/*
 * nmlparser.cpp
 *
 *   Created on: 03 May 2011
 *       Author: Jesus Ortiz
 *
 *  Description: TODO
 *
 */

/*
 * INCLUDES
 */

#include "nmlparser.h"


/*
 * CLASS: NMLParser
 */


	/* Constructor and destructor */

NMLParser::NMLParser()
{
	this->lastError.level = NMLPARSER_LEVEL_NOTHING;
	return;
}

NMLParser::~NMLParser()
{
	this->file.close();
}


	/* File functions */

int NMLParser::open(const char *filename)
{
	this->lastError.level = NMLPARSER_LEVEL_NOTHING;
	this->file.open(filename);
	this->currentLine = 1;
	strcpy(this->filename, filename);

	return (!file) ? 0 : 1;
}

NMLParser_token NMLParser::getNextToken()
{
	// Variables
	char c;
	int pos = 0;
	NMLParser_token token;
	char waitingQuotation = 0;
	int waitingComment = 0;

	// Init token type
	token.type = NMLPARSER_LEVEL_ERROR;

	// Check EOF
	if (this->file.eof())
	{
		token.type = NMLPARSER_EOF;
		return token;
	}

	// Jump spaces
	do
	{
		c = this->file.get();

		if (file.eof())
		{
			token.type = NMLPARSER_EOF;
			return token;
		}

		if (c == '\n')
			this->currentLine++;

	}while (c == ' ' || c == '\n' || c == '\r' || c == '\t');

	// Read token
	while (!this->file.eof())
	{
			/* Manage comments */

		if (waitingComment == 0 && c == '/')
		{
			waitingComment = 1;
		}
		else if (waitingComment == 1 && c == '*')
		{
			waitingComment = 2;

			if (pos > 1)
			{
				this->file.unget();
				this->file.unget();
				token.type = NMLPARSER_WORD;
				pos -= 2;
				break;
			}
		}
		else if (waitingComment == 1 && c != '*')
		{
			waitingComment = 0;
		}
		else if (waitingComment == 2 && c == '*')
		{
			waitingComment = 3;
		}
		else if (waitingComment == 3 && c == '/')
		{
			waitingComment = 4;
		}
		else if (waitingComment == 3 && c != '/')
		{
			waitingComment = 2;
		}

		if (waitingComment == 2 || waitingComment == 3)
		{
			c = this->file.get();

			if (c == '\n')
				this->currentLine++;

			continue;
		}
		else if (waitingComment == 4)
		{
			pos = 0;
			waitingComment = 0;

			// Jump spaces
			do
			{
				c = this->file.get();

				if (file.eof())
				{
					token.type = NMLPARSER_EOF;
					return token;
				}

				if (c == '\n')
					this->currentLine++;

			}while (c == ' ' || c == '\n' || c == '\r' || c == '\t');

			continue;
		}

			/* Manage quotations */

		if (pos == 0 && c == NMLPARSER_QUOTATION_SIMPLE)
		{
			waitingQuotation = NMLPARSER_QUOTATION_SIMPLE;
			c = this->file.get();
		}
		else if (pos == 0 && c == NMLPARSER_QUOTATION_DOUBLE)
		{
			waitingQuotation = NMLPARSER_QUOTATION_DOUBLE;
			c = this->file.get();
		}

			/* Processing depending on the quotations*/

		// Outside quotations
		if (waitingQuotation == 0)
		{
			if (c == ' ' || c == '\n' || c == '\r' || c == '\t')
			{
				token.type = NMLPARSER_WORD;
				break;
			}

			if (c == NMLPARSER_OPEN_BRACKET ||
				c == NMLPARSER_CLOSE_BRACKET ||
				c == NMLPARSER_OPEN_PARANTHESES ||
				c == NMLPARSER_CLOSE_PARANTHESES ||
				c == NMLPARSER_COLON ||
				c == NMLPARSER_SEMICOLON ||
				c == NMLPARSER_COMMA)
			{
				if (pos == 0)
				{
					token.value[pos] = c;
					pos++;
					token.type = NMLPARSER_SEPARATOR;
					break;
				}
				else
				{
					this->file.unget();
					token.type = NMLPARSER_WORD;
					break;
				}
			}
		}
		else if (waitingQuotation == NMLPARSER_QUOTATION_SIMPLE)	// Inside single quotations
		{
			if (c == '\n' || c == '\r')
				this->setError(NMLPARSER_LEVEL_WARNING, NMLPARSER_ERROR_BADFILE, "New line in constant");

			if (pos > 0 && c == NMLPARSER_QUOTATION_SIMPLE)
			{
				token.type = NMLPARSER_WORD;
				break;
			}
		}
		else if (waitingQuotation == NMLPARSER_QUOTATION_DOUBLE)	// Inside double quotations
		{
			if (c == '\n' || c == '\r')
				this->setError(NMLPARSER_LEVEL_WARNING, NMLPARSER_ERROR_BADFILE, "New line in constant");

			if (pos > 0 && c == NMLPARSER_QUOTATION_DOUBLE)
			{
				token.type = NMLPARSER_WORD;
				break;
			}
		}

		// Add new character
		token.value[pos] = c;
		pos++;

		// Read new character
		c = this->file.get();

		// Increment line counter
		if (c == '\n')
			this->currentLine++;

		// Check overflow
		if (pos >= NMLPARSER_TOKEN_LEN - 1)
		{
			token.type = NMLPARSER_LEVEL_ERROR;
			break;
		}
	}

	// Close the string
	token.value[pos] = '\0';

	// Check errors
	if (token.type == NMLPARSER_LEVEL_ERROR)
		this->setError(NMLPARSER_LEVEL_ERROR, NMLPARSER_ERROR_BADFILE, "Generic error");

	// Return token
	return token;
}

void NMLParser::close()
{
	this->lastError.level = NMLPARSER_LEVEL_NOTHING;
	this->file.close();
}


	/* Other functions */

// Get the current line
int NMLParser::getCurrentLine()
{
	return this->currentLine;
}

// Get the filename
char *NMLParser::getFilename()
{
	return this->filename;
}

// Get last error
NMLParser_error *NMLParser::getLastError()
{
	if (this->lastError.level == NMLPARSER_LEVEL_NOTHING)
		return NULL;
	else
		return &this->lastError;
}

// Set error
void NMLParser::setError(int code, int level, const char *message, ...)
{
	va_list ap;

	va_start(ap, message);
		vsprintf(this->lastError.message, message, ap);
	va_end(ap);

	this->lastError.code = code;
	this->lastError.level = level;
}


	/* Read specific token */

// Get the next token. If it is not the separator we expect
// shows error and return 0, otherwise return 1
int NMLParser::getSeparator(char separator, NMLParser_token &token)
{
	token = this->getNextToken();

	if (token.type != NMLPARSER_SEPARATOR ||
		token.value[0] != separator)
	{
		this->setError(
			NMLPARSER_LEVEL_ERROR, NMLPARSER_ERROR_BADFILE,
			"Expecting \"%c\" separator. Found \"%s\" instead",
			separator, token.value);
		return 0;
	}

	return 1;
}

// Get the next token. If it is not a separator returns 0
// 1 otherwise
int NMLParser::getSeparator(NMLParser_token &token)
{
	token = this->getNextToken();

	if (token.type != NMLPARSER_SEPARATOR)
	{
		this->setError(
			NMLPARSER_LEVEL_ERROR, NMLPARSER_ERROR_BADFILE,
			"Expecting separator");
		return 0;
	}

	return 1;
}

// Get the next token. If it is not the word we expect
// shows error and return 0, otherwise return 1
int NMLParser::getWord(const char *word, NMLParser_token &token)
{
	token = this->getNextToken();

	if (token.type != NMLPARSER_WORD ||
		strcmp(token.value, word) != 0)
	{
		this->setError(
			NMLPARSER_LEVEL_ERROR, NMLPARSER_ERROR_BADFILE,
			"Expecting word \"%s\". Found \"%s\" instead",
			word, token.value);
		return 0;
	}

	return 1;
}

// Get the next token. If it is not a word returns 0
// 1 otherwise
int NMLParser::getWord(NMLParser_token &token)
{
	token = this->getNextToken();

	if (token.type != NMLPARSER_WORD)
	{
		this->setError(
			NMLPARSER_LEVEL_ERROR, NMLPARSER_ERROR_BADFILE,
			"Expecting word");
		return 0;
	}

	return 1;
}

// Get a file
int NMLParser::getFile(char *filename)
{
	NMLParser_token token;

	if (!this->getWord("file", token))
		return 0;

	if (!this->getSeparator(NMLPARSER_OPEN_PARANTHESES, token))
		return 0;

	if (!this->getWord(token))
		return 0;

	strcpy(filename, token.value);

	return this->getSeparator(NMLPARSER_CLOSE_PARANTHESES, token);
}

// Get a color
int NMLParser::getColor(float *color)
{
	NMLParser_token tokenKeyword;
	NMLParser_token token;

	if (!this->getWord(tokenKeyword))
		return 0;

	// Read separator '('
	if (!this->getSeparator(NMLPARSER_OPEN_PARANTHESES, token))
		return 0;

	if (strcmp(tokenKeyword.value, "rgb-ascii") == 0)
	{
		// Read value
		if (!this->getWord(token))
			return 0;

		if (strlen(token.value) != 6)
		{
			this->setError(
				NMLPARSER_LEVEL_ERROR, NMLPARSER_ERROR_BADFILE,
				"Wrong color format");
			return 0;
		}

		color[0] = this->hex2float(&token.value[0]);
		color[1] = this->hex2float(&token.value[2]);
		color[2] = this->hex2float(&token.value[4]);
		color[3] = 1.0f;
	}
	else if (strcmp(tokenKeyword.value, "rgba-ascii") == 0)
	{
		// Read value
		if (!this->getWord(token))
			return 0;

		if (strlen(token.value) != 8)
		{
			this->setError(
				NMLPARSER_LEVEL_ERROR, NMLPARSER_ERROR_BADFILE,
				"Wrong color format");
			return 0;
		}

		color[0] = this->hex2float(&token.value[0]);
		color[1] = this->hex2float(&token.value[2]);
		color[2] = this->hex2float(&token.value[4]);
		color[3] = this->hex2float(&token.value[6]);
	}
	else if (strcmp(tokenKeyword.value, "rgb-float") == 0)
	{
		// Read value
		if (!this->getWord(token))
			return 0;

		color[0] = (float)atof(token.value);

		// Read separator ','
		if (!this->getSeparator(NMLPARSER_COMMA, token))
			return 0;

		// Read value
		if (!this->getWord(token))
			return 0;

		color[1] = (float)atof(token.value);

		// Read separator ','
		if (!this->getSeparator(NMLPARSER_COMMA, token))
			return 0;

		// Read value
		if (!this->getWord(token))
			return 0;

		color[2] = (float)atof(token.value);

		color[3] = 1.0f;
	}
	else if (strcmp(tokenKeyword.value, "rgba-float") == 0)
	{
		// Read value
		if (!this->getWord(token))
			return 0;

		color[0] = (float)atof(token.value);

		// Read separator ','
		if (!this->getSeparator(NMLPARSER_COMMA, token))
			return 0;

		// Read value
		if (!this->getWord(token))
			return 0;

		color[1] = (float)atof(token.value);

		// Read separator ','
		if (!this->getSeparator(NMLPARSER_COMMA, token))
			return 0;

		// Read value
		if (!this->getWord(token))
			return 0;

		color[2] = (float)atof(token.value);

		// Read separator ','
		if (!this->getSeparator(NMLPARSER_COMMA, token))
			return 0;

		// Read value
		if (!this->getWord(token))
			return 0;

		color[3] = (float)atof(token.value);
	}
	else if (strcmp(tokenKeyword.value, "rgb-integer") == 0)
	{
		// Read value
		if (!this->getWord(token))
			return 0;

		color[0] = (float)atoi(token.value) / 255.0f;

		// Read separator ','
		if (!this->getSeparator(NMLPARSER_COMMA, token))
			return 0;

		// Read value
		if (!this->getWord(token))
			return 0;

		color[1] = (float)atoi(token.value) / 255.0f;

		// Read separator ','
		if (!this->getSeparator(NMLPARSER_COMMA, token))
			return 0;

		// Read value
		if (!this->getWord(token))
			return 0;

		color[2] = (float)atoi(token.value) / 255.0f;

		color[3] = 1.0f;
	}
	else if (strcmp(tokenKeyword.value, "rgba-integer") == 0)
	{
		// Read value
		if (!this->getWord(token))
			return 0;

		color[0] = (float)atoi(token.value) / 255.0f;

		// Read separator ','
		if (!this->getSeparator(NMLPARSER_COMMA, token))
			return 0;

		// Read value
		if (!this->getWord(token))
			return 0;

		color[1] = (float)atoi(token.value) / 255.0f;

		// Read separator ','
		if (!this->getSeparator(NMLPARSER_COMMA, token))
			return 0;

		// Read value
		if (!this->getWord(token))
			return 0;

		color[2] = (float)atoi(token.value) / 255.0f;

		// Read separator ','
		if (!this->getSeparator(NMLPARSER_COMMA, token))
			return 0;

		// Read value
		if (!this->getWord(token))
			return 0;

		color[3] = (float)atoi(token.value) / 255.0f;
	}
	else
	{
		this->setError(
			NMLPARSER_LEVEL_ERROR, NMLPARSER_ERROR_BADFILE,
			"Wrong color type. Found \"%s\"", token.value);
		return 0;
	}

	// Read separator ')'
	return this->getSeparator(NMLPARSER_CLOSE_PARANTHESES, token);
}


	/* Private aux functions */

// Convertes a byte in hexadecimal to float (0.0-1.0), for color conversions
float NMLParser::hex2float(char *hex)
{
	unsigned char value = 0;

	if (hex[0] >= '0' && hex[0] >= '9')
		value += hex[0] - '0';
	else if (hex[0] >= 'a' && hex[0] >= 'f')
		value += hex[0] - 'a' + 10;
	else if (hex[0] >= 'A' && hex[0] >= 'F')
		value += hex[0] - 'A' + 10;

	value = value << 4;

	if (hex[1] >= '0' && hex[1] >= '9')
		value += hex[1] - '0';
	else if (hex[1] >= 'a' && hex[1] >= 'f')
		value += hex[1] - 'a' + 10;
	else if (hex[1] >= 'A' && hex[1] >= 'F')
		value += hex[1] - 'A' + 10;

	return (float)value / 255.0f;
}


	/* Get a list of tokens between separators (For exmple '.') */

int NMLParser::getTokens(const char *string, char *buffer, char **tokens, char separator)
{
	int stringLen = strlen(string);
	int nTokens = 0;

	if (stringLen == 0)
		return nTokens;

	tokens[0] = &buffer[0];
	nTokens = 1;

	for (int i = 0; i < stringLen; i++)
	{
		if (string[i] == separator)
		{
			buffer[i] = '\0';

			if (i < stringLen - 1)
				tokens[nTokens] = &buffer[i + 1];

			nTokens++;
		}
		else
		{
			buffer[i] = string[i];
		}
	}

	buffer[stringLen] = '\0';
	return nTokens;
}
