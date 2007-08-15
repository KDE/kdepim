/* recordbase.cc			KPilot
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

#include "recordbase.h"
#include "options.h"

RecordBase::RecordBase( const QStringList& fields, const QString &id )
	: fId( id ), fFields( fields ), fModified( false ), fDeleted( false )
{
}

void RecordBase::setDeleted()
{
	fDeleted = true;
}

void RecordBase::setModified()
{
	fModified = true;
}

const QString RecordBase::id() const 
{
	return fId;
}
	
void RecordBase::setId( const QString &id )
{
	fId = id;
}

QVariant RecordBase::value( const QString &field ) const
{
	return fValues.value( field );
}

bool RecordBase::setValue( const QString &field, const QVariant &value )
{
	fValues.insert( field, value );
	fModified = true;
	return true;
}

bool RecordBase::isModified() const
{
	return fModified || isDeleted();
}

bool RecordBase::isDeleted() const
{
	return fDeleted;
}

void RecordBase::synced()
{
	fDeleted = false;
	fModified = false;
}

QString RecordBase::toString() const
{
	return QString();
}

const QStringList RecordBase::fields() const
{
	return fFields;
}
