/**
 * \file Subject.cpp
 * 8/2/2010 LB Initial implementation
 * 
 * Implementation of Subject
 */

// ** INCLUDES **
#include "Subject.h"

#include "Observer.h"

void Subject::AttachObserver( Observer* o )
{
	_observers->push_back(o);
}

void Subject::DetachObserver( Observer* o )
{
	_observers->remove(o);
}

void Subject::NotifyObserver()
{
	for (std::list<Observer*>::const_iterator it = _observers->begin();
		it != _observers->end(); ++it)
		(*it)->Update(this);
}