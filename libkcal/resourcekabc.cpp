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

#include <klocale.h>
#include <kdebug.h>
#include <kurl.h>
#include <kio/job.h>
#include <kstandarddirs.h>

#include <kabc/stdaddressbook.h>

#include "vcaldrag.h"
#include "vcalformat.h"
#include "icalformat.h"
#include "exceptions.h"
#include "incidence.h"
#include "event.h"
#include "todo.h"
#include "journal.h"
#include "filestorage.h"
#include "libkcal/alarm.h"

#include <kresources/resourceconfigwidget.h>

#include "resourcekabcconfig.h"

#include "resourcekabc.h"

using namespace KCal;

extern "C"
{
  KRES::ResourceConfigWidget *config_widget( QWidget *parent ) {
    return new ResourceKABCConfig( parent, "Configure addressbook birthdays" );
  }

  KRES::Resource *resource( const KConfig *config ) {
    return new ResourceKABC( config );
  }
}

ResourceKABC::ResourceKABC( const KConfig* config )
  : ResourceCalendar( config )
{
  if ( config ) {
    readConfig( config );
  }

  init();
}

ResourceKABC::ResourceKABC( )
  : ResourceCalendar( 0 )
{
  mAlarmDays = 1;
  mAlarm = false;

  init();
}

ResourceKABC::~ResourceKABC()
{
}

void ResourceKABC::init()
{
  setType( "birthdays" );

  mOpen = false;
  setReadOnly( true );

  mAddressbook = KABC::StdAddressBook::self();
  connect( mAddressbook, SIGNAL(addressBookChanged(AddressBook*)), SLOT( reload() ) );
}

void ResourceKABC::readConfig( const KConfig *config )
{
  mAlarmDays = config->readNumEntry( "AlarmDays", 1 );
  mAlarm = config->readBoolEntry( "Alarm", false );
}

void ResourceKABC::writeConfig( KConfig *config )
{
  ResourceCalendar::writeConfig( config );
  config->writeEntry( "AlarmDays", mAlarmDays );
  config->writeEntry( "Alarm", mAlarm );
  load();
}


bool ResourceKABC::doOpen()
{
  kdDebug(5800) << "ResourceKABC::doOpen()" << endl;

  mOpen = true;

  return true;
}

bool ResourceKABC::load()
{
  kdDebug() << "ResourceKABC::load()" << endl;

  if ( !mOpen ) return true;

  mCalendar.close();

  // import from kabc
  QDateTime birthdate;
  QString summary;

  KABC::AddressBook::Iterator it;
  for ( it = mAddressbook->begin(); it != mAddressbook->end(); ++it ) {
    if ( (*it).birthday().date().isValid() ) {
      kdDebug() << "found a birthday " << (*it).birthday().toString() << endl;

      QString name = (*it).nickName();
      if (name.isEmpty()) name = (*it).realName();
      summary = i18n("%1's birthday").arg( name );
      birthdate = (*it).birthday();

      Event *ev = new Event();

      ev->setDtStart(birthdate);
      ev->setDtEnd(birthdate);
      ev->setHasEndDate(true);
      ev->setFloats(true);

      ev->setSummary(summary);

      // Set the recurrence
      Recurrence *vRecurrence = ev->recurrence();
      vRecurrence->setRecurStart(birthdate);
      vRecurrence->setYearly(Recurrence::rYearlyMonth,1,-1);
      vRecurrence->addYearlyNum(birthdate.date().month());

      ev->clearAlarms();

      if ( mAlarm ) {
        // Set the alarm
        Alarm* vAlarm = ev->newAlarm();
        vAlarm->setText(summary);
        vAlarm->setTime(birthdate);
        // 24 hours before
        vAlarm->setStartOffset( -1440 * mAlarmDays );
        vAlarm->setEnabled(true);
      }

      // insert category
      ev->setCategories(i18n("Birthday"));

      mCalendar.addEvent(ev);
      kdDebug() << "imported " << birthdate.toString() << endl;
    }
  }

  emit resourceChanged( this );

  return true;
}

void ResourceKABC::setAlarm( bool a )
{
  mAlarm = a;
}

bool ResourceKABC::alarm()
{
  return mAlarm;
}

void ResourceKABC::setAlarmDays( int ad )
{
  mAlarmDays = ad;
}

int ResourceKABC::alarmDays()
{
  return mAlarmDays;
}

bool ResourceKABC::save()
{
  // is always read only!
  return true;
}

bool ResourceKABC::isSaving()
{
  return false;
}

void ResourceKABC::doClose()
{
  if ( !mOpen ) return;

  mCalendar.close();
  mOpen = false;
}


bool ResourceKABC::addEvent(Event *event)
{
  // read only!
  return false;
  //mCalendar.addEvent( event );
}

void ResourceKABC::deleteEvent(Event *event)
{
  // read only!
  //kdDebug(5800) << "ResourceKABC::deleteEvent" << endl;
  //mCalendar.deleteEvent( event );
}


Event *ResourceKABC::event( const QString &uid )
{
  return mCalendar.event( uid );
}

QPtrList<Event> ResourceKABC::rawEventsForDate(const QDate &qd, bool sorted)
{
  return mCalendar.rawEventsForDate( qd, sorted );
}


QPtrList<Event> ResourceKABC::rawEvents( const QDate &start, const QDate &end,
                                          bool inclusive )
{
  return mCalendar.rawEvents( start, end, inclusive );
}

QPtrList<Event> ResourceKABC::rawEventsForDate(const QDateTime &qdt)
{
  return mCalendar.rawEventsForDate( qdt.date() );
}

QPtrList<Event> ResourceKABC::rawEvents()
{
  return mCalendar.rawEvents();
}

bool ResourceKABC::addTodo(Todo *todo)
{
  //read  only!
  return false;
  //mCalendar.addTodo( todo );
}

void ResourceKABC::deleteTodo(Todo *todo)
{
  // read only!
  //mCalendar.deleteTodo( todo );
}


QPtrList<Todo> ResourceKABC::rawTodos()
{
  return mCalendar.rawTodos();
}

Todo *ResourceKABC::todo( const QString &uid )
{
  return mCalendar.todo( uid );
}

QPtrList<Todo> ResourceKABC::todos( const QDate &date )
{
  return mCalendar.todos( date );
}


bool ResourceKABC::addJournal(Journal *journal)
{
  // read only!
  //kdDebug(5800) << "Adding Journal on " << journal->dtStart().toString() << endl;
  //return mCalendar.addJournal( journal );
  return false;
}

Journal *ResourceKABC::journal(const QDate &date)
{
//  kdDebug(5800) << "ResourceKABC::journal() " << date.toString() << endl;

  return mCalendar.journal( date );
}

Journal *ResourceKABC::journal(const QString &uid)
{
  return mCalendar.journal( uid );
}

QPtrList<Journal> ResourceKABC::journals()
{
  return mCalendar.journals();
}


Alarm::List ResourceKABC::alarmsTo( const QDateTime &to )
{
  return mCalendar.alarmsTo( to );
}

Alarm::List ResourceKABC::alarms( const QDateTime &from, const QDateTime &to )
{
//  kdDebug(5800) << "ResourceKABC::alarms(" << from.toString() << " - " << to.toString() << ")\n";

  return mCalendar.alarms( from, to );
}

void ResourceKABC::update(IncidenceBase *)
{
}

void ResourceKABC::dump() const
{
  ResourceCalendar::dump();
}

void ResourceKABC::reload()
{
  load();
}

#include "resourcekabc.moc"
