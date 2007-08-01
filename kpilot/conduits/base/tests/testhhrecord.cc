/* testhhrecord.cc			KPilot
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

#include "testhhrecord.h"
#include "testrecord.h"
#include "options.h"

TestHHRecord::TestHHRecord( const QStringList& fields, const QString &id )
	: HHRecord( 0L ), fId( id ), fFields( fields ), fModified( false )
		, fDeleted( false ), fArchived( false )
{
}

TestHHRecord::TestHHRecord( const TestHHRecord *other ) : HHRecord( 0L )
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

TestHHRecord::TestHHRecord( const TestRecord *other ) : HHRecord( 0L )
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

bool TestHHRecord::equal( const Record *rec ) const
{
	if( const TestRecord *other = dynamic_cast<const TestRecord*>( rec ) )
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
	else if( const TestHHRecord *other = dynamic_cast<const TestHHRecord*>( rec ) )
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
