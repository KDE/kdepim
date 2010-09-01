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

#include <tqdatetime.h>
#include <tqfileinfo.h>
#include <tqstring.h>
#include <tqptrlist.h>

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

ResourceLocalDir::ResourceLocalDir( const TQString& dirName )
  : ResourceCached( 0 )
{
  mURL = KURL( dirName );

  init();
}


void ResourceLocalDir::readConfig( const KConfig *config )
{
  TQString url = config->readPathEntry( "CalendarURL" );
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

  setSavePolicy( SaveDelayed );

  connect( &mDirWatch, TQT_SIGNAL( dirty( const TQString & ) ),
           TQT_SLOT( reload( const TQString & ) ) );
  connect( &mDirWatch, TQT_SIGNAL( created( const TQString & ) ),
           TQT_SLOT( reload( const TQString & ) ) );
  connect( &mDirWatch, TQT_SIGNAL( deleted( const TQString & ) ),
           TQT_SLOT( reload( const TQString & ) ) );

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
  TQFileInfo dirInfo( mURL.path() );
  return dirInfo.isDir() && dirInfo.isReadable() &&
         ( dirInfo.isWritable() || readOnly() );
}

bool ResourceLocalDir::doLoad()
{
  kdDebug(5800) << "ResourceLocalDir::load()" << endl;

  mCalendar.close();
  TQString dirName = mURL.path();

  if ( !( KStandardDirs::exists( dirName ) || KStandardDirs::exists( dirName + "/") ) ) {
    kdDebug(5800) << "ResourceLocalDir::load(): Directory '" << dirName
                  << "' doesn't exist yet. Creating it..." << endl;
    // Create the directory. Use 0775 to allow group-writable if the umask
    // allows it (permissions will be 0775 & ~umask). This is desired e.g. for
    // group-shared directories!
    return KStandardDirs::makeDir( dirName, 0775 );
  }

  // The directory exists. Now try to open (the files in) it.
  kdDebug(5800) << "ResourceLocalDir::load(): '" << dirName << "'" << endl;
  TQFileInfo dirInfo( dirName );
  if ( !( dirInfo.isDir() && dirInfo.isReadable() &&
          ( dirInfo.isWritable() || readOnly() ) ) )
    return false;

  TQDir dir( dirName );
  TQStringList entries = dir.entryList( TQDir::Files | TQDir::Readable );

  bool success = true;
  TQStringList::ConstIterator it;
  for( it = entries.constBegin(); it != entries.constEnd(); ++it ) {
    if ( (*it).endsWith( "~" ) ) // is backup file, ignore it
      continue;

    TQString fileName = dirName + "/" + *it;
    kdDebug(5800) << " read '" << fileName << "'" << endl;
    CalendarLocal cal( mCalendar.timeZoneId() );
    if ( !doFileLoad( cal, fileName ) ) {
      success = false;
    }
  }

  return success;
}

bool ResourceLocalDir::doFileLoad( CalendarLocal &cal, const TQString &fileName )
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
  bool success = true;

  list = addedIncidences();
  list += changedIncidences();

  for ( Incidence::List::iterator it = list.begin(); it != list.end(); ++it )
    if ( !doSave( *it ) )
      success = false;

  return success;
}

bool ResourceLocalDir::doSave( Incidence *incidence )
{
  if ( mDeletedIncidences.contains( incidence ) ) {
    mDeletedIncidences.remove( incidence );
    return true;
  }

  mDirWatch.stopScan();  // do prohibit the dirty() signal and a following reload()

  TQString fileName = mURL.path() + "/" + incidence->uid();
  kdDebug(5800) << "writing '" << fileName << "'" << endl;

  CalendarLocal cal( mCalendar.timeZoneId() );
  cal.addIncidence( incidence->clone() );
  const bool ret = cal.save( fileName );

  mDirWatch.startScan();

  return ret;
}

KABC::Lock *ResourceLocalDir::lock()
{
  return mLock;
}

void ResourceLocalDir::reload( const TQString &file )
{
  kdDebug(5800) << "ResourceLocalDir::reload()" << endl;

  if ( !isOpen() )
    return;

  kdDebug(5800) << "  File: '" << file << "'" << endl;

  mCalendar.close();
  load();

  emit resourceChanged( this );
}


bool ResourceLocalDir::deleteEvent(Event *event)
{
  kdDebug(5800) << "ResourceLocalDir::deleteEvent" << endl;
  if ( deleteIncidenceFile(event) ) {
    if ( mCalendar.deleteEvent( event ) ) {
      mDeletedIncidences.append( event );
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}


bool ResourceLocalDir::deleteTodo(Todo *todo)
{
  if ( deleteIncidenceFile(todo) ) {
    if ( mCalendar.deleteTodo( todo ) ) {
      mDeletedIncidences.append( todo );
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}


bool ResourceLocalDir::deleteJournal( Journal *journal )
{
  if ( deleteIncidenceFile( journal ) ) {
    if ( mCalendar.deleteJournal( journal ) ) {
      mDeletedIncidences.append( journal );
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}


void ResourceLocalDir::dump() const
{
  ResourceCalendar::dump();
  kdDebug(5800) << "  Url: " << mURL.url() << endl;
}

bool ResourceLocalDir::deleteIncidenceFile(Incidence *incidence)
{
  TQFile file( mURL.path() + "/" + incidence->uid() );
  if ( !file.exists() )
    return true;

  mDirWatch.stopScan();
  bool removed = file.remove();
  mDirWatch.startScan();
  return removed;
}

#include "resourcelocaldir.moc"
