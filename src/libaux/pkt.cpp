/*
 * pkt.cpp
 *
 *  Created on: 2 Aug 2010
 *      Author: showroom
 */


/*
 * INCLUDES
 */

#include <iostream>
#include "pkt.h"


/*
 * NAMESPACES
 */

using namespace std;


/*
 * CLASS: pktifstream
 */

void pktifstream::open(const char *filename, ios_base::openmode mode)
{
		/* Variables */
	int posSeparator = -1;
	char *archivename;

		/* Check file mode */

	// Look if there is a double / -> '//'
	for (unsigned int i = 0; i < strlen(filename) - 1; i++)
	{
		if (filename[i] == '/' && filename[i + 1] == '/')
		{
			posSeparator = i;
			break;
		}
	}

		/* Open the file in the good mode */
	if (posSeparator == -1)
	{
		this->openFile(filename, mode);
	}
	else
	{
		archivename = new char[posSeparator + 1];
		memcpy(archivename, filename, posSeparator);
		archivename[posSeparator] = '\0';
		this->openArchive(archivename, filename + posSeparator + 2, mode);
		delete [] archivename;
	}
}

streampos pktifstream::tellg()
{
	if (this->filemode == PKT_FILEMODE_NORMAL)
		return ifstream::tellg();

	return ifstream::tellg() + offsetBegin;
}

istream &pktifstream::seekg(streampos pos)
{
	if (this->filemode == PKT_FILEMODE_NORMAL)
		return ifstream::seekg(pos);

	ifstream::seekg(pos + offsetBegin);
	return *this;
}

istream &pktifstream::seekg(streamoff off, ios_base::seekdir dir)
{
	if (this->filemode == PKT_FILEMODE_NORMAL)
		return ifstream::seekg(off, dir);

	ifstream::seekg(off + offsetBegin, dir);
	return *this;
}

istream &pktifstream::read(char* s, streamsize n)
{
	if (this->filemode == PKT_FILEMODE_NORMAL)
		return ifstream::read(s, n);

	streampos pos = ifstream::tellg();
	streamsize sizeMax = this->offsetEnd - pos;

	if (n >= sizeMax)
	{
		ifstream::read(s, sizeMax);
		this->setstate(ios::eofbit);
		return *this;
	}

	return ifstream::read(s, n);
}

int pktifstream::peek()
{
	if (this->filemode == PKT_FILEMODE_NORMAL)
		return ifstream::peek();

	streampos pos = ifstream::tellg();
	streamsize sizeMax = this->offsetEnd - pos;

	if (sizeMax == 1)
	{
		int c = ifstream::peek();
		this->setstate(ios::eofbit);
		return c;
	}

	return ifstream::peek();
}

istream &pktifstream::ignore(streamsize n, int delim)
{
	if (this->filemode == PKT_FILEMODE_NORMAL)
		return ifstream::ignore(n, delim);

	streampos pos = ifstream::tellg();
	streamsize sizeMax = this->offsetEnd - pos;

	if (n >= sizeMax)
	{
		ifstream::ignore(sizeMax, delim);

		if (ifstream::gcount() == sizeMax)
			this->setstate(ios::eofbit);

		return *this;
	}

	return ifstream::ignore(n, delim);
}

istream &pktifstream::getline(char *s, streamsize n)
{
	if (this->filemode == PKT_FILEMODE_NORMAL)
		return ifstream::getline(s, n);

	streampos pos = ifstream::tellg();
	streamsize sizeMax = this->offsetEnd - pos;

	if (n >= sizeMax)
	{
		ifstream::getline(s, sizeMax);

		if (ifstream::gcount() == sizeMax)
			this->setstate(ios::eofbit);

		return *this;
	}

	return ifstream::getline(s, n);
}

istream &pktifstream::getline(char *s, streamsize n, char delim)
{
	if (this->filemode == PKT_FILEMODE_NORMAL)
		return ifstream::getline(s, n, delim);

	streampos pos = ifstream::tellg();
	streamsize sizeMax = this->offsetEnd - pos;

	if (n >= sizeMax)
	{
		ifstream::getline(s, sizeMax, delim);

		if (ifstream::gcount() == sizeMax)
			this->setstate(ios::eofbit);

		return *this;
	}

	return ifstream::getline(s, n, delim);
}

int pktifstream::get()
{
	if (this->filemode == PKT_FILEMODE_NORMAL)
		return ifstream::get();

	streampos pos = ifstream::tellg();
	streamsize sizeMax = this->offsetEnd - pos;

	if (sizeMax == 1)
	{
		int c = ifstream::get();
		this->setstate(ios::eofbit);
		return c;
	}

	return ifstream::get();
}

istream &pktifstream::get(char &c)
{
	if (this->filemode == PKT_FILEMODE_NORMAL)
		return ifstream::get(c);

	streampos pos = ifstream::tellg();
	streamsize sizeMax = this->offsetEnd - pos;

	if (sizeMax == 1)
	{
		ifstream::get(c);
		this->setstate(ios::eofbit);
		return *this;
	}

	return ifstream::get(c);
}

istream &pktifstream::get(char *s, streamsize n)
{
	if (this->filemode == PKT_FILEMODE_NORMAL)
		return ifstream::get(s, n);

	streampos pos = ifstream::tellg();
	streamsize sizeMax = this->offsetEnd - pos;

	if (n >= sizeMax)
	{
		ifstream::get(s, sizeMax);

		if (ifstream::gcount() == sizeMax)
			this->setstate(ios::eofbit);

		return *this;
	}

	return ifstream::get(s, n);
}

