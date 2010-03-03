/**
 * \file Observer.h
 * 8/2/2010 LB Initial implementation
 * 
 */


#ifndef OBSERVER_H
#define OBSERVER_H

// ** INCLUDES **

class Subject;

/**
 * Observer is a abstract base class for implementing the Observer pattern.
 * It defines an updating interface for objects that should be notified
 * of changes in a Subject.
 */
class Observer
{

public:
	virtual ~Observer();
	virtual void Update(Subject* theChangedSubject) = 0;

protected:
	Observer();

};

#endif // OBSERVER_H