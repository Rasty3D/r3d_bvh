/*
 * nmlparser.h
 *
 *   Created on: 03 May 2011
 *       Author: Jesus Ortiz
 *
 *  Description: TODO
 *
 */

#ifndef NMLPARSER_H_
#define NMLPARSER_H_


/*
 * INCLUDES
 */

	/* General */
#include <stdlib.h>
#include <stdarg.h>

	/* PKT */
#include "pkt.h"


/*
 * DEFINES
 */

	/* Token types */
#define NMLPARSER_ERROR				-1
#define NMLPARSER_EOF				0
#define NMLPARSER_WORD				1
#define NMLPARSER_SEPARATOR			2

	/* Special tokens */
#define NMLPARSER_OPEN_BRACKET		'{'
#define NMLPARSER_CLOSE_BRACKET		'}'
#define NMLPARSER_OPEN_PARANTHESES	'('
#define NMLPARSER_CLOSE_PARANTHESES	')'
#define NMLPARSER_COLON				':'
#define NMLPARSER_SEMICOLON			';'
#define NMLPARSER_COMMA				','
#define NMLPARSER_QUOTATION_SIMPLE	'\''
#define NMLPARSER_QUOTATION_DOUBLE	'\"'

	/* Max token lenght */
#define NMLPARSER_TOKEN_LEN			256

	/* Error levels */
#define NMLPARSER_LEVEL_NOTHING		0
#define NMLPARSER_LEVEL_INFO		1
#define NMLPARSER_LEVEL_WARNING		2
#define NMLPARSER_LEVEL_ERROR		3
#define NMLPARSER_LEVEL_CRITICAL	4

	/* Error codes */
#define NMLPARSER_ERROR_BADFILE		101

	/* Error message */
#define NMLPARSER_ERROR_LENGTH		256


/*
 * TYPES
 */

typedef struct
{
	int type;
	char value[NMLPARSER_TOKEN_LEN];
}NMLParser_token;

typedef struct
{
	int code;
	int level;
	char message[NMLPARSER_ERROR_LENGTH];
}NMLParser_error;


/*
 * CLASS: NMLParser
 */

class NMLParser
{
private:
		/* File handler */
	pktifstream file;

		/* File name */
	// Store the file name for error display
	char filename[NMLPARSER_TOKEN_LEN];

		/* Line counter */
	int currentLine;

		/* Last error */
	NMLParser_error lastError;

public:
		/* Constructor and desstructor */
	NMLParser();
	~NMLParser();

		/* File functions */
	int open(const char *filename);
	NMLParser_token getNextToken();
	void close();

		/* Other functions */
	// Get the current line
	int getCurrentLine();

	// Get the filename
	char *getFilename();

	// Get last error
	NMLParser_error *getLastError();

private:
	// Set error
	void setError(int code, int level, const char *message, ...);

public:
		/* Read specific token */
	// Get the next token. If it is not the separator we expect
	// shows error and return 0, otherwise return 1
	int getSeparator(char separator, NMLParser_token &token);

	// Get the next token. If it is not a separator returns 0
	// 1 otherwise
	int getSeparator(NMLParser_token &token);

	// Get the next token. If it is not the word we expect
	// shows error and return 0, otherwise return 1
	int getWord(const char *word, NMLParser_token &token);

	// Get the next token. If it is not a word returns 0
	// 1 otherwise
	int getWord(NMLParser_token &token);

	// Get a file
	int getFile(char *filename);

	// Get a color
	int getColor(float *color);

private:
		/* Private aux functions */
	// Converts a byte in hexadecimal to float (0.0-1.0), for color conversions
	float hex2float(char *hex);

public:
		/* Get a list of tokens between separators (For exmple '.') */
	static int getTokens(const char *string, char *buffer, char **tokens, char separator);
};

#endif /* NMLPARSER_H_ */
