#ifndef IntId_h
#define IntId_h

#include "mailid.h"

/**
 * Mail ID for mailboxes, which identify their mails by an integer number
 */
class KornIntId : public KornMailId
{
	/**
	 * the mail id
	 */
	int _id;
public:
	/**
	 * KornIntId Destructor
	 */
	virtual ~KornIntId();

	/**
	 * KornIntId Constructor
	 * @param id: mail id
	 */
	KornIntId(int id);

	/**
	 * KornIntId Copy Constructor
	 * @param src: KornIntId to copy from
	 */
	KornIntId(const KornIntId& src);

	/**
	 * Return the mail id
	 * @return the mail id
	 */
	int getId() const {return _id;}

	/**
	 * Return a string representation of this (for debuging purposes only)
	 * @return a string representation
	 */
	virtual QString toString() const;

	/**
	 * Create an exact copy of this.
	 * @return the cloned object
	 */
	virtual KornMailId * clone() const;
};

#endif
