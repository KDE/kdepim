/*
    This file is part of libkcal.

    Copyright (c) 1998 Preston Brown <pbrown@kde.org>
    Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <typeinfo>
#include <stdlib.h>

#include <tqdatetime.h>
#include <tqstring.h>
#include <tqptrlist.h>

#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>
#include <kstandarddirs.h>

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

class ResourceLocal::Private
{
  public:
    TQDateTime mLastModified;
};

ResourceLocal::ResourceLocal( const KConfig* config )
  : ResourceCached( config ), mLock( 0 )
{
  if ( config ) {
    TQString url = config->readPathEntry( "CalendarURL" );
    mURL = KURL( url );

    TQString format = config->readEntry( "Format" );
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

ResourceLocal::ResourceLocal( const TQString& fileName )
  : ResourceCached( 0 )
{
  mURL = KURL( fileName );
  mFormat = new ICalFormat();
  init();
}


void ResourceLocal::writeConfig( KConfig* config )
{
  kdDebug(5800) << "ResourceLocal::writeConfig()" << endl;

  ResourceCalendar::writeConfig( config );
  config->writePathEntry( "CalendarURL", mURL.prettyURL() );
  TQString typeID = typeid( *mFormat ).name();

  if ( typeid( *mFormat ) == typeid( ICalFormat ) )
    config->writeEntry( "Format", "ical" );
  else if ( typeid( *mFormat ) == typeid( VCalFormat ) ) // if ( typeID == "ICalFormat" )
    config->writeEntry( "Format", "vcal" );
  else
    kdDebug(5800) << "ERROR: Unknown format type" << endl;
}

void ResourceLocal::init()
{
  d = new ResourceLocal::Private;

  setType( "file" );

  setSavePolicy( SaveDelayed );

  connect( &mDirWatch, TQT_SIGNAL( dirty( const TQString & ) ),
           TQT_SLOT( reload() ) );
  connect( &mDirWatch, TQT_SIGNAL( created( const TQString & ) ),
           TQT_SLOT( reload() ) );
  connect( &mDirWatch, TQT_SIGNAL( deleted( const TQString & ) ),
           TQT_SLOT( reload() ) );

  mLock = new KABC::Lock( mURL.path() );

  mDirWatch.addFile( mURL.path() );
  mDirWatch.startScan();
}


ResourceLocal::~ResourceLocal()
{
  mDirWatch.stopScan();

  close();

  delete mLock;

  delete d;
}

TQDateTime ResourceLocal::readLastModified()
{
  TQFileInfo fi( mURL.path() );
  return fi.lastModified();
}

bool ResourceLocal::doLoad()
{
  bool success;

  if ( !KStandardDirs::exists( mURL.path() ) ) {
    kdDebug(5800) << "ResourceLocal::load(): File doesn't exist yet." << endl;
    // Save the empty calendar, so the calendar file will be created.
    success = doSave();
  } else {
    success = mCalendar.load( mURL.path() );
    if ( success ) d->mLastModified = readLastModified();
  }

  return success;
}

bool ResourceLocal::doSave()
{
  bool success = mCalendar.save( mURL.path() );
  d->mLastModified = readLastModified();

  return success;
}

KABC::Lock *ResourceLocal::lock()
{
  return mLock;
}

bool ResourceLocal::doReload()
{
  kdDebug(5800) << "ResourceLocal::doReload()" << endl;

  if ( !isOpen() ) return false;

  if ( d->mLastModified == readLastModified() ) {
    kdDebug(5800) << "ResourceLocal::reload(): file not modified since last read."
              << endl;
    return false;
  }

  mCalendar.close();
  mCalendar.load( mURL.path() );
  return true;
}

void ResourceLocal::reload()
{
  if ( doReload() )
    emit resourceChanged( this );
}

void ResourceLocal::dump() const
{
  ResourceCalendar::dump();
  kdDebug(5800) << "  Url: " << mURL.url() << endl;
}

TQString ResourceLocal::fileName() const
{
  return mURL.path();
}

bool ResourceLocal::setFileName( const TQString &fileName )
{
  bool open = isOpen();
  if ( open ) close();
  delete mLock;
  mDirWatch.stopScan();
  mDirWatch.removeFile( mURL.path() );
  mURL = KURL( fileName );
  mLock = new KABC::Lock( mURL.path() );
  mDirWatch.addFile( mURL.path() );
  mDirWatch.startScan();
  return true;
}

bool ResourceLocal::setValue( const TQString &key, const TQString &value ) 
{
  if ( key == "File" ) {
    return setFileName( value );
  } else return false;
}



#include "resourcelocal.moc"
