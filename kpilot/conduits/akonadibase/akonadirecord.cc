/* AkonadiRecord.cc			KPilot
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

#include "akonadirecord.h"

#include "options.h"
#include "hhrecord.h"

class AkonadiRecordPrivate : public QSharedData
{
public:
	Akonadi::Item fItem;
	QString fTempId;
	QDateTime fLastSyncDateTime;
	bool fDeleted;
};

AkonadiRecord::AkonadiRecord( const Akonadi::Item& item, const QDateTime& lastSync )
	: d( new AkonadiRecordPrivate )
{
	d->fItem = item;
	d->fLastSyncDateTime = lastSync;
	// Item times have UTC timeSpec
	d->fLastSyncDateTime.setTimeSpec( Qt::UTC );
	d->fDeleted = false;
}

AkonadiRecord::AkonadiRecord( const QString& id ) : d( new AkonadiRecordPrivate )
{
	d->fTempId = id;
	d->fDeleted = true;
}

AkonadiRecord::~AkonadiRecord()
{
}

const QString AkonadiRecord::id() const
{
	if( d->fTempId.isEmpty() )
	{
		return QString::number( d->fItem.id() );
	}
	else
	{
		return d->fTempId;
	}
}

Akonadi::Item AkonadiRecord::item() const
{
	FUNCTIONSETUP;
	
	return d->fItem;
}

bool AkonadiRecord::isDeleted() const
{
	FUNCTIONSETUP;
	return d->fDeleted;
}

bool AkonadiRecord::isModified() const
{
	FUNCTIONSETUP;
	
	if( !d->fLastSyncDateTime.isValid() )
	{
		// Whe the fLastSyncDateTime isn't valid, the record is most probably marked
		// for deletion and thus modified.
		return true;
	}
	
	return d->fItem.modificationTime() > d->fLastSyncDateTime;
}

void AkonadiRecord::setId( const QString &id )
{
	FUNCTIONSETUP;
	
	// Id's < 0 are temporary id's
	if( id.toLongLong() < 0 )
	{
		d->fTempId = id;
	}
	else
	{
		d->fTempId.clear();
		d->fItem.setId( id.toULongLong() );
	}
}

void AkonadiRecord::setItem( const Akonadi::Item& item )
{
	FUNCTIONSETUP;
	
	d->fItem = item;
	// Make sure that we return the right id after updating the itemobject.
	setId( QString::number( item.id() ) );
}

void AkonadiRecord::synced()
{
	FUNCTIONSETUP;
	// Nothing to do here.
}
