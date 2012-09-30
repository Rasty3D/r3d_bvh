/*
 * indexer.cpp
 *
 *  Created on: 5 May 2011
 *      Author: Jesus Ortiz
 */

/*
 * INCLUDES
 */

#include "indexer.h"


/*
 * NAMESPACES
 */

using namespace std;


/*
 * CLASS: Indexer
 */


	/* Constructor and destructor */

Indexer::Indexer()
{
	this->child = NULL;
	this->nextBrother = NULL;
	this->element = NULL;
	this->character = '\0';
}

Indexer::~Indexer()
{
	this->clear();
}


	/* Manage elements */

// Add element to the current position
// Returns 1 on success
// Returns 0 if the name already exists
int Indexer::add(const char *name, void *element, int overwrite)
{
	// Check if it's a valid element
	if (element == NULL)
		return 0;

	// Last character
	if (name[1] == '\0')
	{
		// Check character
		if (this->character == '\0')
		{
			this->character = name[0];
			this->element = element;
			return 1;
		}
		else if (this->character == name[0])
		{
			// The name already exists
			if (overwrite || this->element == NULL)
			{
				this->element = element;
				return 1;
			}
			else
			{
				return 0;
			}
		}
		else if (this->nextBrother != NULL)	// Check next brother
		{
			return this->nextBrother->add(name, element, overwrite);
		}
		else
		{
			// There is no brother with the name and it's the last character
			// -> Add new element
			this->nextBrother = new Indexer;
			return this->nextBrother->add(name, element, overwrite);
		}
	}

	// Check character
	if (this->character == '\0')
	{
		// Save character
		this->character = name[0];

		if (this->child == NULL)
		{
			// Add the rest of the characters
			this->child = new Indexer;
			return this->child->add(name + 1, element, overwrite);
		}
		else
		{
			// The child should be NULL
			return 0;
		}
	}
	else if (this->character == name[0])
	{
		if (this->child == NULL)
		{
			// Add the rest of the characters
			this->child = new Indexer;
		}

		return this->child->add(name + 1, element, overwrite);
	}
	else if (this->nextBrother != NULL)	// Check next brother
	{
		return this->nextBrother->add(name, element, overwrite);
	}
	else
	{
		// There is no brother with the name
		// -> Add the rest of the characters
		this->nextBrother = new Indexer;
		return this->nextBrother->add(name, element, overwrite);
	}
}

// Get element from the current position
void *Indexer::get(const char *name)
{
	// Last character
	if (name[1] == '\0')
	{
		// Check character
		if (this->character == name[0])
		{
			return this->element;
		}
		else if (this->nextBrother != NULL)	// Check next brother
		{
			return this->nextBrother->get(name);
		}
		else
		{
			return NULL;
		}
	}

	// Check character
	if (this->character == name[0])
	{
		if (this->child == NULL)
			return NULL;
		else
			return this->child->get(name + 1);
	}
	else if (this->nextBrother != NULL)	// Check next brother
	{
		return this->nextBrother->get(name);
	}
	else
	{
		return NULL;
	}
}

// Removes the refernece to an element
int Indexer::remove(const char *name)
{
	// Last character
	if (name[1] == '\0')
	{
		// Check character
		if (this->character == '\0')
		{
			// Name not found
			return 0;
		}
		else if (this->character == name[0])
		{
			// Name found -> Delete reference
			this->element = NULL;
			return 1;
		}
		else if (this->nextBrother != NULL)	// Check next brother
		{
			return this->nextBrother->remove(name);
		}
		else
		{
			// There is no brother with the name and it's the last character
			// Name not found
			return 0;
		}
	}

	// Check character
	if (this->character == '\0')
	{
		// Name not found
		return 0;
	}
	else if (this->character == name[0])
	{
		if (this->child == NULL)
		{
			// Name not found
			return 0;
		}
		else
		{
			return this->child->remove(name + 1);
		}
	}
	else if (this->nextBrother != NULL)	// Check next brother
	{
		return this->nextBrother->remove(name);
	}
	else
	{
		// There is no brother with the name
		// Name not found
		return 0;
	}
}

// Clean tree
void Indexer::clear()
{
	// Delete all children
	if (this->child != NULL)
	{
		Indexer *aux = this->child;
		Indexer *prevBrother;

		while (aux != NULL)
		{
			prevBrother = aux;
			aux = aux->nextBrother;
			delete prevBrother;
		}

		this->child = NULL;
	}
}


	/* Print tree */

void Indexer::print()
{
	cout << this->character;

	if (this->child != NULL)
		this->child->print();

	if (this->nextBrother != NULL)
		this->nextBrother->print();
}
