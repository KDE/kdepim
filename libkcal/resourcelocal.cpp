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

#include "resourcelocalconfig.h"

#include "resourcelocal.h"

using namespace KCal;

extern "C"
{
  KRES::ResourceConfigWidget *config_widget( QWidget *parent ) {
    return new ResourceLocalConfig( parent, "Configure File-Based Calendar" );
  }

  KRES::Resource *resource( const KConfig *config ) {
    return new ResourceLocal( config );
  }
}

ResourceLocal::ResourceLocal( const KConfig* config )
  : ResourceCalendar( config )
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
  : ResourceCalendar( 0 )
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


bool ResourceLocal::addEvent(Event *event)
{
  return mCalendar.addEvent( event );
}

// probably not really efficient, but...it works for now.
void ResourceLocal::deleteEvent(Event *event)
{
  kdDebug(5800) << "ResourceLocal::deleteEvent" << endl;

  mCalendar.deleteEvent( event );
}


Event *ResourceLocal::event( const QString &uid )
{
  return mCalendar.event( uid );
}

int ResourceLocal::numEvents(const QDate &qd)
{
  return mCalendar.numEvents( qd );
}

QPtrList<Event> ResourceLocal::rawEventsForDate(const QDate &qd, bool sorted)
{
  return mCalendar.rawEventsForDate( qd, sorted );
}


QPtrList<Event> ResourceLocal::rawEvents( const QDate &start, const QDate &end,
                                          bool inclusive )
{
  return mCalendar.rawEvents( start, end, inclusive );
}

QPtrList<Event> ResourceLocal::rawEventsForDate(const QDateTime &qdt)
{
  return mCalendar.rawEventsForDate( qdt.date() );
}

QPtrList<Event> ResourceLocal::rawEvents()
{
  return mCalendar.rawEvents();
}

bool ResourceLocal::addTodo(Todo *todo)
{
  return mCalendar.addTodo( todo );
}

void ResourceLocal::deleteTodo(Todo *todo)
{
  mCalendar.deleteTodo( todo );
}


QPtrList<Todo> ResourceLocal::rawTodos()
{
  return mCalendar.rawTodos();
}

Todo *ResourceLocal::todo( const QString &uid )
{
  return mCalendar.todo( uid );
}

QPtrList<Todo> ResourceLocal::todos( const QDate &date )
{
  return mCalendar.todos( date );
}


bool ResourceLocal::addJournal(Journal *journal)
{
  kdDebug(5800) << "Adding Journal on " << journal->dtStart().toString() << endl;

  return mCalendar.addJournal( journal );
}

Journal *ResourceLocal::journal(const QDate &date)
{
//  kdDebug(5800) << "ResourceLocal::journal() " << date.toString() << endl;

  return mCalendar.journal( date );
}

Journal *ResourceLocal::journal(const QString &uid)
{
  return mCalendar.journal( uid );
}

QPtrList<Journal> ResourceLocal::journals()
{
  return mCalendar.journals();
}


Alarm::List ResourceLocal::alarmsTo( const QDateTime &to )
{
  return mCalendar.alarmsTo( to );
}

Alarm::List ResourceLocal::alarms( const QDateTime &from, const QDateTime &to )
{
//  kdDebug(5800) << "ResourceLocal::alarms(" << from.toString() << " - " << to.toString() << ")\n";

  return mCalendar.alarms( from, to );
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
