//Copyright (C) 2004, Mart Kelder

#ifndef SORTEDMAILSUBJECT_H
#define SORTEDMAILSUBJECT_H

#include "mailsubject.h"
#include <qptrlist.h>

/*
 * This class is a normal QPtrList with an overloaded function compareItems.
 * This way, it is possible to sort this list on date.
 */

class SortedMailSubject : public QPtrList< KornMailSubject >
{
public:
	SortedMailSubject() : QPtrList< KornMailSubject >() {}
	SortedMailSubject( const SortedMailSubject& sms ) : QPtrList< KornMailSubject >( sms ) {}
	~SortedMailSubject() {}
	
	virtual int compareItems ( QPtrCollection::Item item1, QPtrCollection::Item item2 );
};

#endif
