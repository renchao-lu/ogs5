/**
 * \file Subject.h
 * 8/2/2010 LB Initial implementation
 * 
 */


#ifndef SUBJECT_H
#define SUBJECT_H

// ** INCLUDES **
#include <list>

class Observer;

/**
 * Subject is a abstract base class for implementing the Observer pattern.
 * It knows its Observers. Any number of Observer objects may observe a
 * subject. It provides an interface for attaching and detaching Observer
 * objects.
 */
class Subject
{

public:
	virtual ~Subject();

	virtual void AttachObserver(Observer*);
	virtual void DetachObserver(Observer*);
	virtual void NotifyObserver();

protected:
	Subject();
	
private:
	std::list<Observer*>* _observers;
};

#endif // SUBJECT_H