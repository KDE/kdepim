/* testhhdataproxy.cc			KPilot
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

#include "testhhdataproxy.h"
#include "testhhrecord.h"

#include "options.h"

TestHHDataProxy::TestHHDataProxy() : HHDataProxy( 0l ) {}

TestHHDataProxy::TestHHDataProxy( int count ) : HHDataProxy( 0l )
{
	QStringList fields;
	fields << CSL1( "f1" ) << CSL1( "f2" );
	
	for( int i = 1; i <= count; ++i )
	{
		QString id = CSL1( "hh-" ) + QString::number( i );
		
		TestHHRecord *rec = new TestHHRecord( fields, id );
		rec->setValue( CSL1( "f1" )
			, CSL1( "Value 1: " ) + QString::number( qrand() ) );
		rec->setValue( CSL1( "f2" )
			, CSL1( "Value 2: " ) + QString::number( qrand() ) );
		rec->synced();
		
		fRecords.insert( rec->id(), rec );
		fRecordsByDescription.insert( rec->description(), rec );
	}
	
	fIterator = QMapIterator<QString, Record*>( fRecords );
}

HHRecord* TestHHDataProxy::createHHRecord( PilotRecord *rec )
{
	Q_UNUSED( rec );
	return 0L;
}

bool TestHHDataProxy::isOpen() const
{
	return true;
}

void TestHHDataProxy::loadAllRecords()
{
}

bool TestHHDataProxy::commitCreate( Record *rec )
{
	Q_UNUSED( rec );
	return true;
}
	
void TestHHDataProxy::undoCommitCreate( const Record *rec )
{
	Q_UNUSED( rec );
}

bool TestHHDataProxy::commitUpdate( Record *rec )
{
	Q_UNUSED( rec );
	return true;
}
