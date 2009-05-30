/* contactshhrecord.cc			KPilot
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

#include "contactshhrecord.h"

#include <QtCore/QDateTime>

#include "options.h"
#include "pilotAddress.h"

ContactsHHRecord::ContactsHHRecord( PilotRecord *record, const QString &category ) 
	: HHRecord( record, category )
{
}

QString ContactsHHRecord::description() const
{
	PilotAddress pa = pilotAddress();
	return pa.getField( entryFirstname ) + ' ' + pa.getField( entryLastname );
}

bool ContactsHHRecord::equal( const HHRecord* other ) const
{
	FUNCTIONSETUP;
	
	const ContactsHHRecord* hrOther = static_cast<const ContactsHHRecord*>( other );
	return hrOther->pilotAddress() == PilotAddress( fRecord );
}

PilotAddress ContactsHHRecord::pilotAddress() const
{
	return PilotAddress( fRecord );
}

void ContactsHHRecord::setPilotAddress( const PilotAddress& address )
{
	// Free the old data.
	KPILOT_DELETE( fRecord );
	// And set it to the updated address.
	fRecord = address.pack();
}

QString ContactsHHRecord::toString() const
{
	PilotAddress pa = pilotAddress();
	QString rs = id();
	rs += CSL1( ":" ) + pa.getField( entryFirstname );
	rs += CSL1( ":" ) + pa.getField( entryLastname );
	return rs;
}
