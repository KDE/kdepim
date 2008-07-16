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

AkonadiContact::AkonadiContact()
{
	// TODO: Implement
}

AkonadiContact::~AkonadiContact()
{
}

QStringList AkonadiContact::categories() const
{
	FUNCTIONSETUP;
	// TODO: Implement
	return QStringList();
}

int AkonadiContact::categoryCount() const
{
	FUNCTIONSETUP;
	// TODO: Implement
	return 0;
}

bool AkonadiContact::containsCategory( const QString& category ) const
{
	FUNCTIONSETUP;
	// TODO: Implement
	return false;
}

bool AkonadiContact::equal( const Record* other ) const
{
	FUNCTIONSETUP;
	// TODO: Implement
	return false;
}

const QString AkonadiContact::id() const
{
	FUNCTIONSETUP;
	// TODO: Implement
	return QString();
}

bool AkonadiContact::isDeleted() const
{
	FUNCTIONSETUP;
	// TODO: Implement
	return false;
}

bool AkonadiContact::isModified() const
{
	FUNCTIONSETUP;
	// TODO: Implement
	return false;
}

void AkonadiContact::setId( const QString &id )
{
	FUNCTIONSETUP;
	// TODO: Implement
}

void AkonadiContact::synced()
{
	FUNCTIONSETUP;
	// TODO: Implement
}

QString AkonadiContact::toString() const
{
	FUNCTIONSETUP;
	// TODO: Implement
	return QString();
}
