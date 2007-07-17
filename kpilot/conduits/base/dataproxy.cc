/* dataproxy.cc			KPilot
**
** Copyright (C) 2007 by Bertjan Broeksema
** Copyright (C) 2007 by Jason "vanRijn" Kasper
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

#include "options.h"

#include "dataproxy.h"

#include "record.h"

DataProxy::DataProxy() : fIterator( fRecords )
{
	FUNCTIONSETUP;
	fLastId = 0;
}

DataProxy::~DataProxy()
{
	FUNCTIONSETUP;
}

QString DataProxy::create( Record *record )
{
	FUNCTIONSETUP;
	
	// Temporary id.
	fLastId = fLastId--;
	QString recordId = QString::number( fLastId );
	
	// Make sure that the new record has the right id and add the record.
	record->setId( recordId );
	fRecords.insert( recordId, record );
	
	// Update rollback/volatility information.
	fCreated.append( recordId );
	fCounter.created();
	
	return recordId;
}

void DataProxy::remove( const QString &id )
{
	FUNCTIONSETUP;
	
	Record *rec = fRecords.value( id );
	if( rec == 0L )
	{
		// No record
		return;
	}
	
	// Remove record.
	fRecords.remove( id );
	
	// Update rollback/volatility information.
	fDeleted.append( rec );
	fCounter.deleted();
}

void DataProxy::update( const QString &id, Record *newRecord )
{
	FUNCTIONSETUP;
	
	Record *oldRecord = fRecords.value( id );
	if( oldRecord == 0L )
	{
		// No record, should not happen.
		DEBUGKPILOT << fname << ": There is no record with id " << id
			<< ". Record not updated and not added" << endl;
		return;
	}
	
	// Make sure that the new record has the right id and update the old record.
	newRecord->setId( id );
	fRecords.insert( id, newRecord );
	
	// Update rollback/volatility information.
	fUpdated.append( oldRecord );
	fCounter.updated();
}

QList<QString> DataProxy::ids() const
{
	return fRecords.keys();
}

const CUDCounter* DataProxy::counter() const
{
	FUNCTIONSETUP;
	
	return &fCounter;
}

void DataProxy::syncFinished()
{
	FUNCTIONSETUP;
	
	fCounter.setEndCount( fRecords.size() );
}

void DataProxy::setIterateMode( const Mode m )
{
	FUNCTIONSETUP;
	
	fMode = m;
}

unsigned int DataProxy::recordCount() const
{
	return fRecords.size();
}

Record* DataProxy::find( const QString &id ) const
{
	FUNCTIONSETUP;
	return fRecords.value( id );
}

void DataProxy::resetIterator()
{
	fIterator = QMapIterator<QString, Record*>( fRecords );
}

bool DataProxy::hasNext() const
{
	FUNCTIONSETUP;
	
	if( fMode == All )
	{
		return fIterator.hasNext();
	}
	else
	{
		QMapIterator<QString, Record*> tmpIt = fIterator;
		while( tmpIt.hasNext() )
		{
			Record *rec = tmpIt.next().value();
			if( rec->isModified() )
			{
				return true;
			}
		}
	}
	
	return false;
}

Record* DataProxy::next()
{
	FUNCTIONSETUP;
	
	if( fMode == All )
	{
			return fIterator.next().value();
	}
	else
	{
		while( fIterator.hasNext() )
		{
			Record *rec = fIterator.next().value();
			if( rec->isModified() )
			{
				return rec;
			}
		}
	}
	
	return 0L;
}
