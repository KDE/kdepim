/*
    This file is part of kdepim.

    Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>


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

#include "exchangecalendaradaptor.h"

#include "kcal_resourceexchange.h"
#include <groupwaredownloadjob.h>
#include <groupwareuploadjob.h>
#include <kcal_groupwareprefs.h>

using namespace KCal;

ResourceExchange::ResourceExchange()
  : ResourceGroupwareBase()
{
  init();
}

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
  setFolderLister( new KPIM::FolderLister( KPIM::FolderLister::Calendar ) );
  setAdaptor( new ExchangeCalendarAdaptor() );
  
  ResourceGroupwareBase::init();
}

#include "kcal_resourceexchange.moc"
