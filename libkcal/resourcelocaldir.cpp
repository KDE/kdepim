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

#include <kresources/resourceconfigwidget.h>

#include "resourcelocaldirconfig.h"

#include "resourcelocaldir.h"

using namespace KCal;

extern "C"
{
  KRES::ResourceConfigWidget *config_widget( QWidget *parent ) {
    return new ResourceLocalDirConfig( parent, "Configure Dir-Based Calendar" );
  }

  KRES::Resource *resource( const KConfig *config ) {
    return new ResourceLocalDir( config );
  }
}

ResourceLocalDir::ResourceLocalDir( const KConfig* config )
  : ResourceCalendar( config )
{
  if ( config ) {
    readConfig( config );
  }

  init();
}

ResourceLocalDir::ResourceLocalDir( const QString& dirName ) 
  : ResourceCalendar( 0 )
{
  mURL = KURL( dirName );

  init();
}


void ResourceLocalDir::readConfig( const KConfig *config )
{
  QString url = config->readEntry( "CalendarURL" );
  mURL = KURL( url );
}

void ResourceLocalDir::writeConfig( KConfig *config )
{
  kdDebug() << "ResourceLocalDir::writeConfig()" << endl;

  ResourceCalendar::writeConfig( config );

  config->writeEntry( "CalendarURL", mURL.prettyURL() );
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


bool ResourceLocalDir::addEvent(Event *event)
{
  return mCalendar.addEvent( event );
}

void ResourceLocalDir::deleteEvent(Event *event)
{
  kdDebug(5800) << "ResourceLocalDir::deleteEvent" << endl;

  mCalendar.deleteEvent( event );
}


Event *ResourceLocalDir::event( const QString &uid )
{
  return mCalendar.event( uid );
}

QPtrList<Event> ResourceLocalDir::rawEventsForDate(const QDate &qd, bool sorted)
{
  return mCalendar.rawEventsForDate( qd, sorted );
}


QPtrList<Event> ResourceLocalDir::rawEvents( const QDate &start, const QDate &end,
                                          bool inclusive )
{
  return mCalendar.rawEvents( start, end, inclusive );
}

QPtrList<Event> ResourceLocalDir::rawEventsForDate(const QDateTime &qdt)
{
  return mCalendar.rawEventsForDate( qdt.date() );
}

QPtrList<Event> ResourceLocalDir::rawEvents()
{
  return mCalendar.rawEvents();
}

bool ResourceLocalDir::addTodo(Todo *todo)
{
  return mCalendar.addTodo( todo );
}

void ResourceLocalDir::deleteTodo(Todo *todo)
{
  mCalendar.deleteTodo( todo );
}


QPtrList<Todo> ResourceLocalDir::rawTodos()
{
  return mCalendar.rawTodos();
}

Todo *ResourceLocalDir::todo( const QString &uid )
{
  return mCalendar.todo( uid );
}

QPtrList<Todo> ResourceLocalDir::todos( const QDate &date )
{
  return mCalendar.todos( date );
}


bool ResourceLocalDir::addJournal(Journal *journal)
{
  kdDebug(5800) << "Adding Journal on " << journal->dtStart().toString() << endl;

  return mCalendar.addJournal( journal );
}

Journal *ResourceLocalDir::journal(const QDate &date)
{
//  kdDebug(5800) << "ResourceLocalDir::journal() " << date.toString() << endl;

  return mCalendar.journal( date );
}

Journal *ResourceLocalDir::journal(const QString &uid)
{
  return mCalendar.journal( uid );
}

QPtrList<Journal> ResourceLocalDir::journals()
{
  return mCalendar.journals();
}


Alarm::List ResourceLocalDir::alarmsTo( const QDateTime &to )
{
  return mCalendar.alarmsTo( to );
}

Alarm::List ResourceLocalDir::alarms( const QDateTime &from, const QDateTime &to )
{
//  kdDebug(5800) << "ResourceLocalDir::alarms(" << from.toString() << " - " << to.toString() << ")\n";

  return mCalendar.alarms( from, to );
}

void ResourceLocalDir::update(IncidenceBase *)
{
}

void ResourceLocalDir::dump() const
{
  ResourceCalendar::dump();
  kdDebug(5800) << "  Url: " << mURL.url() << endl;
}

#include "resourcelocaldir.moc"
