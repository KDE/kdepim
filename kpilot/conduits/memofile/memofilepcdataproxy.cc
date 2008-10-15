/* contactsakonadiproxy.cc			KPilot
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

#include "contactsakonadiproxy.h"

#include <kabc/addressee.h>

#include "contactsakonadirecord.h"

ContactsAkonadiProxy::ContactsAkonadiProxy( const IDMapping& mapping )
	: AkonadiDataProxy( mapping )
{
}

void ContactsAkonadiProxy::addCategory( Record* rec, const QString& category )
{
	ContactsAkonadiRecord* tar = static_cast<ContactsAkonadiRecord*>( rec );
	tar->addCategory( category );
}

void ContactsAkonadiProxy::setCategory( Record* rec, const QString& category )
{
	ContactsAkonadiRecord* tar = static_cast<ContactsAkonadiRecord*>( rec );
	tar->addCategory( category );
}

/* ***** Protected methods ***** */

AkonadiRecord* ContactsAkonadiProxy::createAkonadiRecord( const Akonadi::Item& i
	, const QDateTime& dt ) const
{
	return new ContactsAkonadiRecord( i, dt );
}

AkonadiRecord* ContactsAkonadiProxy::createDeletedAkonadiRecord( const QString& id ) const
{
	return new ContactsAkonadiRecord( id );
}

bool ContactsAkonadiProxy::hasValidPayload( const Akonadi::Item& i ) const
{
	return i.hasPayload<KABC::Addressee>();
}
