/* contactsakonadirecord.cc			KPilot
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

#include "contactsakonadirecord.h"

#include <kabc/addressee.h>

#include "options.h"

ContactsAkonadiRecord::ContactsAkonadiRecord( const Akonadi::Item& i, const QDateTime& dt )
	: AkonadiRecord( i, dt )
{
}

ContactsAkonadiRecord::ContactsAkonadiRecord( const QString& id ) : AkonadiRecord( id )
{
	Akonadi::Item item;
	item.setPayload<KABC::Addressee>( KABC::Addressee() );
	item.setMimeType( "text/directory" );
	setItem( item );
	// Set item changes the Id of the record to the item id which is invalid in case
	// of deleted records.
	setId(id);
}

ContactsAkonadiRecord::~ContactsAkonadiRecord()
{
}

void ContactsAkonadiRecord::addCategory( const QString& category )
{
	FUNCTIONSETUP;
	
	KABC::Addressee a = item().payload<KABC::Addressee>();
	if( !a.hasCategory( category ) )
	{
		a.insertCategory( category );
	}
	
	item().setPayload<KABC::Addressee>( a );
}

KABC::Addressee ContactsAkonadiRecord::addressee() const
{
	FUNCTIONSETUP;
	
	return item().payload<KABC::Addressee>();
}

QStringList ContactsAkonadiRecord::categories() const
{
	FUNCTIONSETUP;
	
	return item().payload<KABC::Addressee>().categories();
}

int ContactsAkonadiRecord::categoryCount() const
{
	FUNCTIONSETUP;
	
	return item().payload<KABC::Addressee>().categories().size();
}

bool ContactsAkonadiRecord::containsCategory( const QString& category ) const
{
	FUNCTIONSETUP;
	
	return item().payload<KABC::Addressee>().categories().contains( category );
}

QString ContactsAkonadiRecord::description() const
{
	return addressee().givenName() + ' ' + addressee().familyName();
}

void ContactsAkonadiRecord::setAddressee( const KABC::Addressee& addressee )
{
	FUNCTIONSETUP;
	
	Akonadi::Item i = item();
	i.setPayload<KABC::Addressee>( addressee );
	setItem( i );
}

QString ContactsAkonadiRecord::toString() const
{
	return CSL1( "IMPLEMENT: conduits/contacts/contactsakonadirecord.cc::toString()" );
}
