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
#include <kabc/locknull.h>

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

#include <kresources/configwidget.h>

#include "resourcekabcconfig.h"

#include "resourcekabc.h"

using namespace KCal;

extern "C"
{
  void *init_kcal_kabc()
  {
    return new KRES::PluginFactory<ResourceKABC,ResourceKABCConfig>();
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
  delete mLock;
}

void ResourceKABC::init()
{
  setType( "birthdays" );

  setReadOnly( true );

  mLock = new KABC::LockNull( false );

  mAddressbook = 0;
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

  if ( isOpen() )
    load();
}


bool ResourceKABC::doOpen()
{
  kdDebug(5800) << "ResourceKABC::doOpen()" << endl;

  mAddressbook = KABC::StdAddressBook::self();
  connect( mAddressbook, SIGNAL(addressBookChanged(AddressBook*)), SLOT( reload() ) );

  return true;
}

bool ResourceKABC::doLoad()
{
  kdDebug(5800) << "ResourceKABC::load()" << endl;

  mCalendar.close();

  // import from kabc
  QString summary;

  KABC::Addressee::List anniversaries;
  KABC::Addressee::List::Iterator addrIt;

  KABC::AddressBook::Iterator it;
  for ( it = mAddressbook->begin(); it != mAddressbook->end(); ++it ) {

    QDateTime birthdate = (*it).birthday().date();
    if ( birthdate.isValid() ) {
      kdDebug(5800) << "found a birthday " << birthdate.toString() << endl;

      QString name = (*it).nickName();
      if (name.isEmpty()) name = (*it).realName();
      summary = i18n("%1's birthday").arg( name );

      Event *ev = new Event();
      ev->setUid( (*it).uid()+"_KABC_Birthday");

      ev->setDtStart(birthdate);
      ev->setDtEnd(birthdate);
      ev->setHasEndDate(true);
      ev->setFloats(true);
      ev->setTransparency( Event::Transparent );

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

      ev->setReadOnly( true );
      mCalendar.addEvent(ev);
      kdDebug(5800) << "imported " << birthdate.toString() << endl;
    }

    QString anniversary_string = (*it).custom( "KADDRESSBOOK", "X-Anniversary" );
    if (anniversary_string.isEmpty() )
      continue;
    QDateTime anniversary = QDate::fromString( anniversary_string, Qt::ISODate );
    if ( !anniversary.isValid() )
      continue;

    QString name = (*it).custom( "KADDRESSBOOK", "X-SpousesName" );
    if ( name.isEmpty() )
      anniversaries.append( *it );
    else {
      bool found = false;
      for ( addrIt = anniversaries.begin(); addrIt != anniversaries.end(); ++addrIt ) {
        if ( name == (*addrIt).realName() ) {
          QDateTime spouseAnniversary = QDate::fromString( (*addrIt).custom( "KADDRESSBOOK", "X-Anniversary" ), Qt::ISODate );
          if ( anniversary == spouseAnniversary ) {
            found = true;
            break;

          }
        }
      }

      if ( !found )
        anniversaries.append( *it );
    }
  }

  for ( addrIt = anniversaries.begin(); addrIt != anniversaries.end(); ++addrIt ) {
    QDateTime anniversary = QDate::fromString( (*addrIt).custom( "KADDRESSBOOK", "X-Anniversary" ), Qt::ISODate );
    kdDebug(5800) << "found a anniversary " << anniversary.toString() << endl;

    QString name = (*addrIt).nickName();
    QString spouseName = (*addrIt).custom( "KADDRESSBOOK", "X-SpousesName" );
    if ( name.isEmpty() )
      name = (*addrIt).givenName();
    if ( !spouseName.isEmpty() ) {
      KABC::Addressee spouse;
      spouse.setNameFromString( spouseName );
      name += " & " + spouse.givenName();
    }
    summary = i18n("%1's anniversary").arg( name );

    Event *ev = new Event();
      ev->setUid( (*it).uid()+"_KABC_Anniversary");

    ev->setDtStart(anniversary);
    ev->setDtEnd(anniversary);
    ev->setHasEndDate(true);
    ev->setFloats(true);

    ev->setSummary(summary);

    // Set the recurrence
    Recurrence *vRecurrence = ev->recurrence();
    vRecurrence->setRecurStart(anniversary);
    vRecurrence->setYearly(Recurrence::rYearlyMonth,1,-1);
    vRecurrence->addYearlyNum(anniversary.date().month());

    ev->clearAlarms();

    if ( mAlarm ) {
      // Set the alarm
      Alarm* vAlarm = ev->newAlarm();
      vAlarm->setText(summary);
      vAlarm->setTime(anniversary);
      // 24 hours before
      vAlarm->setStartOffset( -1440 * mAlarmDays );
      vAlarm->setEnabled(true);
    }

    // insert category
    ev->setCategories(i18n("Anniversary"));

    ev->setReadOnly( true );
    mCalendar.addEvent(ev);
    kdDebug(5800) << "imported " << anniversary.toString() << endl;
  }

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

bool ResourceKABC::doSave()
{
  // is always read only!
  return true;
}

bool ResourceKABC::isSaving()
{
  return false;
}

KABC::Lock *ResourceKABC::lock()
{
  return mLock;
}


bool ResourceKABC::addEvent(Event*)
{
  return false;
}

void ResourceKABC::deleteEvent(Event*)
{
}


Event *ResourceKABC::event( const QString &uid )
{
  return mCalendar.event( uid );
}

Event::List ResourceKABC::rawEventsForDate(const QDate &qd, bool sorted)
{
  return mCalendar.rawEventsForDate( qd, sorted );
}


Event::List ResourceKABC::rawEvents( const QDate &start, const QDate &end,
                                          bool inclusive )
{
  return mCalendar.rawEvents( start, end, inclusive );
}

Event::List ResourceKABC::rawEventsForDate(const QDateTime &qdt)
{
  return mCalendar.rawEventsForDate( qdt.date() );
}

Event::List ResourceKABC::rawEvents()
{
  return mCalendar.rawEvents();
}

bool ResourceKABC::addTodo(Todo*)
{
  return false;
}

void ResourceKABC::deleteTodo(Todo*)
{
}


Todo::List ResourceKABC::rawTodos()
{
  return mCalendar.rawTodos();
}

Todo *ResourceKABC::todo( const QString &uid )
{
  return mCalendar.todo( uid );
}

Todo::List ResourceKABC::rawTodosForDate( const QDate &date )
{
  return mCalendar.rawTodosForDate( date );
}


bool ResourceKABC::addJournal(Journal*)
{
  return false;
}

void ResourceKABC::deleteJournal(Journal*)
{
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

Journal::List ResourceKABC::journals()
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

void ResourceKABC::dump() const
{
  ResourceCalendar::dump();
}

void ResourceKABC::reload()
{
  load();
}

void ResourceKABC::setTimeZoneId( const QString& tzid )
{
  mCalendar.setTimeZoneId( tzid );
}

#include "resourcekabc.moc"
