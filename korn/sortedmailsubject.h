/*
 * Copyright (C) 2004, Mart Kelder (mart.kde@hccnet.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef MK_SORTEDMAILSUBJECT_H
#define MK_SORTEDMAILSUBJECT_H

#include "mailsubject.h"
#include <q3ptrlist.h>

/*
 * This class is a normal QPtrList with an overloaded function compareItems.
 * This way, it is possible to sort this list on date.
 */

class SortedMailSubject : public Q3PtrList< KornMailSubject >
{
public:
	SortedMailSubject() : Q3PtrList< KornMailSubject >() {}
	SortedMailSubject( const SortedMailSubject& sms ) : Q3PtrList< KornMailSubject >( sms ) {}
	~SortedMailSubject() {}
	
	virtual int compareItems ( Q3PtrCollection::Item item1, Q3PtrCollection::Item item2 );
};

#endif //MK_SORTEDMAILSUBJECT_H
