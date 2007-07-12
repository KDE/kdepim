/* testrecord.cc			KPilot
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

#include "testrecord.h"

#include "options.h"

TestRecord::TestRecord()
{
	fFields << CSL1( "f1" ) << CSL1( "f2" );
}

TestRecord::TestRecord( const QString & id ) : Record( id ), fModified(false)
{
	fFields << CSL1( "f1" ) << CSL1( "f2" );
}

TestRecord::TestRecord( const QStringList &fields )
{
	fFields = fields;
}

TestRecord::~TestRecord()
{
}

// Not accurate but that doesn't matter, we won't test this anyway.
Record* TestRecord::duplicate()
{
	return new TestRecord( fId );
}

bool TestRecord::setValue( const QString &field, const QVariant &value )
{
	// Just put everything in it and set the modified flag.
	fFieldValues.insert( field, value );
	fModified = true;
	return true;
}

bool TestRecord::isModified() const
{
	return fModified; 
}

const QStringList TestRecord::fields() const
{
	return fFields;
}

QString TestRecord::toString() const
{
	return QString();
}

// For test purposes.
void TestRecord::setUnmodified()
{
	fModified = false;
}
