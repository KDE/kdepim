/* testhhrecord.cc			KPilot
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

#include "testhhrecord.h"
#include "testrecord.h"

#include "options.h"
#include "pilotRecord.h"

TestHHRecord::TestHHRecord( const QStringList& fields, const QString &id )
	: HHRecord( 0L, CSL1( "Unfiled" ) ), fId( id ), fFields( fields ), fModified( false )
		, fDeleted( false ), fArchived( false )
{
	pi_buffer_t *buf = pi_buffer_new( QString( "" ).size() );
	Pilot::toPilot( QString(""), buf->data, 0 );
		
	fRecord = new PilotRecord( buf, 0, 0, 0);
	fRecord->setCategory( 0 );
}

TestHHRecord::TestHHRecord( const TestHHRecord *other )
	: HHRecord( 0L, CSL1( "Unfiled" ) )
{
	fId = other->id();
	fFields = other->fields();
	
	QStringListIterator it(fFields);
	
	while( it.hasNext() )
	{
		QString field = it.next();
		
		setValue( field, other->value( field ) );
	}
	
	fModified = other->isModified();
	fDeleted = other->isDeleted();
	fArchived = other->isArchived();;
}

TestHHRecord::TestHHRecord( const TestRecord *other ) 
	: HHRecord( 0L, CSL1( "Unfiled" ) )
{
	fId = other->id();
	fFields = other->fields();
	
	QStringListIterator it(fFields);
	
	while( it.hasNext() )
	{
		QString field = it.next();
		
		setValue( field, other->value( field ) );
	}
	
	fModified = other->isModified();
	fDeleted = other->isDeleted();
	fArchived = false;
}

QString TestHHRecord::description() const
{
	return QString();
}

void TestHHRecord::setArchived()
{
	setDeleted();
	fArchived = true;
}

void TestHHRecord::setDeleted()
{
	fDeleted = true;
}

void TestHHRecord::setModified()
{
	fModified = true;
}

const QString TestHHRecord::id() const 
{
	return fId;
}

void TestHHRecord::setCategory( int id, const QString& category )
{
	Q_UNUSED( id );
	fCategory = category;
}

void TestHHRecord::setId( const QString &id )
{
	fId = id;
}

QVariant TestHHRecord::value( const QString &field ) const
{
	return fValues.value( field );
}

bool TestHHRecord::setValue( const QString &field, const QVariant &value )
{
	fValues.insert( field, value );
	fModified = true;
	return true;
}

bool TestHHRecord::isModified() const
{
	return fModified || isDeleted();
}

bool TestHHRecord::isDeleted() const
{
	return fDeleted;
}

void TestHHRecord::synced()
{
	fDeleted = false;
	fModified = false;
}

QString TestHHRecord::toString() const
{
	QString representation = fId + CSL1( " [" );
	QStringListIterator it(fFields);
	
	while( it.hasNext() )
	{
		QString field = it.next();
		
		representation += CSL1( " " );
		representation += field;
		representation += CSL1( "=\"" );
		representation += fValues.value( field ).toString();
		representation += CSL1( "\"" );
	}
	
	representation += CSL1( " ]" );
	
	return representation;
}

const QStringList TestHHRecord::fields() const
{
	return fFields;
}

TestHHRecord* TestHHRecord::duplicate() const
{
	return new TestHHRecord( this );
}

bool TestHHRecord::equal( const HHRecord *rec ) const
{
	if( const TestHHRecord *other = dynamic_cast<const TestHHRecord*>( rec ) )
	{
		QStringList fields = other->fields();
		QStringListIterator it(fields);
		
		bool allEqual = true;
		
		while( it.hasNext() )
		{
			QString field = it.next();
			
			allEqual = allEqual && ( fValues.value( field ) == other->value( field ) );
		}
		
		return allEqual && (fields == fFields);
	}
	
	return false;
}
