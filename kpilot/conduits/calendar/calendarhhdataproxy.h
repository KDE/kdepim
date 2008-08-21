#ifndef CALENDARHHDATAPROXY_H
#define CALENDARHHDATAPROXY_H
/* calendarhhdataproxy.h			KPilot
**
** Copyright (C) 2008 by Bertjan Broeksema <b.broeksema@kdemail.net>
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "hhdataproxy.h"

class CalendarHHDataProxy : public HHDataProxy
{
public:
	/**
	 * Tries to create a new Datastore and returns whether or not it succeeded.
	 */
	/* virtual */ bool createDataStore();
	
	/**
	 * Creates a new CalendarHHDataProxy object.
	 */
	CalendarHHDataProxy( PilotDatabase *db );
	
protected:
	/** These functions must be implemented by the subclassing conduit **/

	/**
	 * This function creates a (subclass of) HHRecord for @p rec.
	 */
	/* virtual */ HHRecord* createHHRecord( PilotRecord *rec );

	/**
	 * Implementing classes read the appinfo block and return a pointer so that
	 * category information can be read and altered.
	 */
	/* virtual */ PilotAppInfoBase* readAppInfo();
};

#endif
