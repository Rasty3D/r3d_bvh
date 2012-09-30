/*
 * indexer.h
 *
 *  Created on: 5 May 2011
 *      Author: Jesus Ortiz
 */

#ifndef INDEXER_H_
#define INDEXER_H_

/*
 * INCLUDES
 */

#include <iostream>
#include <stdlib.h>


/*
 * CLASS: Indexer
 */

class Indexer
{
private:
	Indexer *child;
	Indexer *nextBrother;

	void *element;
	char character;

public:
		/* Constructor and destructor */
	Indexer();
	~Indexer();

		/* Manage elements */
	// Add element to the current position
	// Returns 1 on success
	// Returns 0 if the name already exists
	int add(const char *name, void *element, int overwrite);

	// Get element from the current position
	void *get(const char *name);

	// Removes the refernece to an element
	int remove(const char *name);

	// Clean tree
	void clear();

		/* Print tree */
	void print();
};

#endif	/* INDEXER_H_ */
