/* category.cc			KPilot
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

#include "category.h"

Category::Category( QString name, bool renamed, int index, char id )
	: fName( name ), fRenamed( renamed ), fIndex( index ), fId( id )
{
}

int Category::index() const
{
	return fIndex;
}

void Category::setIndex( const int index )
{
	fIndex = index;
}

char Category::id() const
{
	return fId;
}

void Category::setId( const char id )
{
	fId = id;
}

QString Category::name() const
{
	return fName;
}

void Category::setName( const QString &name )
{
	fName = name;
}
