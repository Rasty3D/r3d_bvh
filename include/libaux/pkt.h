/*
 * pkt.h
 *
 *  Created on: 2 Aug 2010
 *      Author: showroom
 */

#ifndef PKT_H_
#define PKT_H_


/*
 * INCLUDES
 */

#include <fstream>
#include <string.h>


/*
 * DEFINES
 */

	/* File modes */
#define PKT_TYPEFLAG_NORMAL		'0'	// Normalfile			V
#define PKT_TYPEFLAG_HARD		'1'	// Hard link			X
#define PKT_TYPEFLAG_SYM		'2'	// Symbolic link		X
#define PKT_TYPEFLAG_CHAR		'3'	// Character special	X
#define PKT_TYPEFLAG_BLOCK		'4'	// Block special		X
#define PKT_TYPEFLAG_DIR		'5'	// Directory			V
#define PKT_TYPEFLAG_FIFO		'6'	// FIFO					X

	/* File modes */
#define PKT_FILEMODE_NORMAL		0
#define PKT_FILEMODE_TAR		1


/*
 * TYPES
 */

#pragma pack(push, 1)
typedef struct
{								// Off - Size
	char filename[100];			// 0   - 100
	char filemode[8];			// 100 - 8		// In octal, finished with '\0' or ' ', padded with '0'
	char ownerId[8];			// 108 - 8		// In octal, finished with '\0' or ' ', padded with '0'
	char groupId[8];			// 116 - 8		// In octal, finished with '\0' or ' ', padded with '0'
	char filesize[12];			// 124 - 12		// In octal, finished with '\0' or ' ', padded with '0'
	char lastModification[12];	// 136 - 12		// In octal, finished with '\0' or ' ', padded with '0'
												// In Unix time (seconds from 00:00 of January 1st 1970)
	char checksum[8];			// 148 - 8		// In octal, finished with '\0', ' ', padded with '0'
												// Sum of the previous fields including eigth ' ' characters
	char typeFlag;				// 156 - 1
	char nameLikedFile[100];	// 157 - 100
	char ustarIndicator[8];		// 257 - 8		"ustar  \0"
	char ownerName[32];			// 265 - 32
	char groupName[32];			// 297 - 32
	char deviceMajorNumber[8];	// 329 - 8
	char deviceMinorNumber[8];	// 337 - 8
	char filenamePrefix[155];	// 345 - 155
	char padding[12];			// 500 - 12
}pktheader;
#pragma pack(pop)


/*
 * NAMESPACES
 */

using namespace std;


/*
 * CLASS: pktifstream
 */

class pktifstream: public ifstream
{
private:
		/* Private variables */
	int filemode;
	streampos offsetBegin;
	streampos offsetEnd;

public:
		/* Public functions */
	void open(const char *filename, ios_base::openmode mode = ios_base::in);

	streampos tellg();

	istream &seekg(streampos pos);
	istream &seekg(streamoff off, ios_base::seekdir dir);

	istream &read(char* s, streamsize n);

	int peek();

	istream &ignore(streamsize n = 1, int delim = EOF);

	istream &getline(char *s, streamsize n);
	istream &getline(char *s, streamsize n, char delim);

	int get();
	istream &get(char &c);
	istream &get(char *s, streamsize n);
	istream &get(char *s, streamsize n, char delim);
	istream &get(streambuf &sb);
	istream &get(streambuf &sb, char delim);

private:
		/* Open a normal file */
	void openFile(const char *filename, ios_base::openmode mode = ios_base::in);

		/* Open file from an archive file */
	void openArchive(const char *archivename, const char *filename, ios_base::openmode mode = ios_base::in);

		/* Converts an ASCII string with an octal number to an unsigned long integer */
	unsigned long oct2ulong(char *oct, int lOct);

		/* Check the checksum */
		// Returns 	0 if fails
		//			1 if pass
	int testChecksum(char *header);

		/* Check if the buffer is full of zeroes */
		// Return 1 if full of zeroes
		// Return 0 if there is some byte different to zero
	int testZeroes(char *buffer, int len);
};

#endif /* PKT_H_ */
