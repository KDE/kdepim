/*
    Copyright (c) 2005 by Volker Krause <vkrause@kde.org>
    Copyright (c) 2005 by Florian Schröder <florian@deltatauchi.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <klocale.h>

#include "sloxfolder.h"

SloxFolder::SloxFolder( const QString &id, const QString &parentId, const QString &type, const QString &name, bool def ) :
  item( 0 ),
  mId( id ),
  mParentId( parentId ),
  mName( name ),
  mDefault( def )
{
  if ( type == "calendar" )
    mType = Calendar;
  else if ( type == "task" )
    mType = Tasks;
  else if ( type == "contact" )
    mType = Contacts;
  else
    mType = Unbound;
}

QString SloxFolder::name( ) const
{
  // special cases for system folders
  if ( mName == "system_global" )
    return i18n( "Global Address Book" );
  if ( mName == "system_ldap" )
    return i18n( "Internal Address Book" );
  return mName;
}
