#ifndef MailId_h
#define MailId_h

#include <qstring.h>

/**
 * Abstract base class for mail ids. Concrete mail ids store the id.
 * Its sole purpose is to treat all possible mail id format in a unique
 * way (by inheritence).
 */
class KornMailId
{
public:
	/**
	 * KornMailId Destructor
	 */
	virtual ~KornMailId();

	/**
	 * Return a string representation of this (for debuggin purposes only)
	 * @return a string representation
	 */
	virtual QString toString() const = 0;

	/**
	 * Create an exact copy of this.
	 * @return the cloned object
	 */
	virtual KornMailId * clone() const = 0;
};


#endif
