/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "addresseeconfig.h"
#include "kabprefs.h"

using namespace KABC;

AddresseeConfig::AddresseeConfig()
{
  mAddressee = Addressee();
}

AddresseeConfig::AddresseeConfig( const Addressee &addr )
{
  mAddressee = addr;
}

void AddresseeConfig::setAddressee( const Addressee &addr )
{
  mAddressee = addr;
}

Addressee AddresseeConfig::addressee()
{
  return mAddressee;
}

void AddresseeConfig::setAutomaticNameParsing( bool value )
{
  KConfig config( "kaddressbook_addrconfig" );
  config.setGroup( mAddressee.uid() );
  config.writeEntry( "AutomaticNameParsing", value );
  config.sync();
}

bool AddresseeConfig::automaticNameParsing()
{
  KConfig config( "kaddressbook_addrconfig" );
  config.setGroup( mAddressee.uid() );
  return config.readBoolEntry( "AutomaticNameParsing",
                               KABPrefs::instance()->automaticNameParsing() );
}

void AddresseeConfig::setNoDefaultAddrTypes( const QValueList<int> &types )
{
  KConfig config( "kaddressbook_addrconfig" );
  config.setGroup( mAddressee.uid() );
  config.writeEntry( "NoDefaultAddrTypes", types );
  config.sync();
}

QValueList<int> AddresseeConfig::noDefaultAddrTypes() const
{
  KConfig config( "kaddressbook_addrconfig" );
  config.setGroup( mAddressee.uid() );
  return config.readIntListEntry( "NoDefaultAddrTypes" );
}

void AddresseeConfig::setCustomFields( const QStringList &fields )
{
  KConfig config( "kaddressbook_addrconfig" );
  config.setGroup( mAddressee.uid() );
  config.writeEntry( "LocalCustomFields", fields );
  config.sync();
}

QStringList AddresseeConfig::customFields() const
{
  KConfig config( "kaddressbook_addrconfig" );
  config.setGroup( mAddressee.uid() );
  return config.readListEntry( "LocalCustomFields" );
}

void AddresseeConfig::remove()
{
  KConfig config( "kaddressbook_addrconfig" );
  config.deleteGroup( mAddressee.uid() );
}
