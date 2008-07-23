/* akonadicontact.cc			KPilot
**
** Copyright (C) 2007 by Bertjan Broeksema <b.broeksema@kdemail.net>
** Copyright (C) 2007 by Jason "vanRijn" Kasper <vr@movingparts.net>
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

#include "akonadicontact.h"

#include "options.h"
#include "hhrecord.h"

#include "hhcontact.h"

AkonadiContact::AkonadiContact( const Akonadi::Item& item, const QDateTime& lastSync )
	: fItem( item ), fLastSyncDateTime( lastSync )
{
	// ContactsAkonadiDataProxy checks if item has an KABC::Addressee as payload
	// so we don't check that again here.
}

AkonadiContact::AkonadiContact()
{
	KABC::Addressee addressee;
	fItem.setMimeType( "text/directory" );
	fItem.setPayload<KABC::Addressee>( addressee );
}

AkonadiContact::~AkonadiContact()
{
}

void AkonadiContact::addCategory( const QString& category )
{
	FUNCTIONSETUP;
	
	KABC::Addressee a = fItem.payload<KABC::Addressee>();
	if( !a.hasCategory( category ) )
	{
		a.insertCategory( category );
	}
	
	fItem.setPayload<KABC::Addressee>( a );
}

KABC::Addressee AkonadiContact::addressee() const
{
	FUNCTIONSETUP;
	
	return fItem.payload<KABC::Addressee>();
}

QStringList AkonadiContact::categories() const
{
	FUNCTIONSETUP;
	
	return fItem.payload<KABC::Addressee>().categories();
}

int AkonadiContact::categoryCount() const
{
	FUNCTIONSETUP;
	
	return fItem.payload<KABC::Addressee>().categories().size();
}

bool AkonadiContact::containsCategory( const QString& category ) const
{
	FUNCTIONSETUP;
	
	return fItem.payload<KABC::Addressee>().hasCategory( category );
}

const QString AkonadiContact::id() const
{
	FUNCTIONSETUP;
	
	return QString::number( fItem.id() );
}

Akonadi::Item& AkonadiContact::item()
{
 FUNCTIONSETUP;
	
 return fItem;
}

bool AkonadiContact::isDeleted() const
{
	FUNCTIONSETUP;
	// As long as there Is an AkonadiContact object it is not deleted.
	return false;
}

bool AkonadiContact::isModified() const
{
	FUNCTIONSETUP;
	
	if( !fLastSyncDateTime.isValid() )
	{
		return false;
	}
	
	return fItem.modificationTime() > fLastSyncDateTime;
}

void AkonadiContact::setAddressee( const KABC::Addressee& addressee )
{
	FUNCTIONSETUP;
	
	fItem.setPayload<KABC::Addressee>( addressee );
}

void AkonadiContact::setCategory( const QString& category )
{
	FUNCTIONSETUP;
	
	KABC::Addressee a = fItem.payload<KABC::Addressee>();
	QStringList cats;
	cats << category;
	a.setCategories( cats );
	
	fItem.setPayload<KABC::Addressee>( a );
}

void AkonadiContact::setId( const QString &id )
{
	FUNCTIONSETUP;
	
	QString oldId = QString::number( fItem.id() );
	
	DEBUGKPILOT << "AkonadiContact::setId() [old,new]: [" << oldId << "," << id << "]";
	
	fItem.setId( id.toULongLong() );
}

void AkonadiContact::synced()
{
	FUNCTIONSETUP;
	// Nothing to do here.
}

QString AkonadiContact::toString() const
{
	FUNCTIONSETUP;
	
	return fItem.payload<KABC::Addressee>().toString();
}
