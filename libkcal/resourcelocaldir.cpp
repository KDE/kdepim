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

extern "C"
{
  void *init_kcal_localdir()
  {
    return new KRES::PluginFactory<ResourceLocalDir,ResourceLocalDirConfig>();
  }
}


ResourceLocalDir::ResourceLocalDir( const KConfig* config )
  : ResourceCached( config )
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
  kdDebug() << "ResourceLocalDir::writeConfig()" << endl;

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

  mDirWatch.addDir( mURL.path(), true );
  mDirWatch.startScan();
}


ResourceLocalDir::~ResourceLocalDir()
{
  close();
}

bool ResourceLocalDir::doOpen()
{
  kdDebug(5800) << "Opening resource " << resourceName() << " with URL " << mURL.prettyURL() << endl;

  mOpen = true;

  return true;
}

bool ResourceLocalDir::load()
{
  kdDebug() << "ResourceLocalDir::load()" << endl;

  if ( !mOpen ) return true;

  mCalendar.close();

  QString dirName = mURL.path();

  kdDebug() << "ResourceLocalDir::load(): '" << dirName << "'" << endl;

  QDir dir( dirName );

  QStringList entries = dir.entryList( QDir::Files | QDir::Readable );

  QStringList::ConstIterator it;
  for( it = entries.begin(); it != entries.end(); ++it ) {
    QString fileName = dirName + "/" + *it;
    kdDebug() << " read '" << fileName << "'" << endl;
    CalendarLocal cal;
    cal.load( fileName );
    QPtrList<Incidence> incidences = cal.rawIncidences();
    Incidence *i = incidences.first();
    if ( i ) mCalendar.addIncidence( i->clone() );
  }

  return true;
}

bool ResourceLocalDir::save()
{
  kdDebug() << "ResourceLocalDir::save()" << endl;

  if ( !mOpen ) return true;

  QPtrList<Incidence> incidences = mCalendar.rawIncidences();

  Incidence *i;
  for( i = incidences.first(); i; i = incidences.next() ) {
    QString fileName = mURL.path() + "/" + i->uid();
    kdDebug() << "writing '" << fileName << "'" << endl;

    CalendarLocal cal;
    cal.addIncidence( i->clone() );
    cal.save( fileName );
  }

  return true;
}

void ResourceLocalDir::reload( const QString &file )
{
  kdDebug() << "ResourceLocalDir::reload()" << endl;

  if ( !mOpen ) return;

  kdDebug() << "  File: '" << file << "'" << endl;

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


void ResourceLocalDir::update(IncidenceBase *)
{
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