istream &pktifstream::get(char *s, streamsize n, char delim)
{
	if (this->filemode == PKT_FILEMODE_NORMAL)
		return ifstream::get(s, n, delim);

	streampos pos = ifstream::tellg();
	streamsize sizeMax = this->offsetEnd - pos;

	if (n >= sizeMax)
	{
		ifstream::get(s, sizeMax, delim);

		if (ifstream::gcount() == sizeMax)
			this->setstate(ios::eofbit);

		return *this;
	}

	return ifstream::get(s, n, delim);
}

istream &pktifstream::get(streambuf &sb)
{
	if (this->filemode == PKT_FILEMODE_NORMAL)
		return ifstream::get(sb);

	streampos pos = ifstream::tellg();
	streamsize sizeMax = this->offsetEnd - pos;

	ifstream::get(sb);

	if (ifstream::gcount() >= sizeMax)
		this->setstate(ios::eofbit);

	return *this;
}

istream &pktifstream::get(streambuf &sb, char delim)
{
	if (this->filemode == PKT_FILEMODE_NORMAL)
		return ifstream::get(sb, delim);

	streampos pos = ifstream::tellg();
	streamsize sizeMax = this->offsetEnd - pos;

	ifstream::get(sb, delim);

	if (ifstream::gcount() >= sizeMax)
		this->setstate(ios::eofbit);

	return *this;
}

void pktifstream::openFile(const char *filename, ios_base::openmode mode)
{
	this->filemode = PKT_FILEMODE_NORMAL;
	this->offsetBegin = 0;
	ifstream::open(filename, mode);
}

void pktifstream::openArchive(const char *archivename, const char *filename, ios_base::openmode mode)
{
	this->filemode = PKT_FILEMODE_TAR;
	this->offsetBegin = 0;
	ifstream::open(archivename, mode);

	if (!(*this))
		return;

	// Look for the file in the tar file
	pktheader header;
	int fileFound = 0;
	unsigned long fileSize;

	while (!fileFound)
	{
		if (!(*this) || ifstream::eof())
			break;

		ifstream::read((char*)&header, 512);

		// Check if it's full of zeroes
		if (this->testZeroes((char*)&header, 512))
		{
			//cout << "Error: File not found" << endl;
			this->close();
			this->setstate(ios::failbit);
			return;
		}

		if (!this->testChecksum((char*)&header))
		{
			//cout << "Error: Wrong checksum" << endl;
			this->close();
			this->setstate(ios::failbit);
			return;
		}

		//cout << "Filename: " << header.filename << endl;

		// Check filename
		if (strcmp(header.filename, filename) == 0)
		{
			offsetBegin = ifstream::tellg();
			offsetEnd = offsetBegin + (streampos)this->oct2ulong((char*)&header.filesize, 12);

			fileFound = 1;
			break;
		}

		fileSize = this->oct2ulong((char*)&header.filesize, 12);

		if ((fileSize % 512) != 0)
			fileSize = (fileSize / 512 + 1) * 512;

		ifstream::seekg(fileSize, ios::cur);
	}

	if (!fileFound)
	{
		//cout << "Error: File not found" << endl;
		this->close();
		this->setstate(ios::failbit);
		return;
	}

	// Check if it is a normal file
	if (header.typeFlag != PKT_TYPEFLAG_NORMAL)
	{
		//cout << "The archived file is not a normal file" << endl;
		this->close();
		this->setstate(ios::failbit);
		return;
	}
}

unsigned long pktifstream::oct2ulong(char *oct, int lOct)
{
	// Variables
	unsigned long value = 0;

	// Check the last character
	if (oct[lOct - 1] != '\0' && oct[lOct - 1] != ' ')
	{
		cout << "Error: Wrong octal end format" << endl;
		return 0;
	}

	for (int i = 0; i < lOct - 1; i++)
	{
		if (oct[i] < '0' || oct[i] > '7')
		{
			cout << "Error: Wrong octal numeric format [" << i << "]" << endl;
			return 0;
		}

		value += (oct[i] - '0') << ((lOct - i - 2) * 3);
	}

	return value;
}

int pktifstream::testChecksum(char *header)
{
	unsigned char *headerUnsigned = (unsigned char*)header;
	unsigned long checksumUnsigned = 32 * 8;

	signed char *headerSigned = (signed char*)header;
	unsigned long checksumSigned = 32 * 8;

	unsigned long checksumHeader = this->oct2ulong(&header[148], 7);

	// Check the last two characters
	if (header[154] != '\0' || header[155] != ' ')
	{
		cout << "Error: Wrong header end format" << endl;
		return 0;
	}

	// Calculate checksum
	for (int i = 0; i < 148; i++)
	{
		checksumUnsigned += headerUnsigned[i];
		checksumSigned += headerSigned[i];
	}

	for (int i = 156; i < 512; i++)
	{
		checksumUnsigned += headerUnsigned[i];
		checksumSigned += headerSigned[i];
	}

	if (checksumUnsigned == checksumHeader)
		return 1;

	if (checksumSigned == checksumHeader)
		return 1;

	return 0;
}

int pktifstream::testZeroes(char *buffer, int len)
{
	for (int i = 0; i < len; i++)
	{
		if (buffer[i] != '\0')
			return 0;
	}

	return 1;
}
