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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <typeinfo>
#include <stdlib.h>

#include <QDateTime>
#include <QString>

#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>
#include <kconfig.h>
#include <kstandarddirs.h>

#include "vcaldrag.h"
#include "vcalformat.h"
#include "icalformat.h"
#include "exceptions.h"
#include "calendarlocal.h"
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
  mURL = KUrl( dirName );

  init();
}


void ResourceLocalDir::readConfig( const KConfig *config )
{
  QString url = config->readPathEntry( "CalendarURL" );
  mURL = KUrl( url );
}

void ResourceLocalDir::writeConfig( KConfig *config )
{
  kDebug(5800) << "ResourceLocalDir::writeConfig()" << endl;

  ResourceCalendar::writeConfig( config );

  config->writePathEntry( "CalendarURL", mURL.prettyUrl() );
}

void ResourceLocalDir::init()
{
  setType( "dir" );

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

bool ResourceLocalDir::doLoad()
{
  kDebug(5800) << "ResourceLocalDir::load()" << endl;

  mCalendar.close();
  QString dirName = mURL.path();
  bool success = true;

  if ( !( KStandardDirs::exists( dirName ) || KStandardDirs::exists( dirName + '/') ) ) {
    kDebug(5800) << "ResourceLocalDir::load(): Directory '" << dirName << "' doesn't exist yet. Creating it..." << endl;

    // Create the directory. Use 0775 to allow group-writable if the umask
    // allows it (permissions will be 0775 & ~umask). This is desired e.g. for
    // group-shared directories!
    success = KStandardDirs::makeDir( dirName, 0775 );
  } else {

    kDebug(5800) << "ResourceLocalDir::load(): '" << dirName << "'" << endl;
    QDir dir( dirName );

    QStringList entries = dir.entryList( QDir::Files | QDir::Readable );

    QStringList::ConstIterator it;
    for( it = entries.begin(); it != entries.end(); ++it ) {
      if ( (*it).endsWith( "~" ) ) // is backup file, ignore it
        continue;

      QString fileName = dirName + '/' + *it;
      kDebug(5800) << " read '" << fileName << "'" << endl;
      CalendarLocal cal( mCalendar.timeZoneId() );
      if ( !doFileLoad( cal, fileName ) ) {
        success = false;
      }
    }
  }

  return success;
}

bool ResourceLocalDir::doFileLoad( CalendarLocal &cal, const QString &fileName )
{
  if ( !cal.load( fileName ) )
    return false;
  Incidence::List incidences = cal.rawIncidences();
  Incidence::List::ConstIterator it;
  for ( it = incidences.constBegin(); it != incidences.constEnd(); ++it ) {
    Incidence *i = *it;
    if ( i ) mCalendar.addIncidence( i->clone() );
  }
  return true;
}

bool ResourceLocalDir::doSave()
{
  Incidence::List list;

  list = addedIncidences();
  for (Incidence::List::iterator it = list.begin(); it != list.end(); ++it)
    doSave(*it);

  list = changedIncidences();
  for (Incidence::List::iterator it = list.begin(); it != list.end(); ++it)
    doSave(*it);

  return true;
}

bool ResourceLocalDir::doSave( Incidence *incidence )
{
  mDirWatch.stopScan();  // do prohibit the dirty() signal and a following reload()

  QString fileName = mURL.path() + '/' + incidence->uid();
  kDebug(5800) << "writing '" << fileName << "'" << endl;

  CalendarLocal cal( mCalendar.timeZoneId() );
  cal.addIncidence( incidence->clone() );
  cal.save( fileName );

  mDirWatch.startScan();

  return true;
}

KABC::Lock *ResourceLocalDir::lock()
{
  return mLock;
}

void ResourceLocalDir::reload( const QString &file )
{
  kDebug(5800) << "ResourceLocalDir::reload()" << endl;

  if ( !isOpen() ) return;

  kDebug(5800) << "  File: '" << file << "'" << endl;

  mCalendar.close();
  load();

  emit resourceChanged( this );
}


bool ResourceLocalDir::deleteEvent(Event *event)
{
  kDebug(5800) << "ResourceLocalDir::deleteEvent" << endl;
  if ( deleteIncidenceFile(event) )
    return( mCalendar.deleteEvent( event ) );
  else
    return( false );
}


bool ResourceLocalDir::deleteTodo(Todo *todo)
{
  if ( deleteIncidenceFile(todo) )
    return( mCalendar.deleteTodo( todo ) );
  else
    return( false );
}


bool ResourceLocalDir::deleteJournal( Journal *journal )
{
  if ( deleteIncidenceFile( journal ) )
    return( mCalendar.deleteJournal( journal ) );
  else
    return( false );
}


void ResourceLocalDir::dump() const
{
  ResourceCalendar::dump();
  kDebug(5800) << "  Url: " << mURL.url() << endl;
}

bool ResourceLocalDir::deleteIncidenceFile(Incidence *incidence)
{
  QFile file( mURL.path() + '/' + incidence->uid() );
  if ( !file.exists() )
    return true;

  mDirWatch.stopScan();
  bool removed = file.remove();
  mDirWatch.startScan();
  return removed;
}

#include "resourcelocaldir.moc"
