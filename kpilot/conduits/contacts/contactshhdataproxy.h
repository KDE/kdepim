#ifndef CONTACTSHHDATAPROXY_H
#define CONTACTSHHDATAPROXY_H
/* contactshhdataproxy.h			KPilot
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

#include <kabc/phonenumber.h>

class PilotAddress;

class ContactsHHDataProxy : public HHDataProxy
{
public:
	/**
	 * Tries to create a new Datastore and returns whether or not it succeeded.
	 */
	/* virtual */ bool createDataStore();
	
	/**
	 * Creates a new ContactsHHDataProxy object.
	 */
	ContactsHHDataProxy( PilotDatabase *db );
	
	/**
	 * Set the phone numbers from @p list in the handheld entry
	 * @p contact as far as possible. @em No overflow handling is done at all. If
	 * the desktop has more than 5 phone entries, the remainder are dropped.
	 */
	void setPhoneNumbers( PilotAddress &a, const KABC::PhoneNumber::List &list );
	
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
	
	/**
	 * Implementing classes should pack and store fAppInfo into the database so
	 * that Category information is stored.
	 */
	/* virtual */ void storeAppInfo();

private:
	class Private;
	Private* d;
};

#endif
