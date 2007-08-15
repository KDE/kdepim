/* testrecord.cc			KPilot
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

#include "testrecord.h"
#include "testhhrecord.h"
#include "options.h"

TestRecord::TestRecord( const QStringList& fields )
	: fFields( fields ), fModified( false ), fDeleted( false )
{
}


TestRecord::TestRecord( const QStringList& fields, const QString &id )
	: fId( id ), fFields( fields ), fModified( false ), fDeleted( false )
{
}

TestRecord::TestRecord( const TestHHRecord *other )
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
}

TestRecord::TestRecord( const TestRecord *other )
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
}

void TestRecord::setDeleted()
{
	fDeleted = true;
}

void TestRecord::setModified()
{
	fModified = true;
}

const QString TestRecord::id() const 
{
	return fId;
}
	
void TestRecord::setId( const QString &id )
{
	fId = id;
}

QVariant TestRecord::value( const QString &field ) const
{
	return fValues.value( field );
}

bool TestRecord::setValue( const QString &field, const QVariant &value )
{
	fValues.insert( field, value );
	fModified = true;
	return true;
}

bool TestRecord::isModified() const
{
	return fModified || isDeleted();
}

bool TestRecord::isDeleted() const
{
	return fDeleted;
}

void TestRecord::synced()
{
	fDeleted = false;
	fModified = false;
}

QString TestRecord::toString() const
{
	QString representation = fId + CSL1( " [" );
	QStringListIterator it(fFields);
	
	while( it.hasNext() )
	{
		QString field = it.next();
		
		representation += CSL1( " " );
		representation += field;
		representation += CSL1( "=" );
		representation += fValues.value( field ).toString();
	}
	
	representation += CSL1( " ]" );
	
	return representation;
}

const QStringList TestRecord::fields() const
{
	return fFields;
}

TestRecord* TestRecord::duplicate() const
{
	return new TestRecord( this );
}


bool TestRecord::equal( const Record *rec ) const
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
