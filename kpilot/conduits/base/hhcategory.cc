/* hhcategory.cc			KPilot
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

#include "hhcategory.h"

HHCategory::HHCategory() : Category( CSL1( "Unfiled" ) ), fRenamed( false )
	, fIndex( 0 ), fId( '0' )
{
}

HHCategory::HHCategory( QString name, bool renamed, int index, char id )
	: Category( name ), fRenamed( renamed ), fIndex( index ), fId( id )
{
}

HHCategory::HHCategory( const HHCategory &copy ) : Category( copy )
{
	fRenamed = copy.renamed();
	fIndex = copy.index();
	fId = copy.id();
}

int HHCategory::index() const
{
	return fIndex;
}

void HHCategory::setIndex( const int index )
{
	fIndex = index;
}

char HHCategory::id() const
{
	return fId;
}

void HHCategory::setId( const char id )
{
	fId = id;
}

bool HHCategory::renamed() const
{
	return fRenamed;
}
	
void HHCategory::setRenamed( bool renamed )
{
	fRenamed = renamed;
}
