/*
    This file is part of libkcal.

    Copyright (c) 1998 Preston Brown
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
    QDateTime mLastModified;
};

ResourceLocal::ResourceLocal( const KConfig* config )
  : ResourceCached( config ), mLock( 0 )
{
  if ( config ) {
    QString url = config->readPathEntry( "CalendarURL" );
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
  kdDebug(5800) << "ResourceLocal::writeConfig()" << endl;

  ResourceCalendar::writeConfig( config );
  config->writePathEntry( "CalendarURL", mURL.prettyURL() );
  QString typeID = typeid( *mFormat ).name();
  
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

  connect( &mDirWatch, SIGNAL( dirty( const QString & ) ),
           SLOT( reload() ) );
  connect( &mDirWatch, SIGNAL( created( const QString & ) ),
           SLOT( reload() ) );
  connect( &mDirWatch, SIGNAL( deleted( const QString & ) ),
           SLOT( reload() ) );

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

QDateTime ResourceLocal::readLastModified()
{
  QFileInfo fi( mURL.path() );
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

void ResourceLocal::reload()
{
  kdDebug(5800) << "ResourceLocal::reload()" << endl;

  if ( !isOpen() ) return;

  if ( d->mLastModified == readLastModified() ) {
    kdDebug(5800) << "ResourceLocal::reload(): file not modified since last read."
              << endl;
    return;
  }

  mCalendar.close();
  mCalendar.load( mURL.path() );

  emit resourceChanged( this );
}


void ResourceLocal::dump() const
{
  ResourceCalendar::dump();
  kdDebug(5800) << "  Url: " << mURL.url() << endl;
}

QString ResourceLocal::fileName() const
{
  return mURL.path();
}

#include "resourcelocal.moc"
