/*
    This file is part of libkcal.
    Copyright (c) 1998 Preston Brown
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <typeinfo>
#include <stdlib.h>

#include <qdatetime.h>
#include <qstring.h>
#include <qptrlist.h>

#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>

#include "vcaldrag.h"
#include "vcalformat.h"
#include "icalformat.h"
#include "exceptions.h"
#include "incidence.h"
#include "event.h"
#include "todo.h"
#include "journal.h"
#include "filestorage.h"

#include <kresources/configwidget.h>

#include "resourcelocalconfig.h"

#include "resourcelocal.h"

using namespace KCal;

extern "C"
{
  void *init_libkcal()
  {
    return new KRES::PluginFactory<ResourceLocal,ResourceLocalConfig>();
  }
}


ResourceLocal::ResourceLocal( const KConfig* config )
  : ResourceCached( config )
{
  if ( config ) {
    QString url = config->readEntry( "CalendarURL" );
    mURL = KURL( url );

    QString format = config->readEntry( "Format" );
    if ( format == "ical" )
      mFormat = new ICalFormat();
    else if ( format == "vcal" )
      mFormat = new VCalFormat();
    else {
      mFormat = new ICalFormat();
    }
  } else {
    mURL = KURL();
    mFormat = new ICalFormat();
  }
  init();
}

ResourceLocal::ResourceLocal( const QString& fileName ) 
  : ResourceCached( 0 )
{
  mURL = KURL( fileName );
  mFormat = new ICalFormat();
  init();
}


void ResourceLocal::writeConfig( KConfig* config )
{
  kdDebug() << "ResourceLocal::writeConfig()" << endl;

  ResourceCalendar::writeConfig( config );
  config->writeEntry( "CalendarURL", mURL.prettyURL() );
  QString typeID = typeid( *mFormat ).name();
  
  if ( typeid( *mFormat ) == typeid( ICalFormat ) )
    config->writeEntry( "Format", "ical" );
  else if ( typeid( *mFormat ) == typeid( VCalFormat ) ) // if ( typeID == "ICalFormat" )
    config->writeEntry( "Format", "vcal" );
  else
    kdDebug() << "ERROR: Unknown format type" << endl;
}

void ResourceLocal::init()
{
  setType( "file" );

  mOpen = false;

  connect( &mDirWatch, SIGNAL( dirty( const QString & ) ),
           SLOT( reload() ) );
  connect( &mDirWatch, SIGNAL( created( const QString & ) ),
           SLOT( reload() ) );
  connect( &mDirWatch, SIGNAL( deleted( const QString & ) ),
           SLOT( reload() ) );

  mDirWatch.addFile( mURL.path() );
  mDirWatch.startScan();
}


ResourceLocal::~ResourceLocal()
{
  close();
}

bool ResourceLocal::doOpen()
{
  kdDebug(5800) << "Opening resource " << resourceName() << " with URL " << mURL.prettyURL() << endl;

  mOpen = true;

  return true;
}

bool ResourceLocal::load()
{
  if ( !mOpen ) return true;
  
  return mCalendar.load( mURL.path() );
}

bool ResourceLocal::save()
{
  if ( !mOpen ) return true;

  return mCalendar.save( mURL.path() );
}

void ResourceLocal::reload()
{
  if ( !mOpen ) return;

  mCalendar.close();
  mCalendar.load( mURL.path() );

  emit resourceChanged( this );
}

void ResourceLocal::doClose()
{
  if ( !mOpen ) return;

  mCalendar.close();
  mOpen = false;
}


void ResourceLocal::update(IncidenceBase *)
{
}

void ResourceLocal::dump() const
{
  ResourceCalendar::dump();
  kdDebug(5800) << "  Url: " << mURL.url() << endl;
}

#include "resourcelocal.moc"
