/* RecordConduit.h			KPilot
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

#include "record.h"

#include "options.h"

Record::Record( const QStringList& fields ) : fId( QString() )
	, fFields( fields ), fModified( false )
{
}

Record::Record( const QStringList& fields, const QString &id ) : fId( id )
	, fFields( fields ), fModified( false )
{
}

Record::Record( const Record &other )
{
	fId = other.fId;
	fFields = other.fFields;
	fFieldValues = other.fFieldValues;
	fModified = other.fModified;
}

const QString Record::id() const
{
	FUNCTIONSETUP;
	
	return fId;
}
	
void Record::setId( const QString &id )
{
	FUNCTIONSETUP;
	
	fId = id;
}

QVariant Record::value( const QString &field ) const
{
	FUNCTIONSETUP;

	return fFieldValues.value( field );
}

bool Record::setValue( const QString &field, const QVariant &value )
{
	if( !fields().contains( field ) )
	{
		return false;
	}
	if( !isValid( field, value ) )
	{
		return false;
	}
	
	fFieldValues.insert( field, value );
	fModified = true;
	return true;
}

bool Record::isModified() const
{
	return fModified;
}

void Record::synced()
{
	fModified = false;
}

QString Record::toString() const
{
	QString record = CSL1( "Record: " ) + fId;
	
	if( fModified )
	{
		record += CSL1( " (M)" );
	}
	
	QStringListIterator it( fields() );
	while( it.hasNext() )
	{
		QString field = it.next();
		
		record += CSL1( "\n - " ) + field + CSL1( " = " ) + value( field ).toString();
	}
	return record;
}

bool Record::isValid( const QString &field, const QVariant &value )
{
	Q_UNUSED( field );
	Q_UNUSED( value );
	return true;
}

bool Record::operator==( const Record &other ) const
{
	return fFields == other.fFields && fFieldValues == other.fFieldValues;
}

bool Record::operator!=( const Record &other ) const
{
	return !( *this == other );
}
