/*
    This file is part of libkcal.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#include "resourcelocaldirconfig.h"

#include "resourcelocaldir.h"

using namespace KCal;

ResourceLocalDir::ResourceLocalDir( const KConfig* config )
  : ResourceCached( config ), mLock( 0 )
{
  if ( config ) {
    readConfig( config );
  }

  init();
}

ResourceLocalDir::ResourceLocalDir( const QString& dirName )
  : ResourceCached( 0 )
{
  mURL = KURL( dirName );

  init();
}


void ResourceLocalDir::readConfig( const KConfig *config )
{
  QString url = config->readPathEntry( "CalendarURL" );
  mURL = KURL( url );
}

void ResourceLocalDir::writeConfig( KConfig *config )
{
  kdDebug(5800) << "ResourceLocalDir::writeConfig()" << endl;

  ResourceCalendar::writeConfig( config );

  config->writePathEntry( "CalendarURL", mURL.prettyURL() );
}

void ResourceLocalDir::init()
{
  setType( "dir" );

  mOpen = false;

  connect( &mDirWatch, SIGNAL( dirty( const QString & ) ),
           SLOT( reload( const QString & ) ) );
  connect( &mDirWatch, SIGNAL( created( const QString & ) ),
           SLOT( reload( const QString & ) ) );
  connect( &mDirWatch, SIGNAL( deleted( const QString & ) ),
           SLOT( reload( const QString & ) ) );

  mLock = new KABC::Lock( mURL.path() );

  mDirWatch.addDir( mURL.path(), true );
  mDirWatch.startScan();
}


ResourceLocalDir::~ResourceLocalDir()
{
  close();

  delete mLock;
}

bool ResourceLocalDir::doOpen()
{
  kdDebug(5800) << "Opening resource " << resourceName() << " with URL " << mURL.prettyURL() << endl;

  mOpen = true;

  return true;
}

bool ResourceLocalDir::doLoad()
{
  kdDebug(5800) << "ResourceLocalDir::load()" << endl;

  if ( !mOpen ) return true;

  mCalendar.close();
  bool success = true;

  QString dirName = mURL.path();
  if ( !KStandardDirs::exists( dirName ) ) {
    kdDebug(5800) << "ResourceLocalDir::load(): Directory doesn't exist yet. Creating it..." << endl;
    
    // Create the directory. Use 0775 to allow group-writable if the umask 
    // allows it (permissions will be 0775 & ~umask). This is desired e.g. for
    // group-shared directories!
    success = KStandardDirs::makeDir( dirName, 0775 );
  } else {

    kdDebug(5800) << "ResourceLocalDir::load(): '" << dirName << "'" << endl;

    QDir dir( dirName );

    QStringList entries = dir.entryList( QDir::Files | QDir::Readable );

    QStringList::ConstIterator it;
    for( it = entries.begin(); it != entries.end(); ++it ) {
      if ( (*it).endsWith( "~" ) ) // is backup file, ignore it
        continue;

      QString fileName = dirName + "/" + *it;
      kdDebug(5800) << " read '" << fileName << "'" << endl;
      CalendarLocal cal( mCalendar.timeZoneId() );
      cal.load( fileName );
      Incidence::List incidences = cal.rawIncidences();
      Incidence *i = incidences.first();
      if ( i ) mCalendar.addIncidence( i->clone() );
    }
  }

  return success;
}

bool ResourceLocalDir::doSave()
{
  kdDebug(5800) << "ResourceLocalDir::save()" << endl;

  if ( !mOpen ) return true;

  Incidence::List incidences = mCalendar.rawIncidences();

  Incidence::List::ConstIterator it;
  for( it = incidences.begin(); it != incidences.end(); ++it ) {
    Incidence *i = *it;
    QString fileName = mURL.path() + "/" + i->uid();
    kdDebug(5800) << "writing '" << fileName << "'" << endl;

    CalendarLocal cal( mCalendar.timeZoneId() );
    cal.addIncidence( i->clone() );
    cal.save( fileName );
  }

  return true;
}

KABC::Lock *ResourceLocalDir::lock()
{
  return mLock;
}

void ResourceLocalDir::reload( const QString &file )
{
  kdDebug(5800) << "ResourceLocalDir::reload()" << endl;

  if ( !mOpen ) return;

  kdDebug(5800) << "  File: '" << file << "'" << endl;

  mCalendar.close();
  load();

  emit resourceChanged( this );
}

void ResourceLocalDir::doClose()
{
  if ( !mOpen ) return;

  mCalendar.close();
  mOpen = false;
}


void ResourceLocalDir::deleteEvent(Event *event)
{
  kdDebug(5800) << "ResourceLocalDir::deleteEvent" << endl;
  if ( deleteIncidenceFile(event) )
    mCalendar.deleteEvent( event );
}


void ResourceLocalDir::deleteTodo(Todo *todo)
{
  if ( deleteIncidenceFile(todo) )
    mCalendar.deleteTodo( todo );
}


void ResourceLocalDir::dump() const
{
  ResourceCalendar::dump();
  kdDebug(5800) << "  Url: " << mURL.url() << endl;
}

bool ResourceLocalDir::deleteIncidenceFile(Incidence *incidence)
{
  QFile file( mURL.path() + "/" + incidence->uid() );
  if ( !file.exists() )
    return true;

  mDirWatch.stopScan();
  bool removed = file.remove();
  mDirWatch.startScan();
  return removed;
}

#include "resourcelocaldir.moc"
