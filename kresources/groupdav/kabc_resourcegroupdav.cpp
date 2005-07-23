/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Till Adam <adam@kde.org>
    Copyright (c) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "groupdavaddressbookadaptor.h"
#include "kresources_groupwareprefs.h"

#include "kabc_resourcegroupdav.h"

using namespace KABC;

ResourceGroupDav::ResourceGroupDav( const KConfig *config )
  : ResourceGroupwareBase( config )
{
  init();
  if ( config ) readConfig( config );
}

void ResourceGroupDav::init()
{
  setType( "ResourceGroupDav" );
  setPrefs( createPrefs() );
  setFolderLister( new KPIM::FolderLister( KPIM::FolderLister::AddressBook ) );
  setAdaptor( new GroupDavAddressBookAdaptor() );
  
  ResourceGroupwareBase::init();
}

#include "kabc_resourcegroupdav.moc"
