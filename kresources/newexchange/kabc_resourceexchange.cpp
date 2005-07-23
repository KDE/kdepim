/*
    This file is part of kdepim.

    Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>


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

#include "exchangeaddressbookadaptor.h"

#include "kabc_resourceexchange.h"
#include "groupwaredownloadjob.h"
#include "groupwareuploadjob.h"
#include "kresources_groupwareprefs.h"

using namespace KABC;

ResourceExchange::ResourceExchange( const KConfig *config )
  : ResourceGroupwareBase( config )
{
  init();
  if ( config ) readConfig( config );
}

void ResourceExchange::init()
{
  setType( "ResourceExchange" );
  setPrefs( createPrefs() );
  setFolderLister( new KPIM::FolderLister( KPIM::FolderLister::AddressBook ) );
  setAdaptor( new ExchangeAddressBookAdaptor() );
  
  ResourceGroupwareBase::init();
}

#include "kabc_resourceexchange.moc"
