/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Till Adam <adam@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "ogofolderlister.h"
#include "ogocalendaradaptor.h"

#include "kcal_resourceopengroupware.h"

using namespace KCal;

ResourceOpenGroupware::ResourceOpenGroupware()
  : ResourceGroupwareBase()
{
  init();
}

ResourceOpenGroupware::ResourceOpenGroupware( const KConfig *config )
  : ResourceGroupwareBase( config )
{
  init();
  if ( config ) readConfig( config );
}

void ResourceOpenGroupware::init()
{
  setType( "ResourceOpenGroupware" );
  setPrefs( createPrefs() );
  setFolderLister( new KPIM::OGoFolderLister( KPIM::FolderLister::Calendar ) );
  setAdaptor( new OGoCalendarAdaptor() );
  
  ResourceGroupwareBase::init();
}

#include "kcal_resourceopengroupware.moc"
