/*
 * indexedlist.h
 *
 *  Created on: 6 May 2011
 *      Author: Jesus Ortiz
 */

#ifndef INDEXEDLIST_H_
#define INDEXEDLIST_H_

/*
 * INCLUDES
 */

#include <list>
#include "indexer.h"


/*
 * CLASS: IndexedList
 */

template <class T>
class IndexedList
{
private:
	std::list<T> elements;
	Indexer indexer;

public:
		/* Manage elements */
	int add(const char *name, const T& value);
	int remove(const char *name);
	T *get(const char *name);
	void clear();
	class std::list<T>::iterator begin();
	class std::list<T>::iterator end();
};

template <class T>
int IndexedList<T>::add(const char *name, const T& value)
{
	this->elements.push_back(value);
	return this->indexer.add(name, &this->elements.back(), 0);
}

template <class T>
int IndexedList<T>::remove(const char *name)
{
	T *element = (T*)this->indexer.get(name);

	class std::list<T>::iterator it;

	for (it = this->elements.begin(); it != this->elements.end(); it++)
	{
		if (&(*it) == element)
		{
			this->elements.erase(it);
			return this->indexer.remove(name);
		}
	}

	return 0;
}

template <class T>
T *IndexedList<T>::get(const char *name)
{
	return (T*)this->indexer.get(name);
}

template <class T>
void IndexedList<T>::clear()
{
	this->indexer.clear();
	this->elements.clear();
}

template <class T>
class std::list<T>::iterator IndexedList<T>::begin()
{
	return this->elements.begin();
}

template <class T>
class std::list<T>::iterator IndexedList<T>::end()
{
	return this->elements.end();
}

#endif	/* INDEXEDLIST_H_ */
