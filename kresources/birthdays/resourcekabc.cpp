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
#include <tqstring.h>
#include <tqptrlist.h>

#include <klocale.h>
#include <kdebug.h>
#include <kurl.h>
#include <kio/job.h>
#include <kstandarddirs.h>

#include <kabc/stdaddressbook.h>
#include <kabc/locknull.h>

#include "libkcal/vcaldrag.h"
#include "libkcal/vcalformat.h"
#include "libkcal/icalformat.h"
#include "libkcal/exceptions.h"
#include "libkcal/incidence.h"
#include "libkcal/event.h"
#include "libkcal/todo.h"
#include "libkcal/journal.h"
#include "libkcal/filestorage.h"
#include "libkcal/alarm.h"

#include <libemailfunctions/email.h>

#include <kresources/configwidget.h>

#include "resourcekabcconfig.h"

#include "resourcekabc.h"

using namespace KCal;

extern "C"
{
  void *init_kcal_kabc()
  {
	  KGlobal::locale()->insertCatalogue( "kres_birthday" );
    KGlobal::locale()->insertCatalogue( "libkcal" );
    return new KRES::PluginFactory<ResourceKABC,ResourceKABCConfig>();
  }
}

ResourceKABC::ResourceKABC( const KConfig* config )
  : ResourceCalendar( config ), mCalendar( TQString::fromLatin1( "UTC" ) ),
    mAlarmDays( 0 ), mAlarm( true ), mUseCategories( false )
{
  if ( config ) {
    readConfig( config );
  } else {
    setResourceName( i18n( "Birthdays" ) );
  }

  init();
}

ResourceKABC::ResourceKABC()
  : ResourceCalendar( 0 ), mCalendar( TQString::fromLatin1( "UTC" ) ),
    mAlarmDays( 0 ), mAlarm( true ), mUseCategories( false )
{
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
  mAlarmDays = config->readNumEntry( "AlarmDays", 0 );
  mAlarm = config->readBoolEntry( "Alarm", true );
  mCategories = config->readListEntry( "Categories" );
  mUseCategories = config->readBoolEntry( "UseCategories", false );
}

void ResourceKABC::writeConfig( KConfig *config )
{
  ResourceCalendar::writeConfig( config );
  config->writeEntry( "AlarmDays", mAlarmDays );
  config->writeEntry( "Alarm", mAlarm );
  config->writeEntry( "Categories", mCategories );
  config->writeEntry( "UseCategories", mUseCategories );
}


bool ResourceKABC::doOpen()
{
  kdDebug(5800) << "ResourceKABC::doOpen()" << endl;

  mAddressbook = KABC::StdAddressBook::self( true );
  connect( mAddressbook, TQT_SIGNAL(addressBookChanged(AddressBook*)), TQT_SLOT( reload() ) );

  return true;
}

bool ResourceKABC::doLoad()
{
  kdDebug(5800) << "ResourceKABC::load()" << endl;

  mCalendar.close();

  // import from kabc
  TQString summary;
  TQStringList::ConstIterator strIt;
  const TQStringList::ConstIterator endStrIt = mCategories.end();
  KABC::Addressee::List anniversaries;
  KABC::Addressee::List::Iterator addrIt;

  KABC::AddressBook::Iterator it;
  const KABC::AddressBook::Iterator endIt = mAddressbook->end();
  for ( it = mAddressbook->begin(); it != endIt; ++it ) {

    if ( mUseCategories ) {
      bool hasCategory = false;
      TQStringList categories = (*it).categories();
      for ( strIt = mCategories.begin(); strIt != endStrIt; ++strIt )
        if ( categories.contains( *strIt ) ) {
          hasCategory = true;
          break;
        }

      if ( !hasCategory )
        continue;
    }

    TQDate birthdate = (*it).birthday().date();
    TQString name_1, email_1, uid_1;
    if ( birthdate.isValid() ) {
      kdDebug(5800) << "found a birthday " << birthdate.toString() << endl;

      name_1 = (*it).nickName();
      email_1 = (*it).fullEmail();
      uid_1 = (*it).uid();
      if (name_1.isEmpty()) name_1 = (*it).realName();
      summary = i18n("%1's birthday").arg( name_1 );


      Event *ev = new Event();
      ev->setUid( uid_1+"_KABC_Birthday");

      ev->setDtStart(birthdate);
      ev->setDtEnd(birthdate);
      ev->setHasEndDate(true);
      ev->setFloats(true);
      ev->setTransparency( Event::Transparent );

      ev->setCustomProperty("KABC", "BIRTHDAY", "YES");
      ev->setCustomProperty("KABC", "UID-1", uid_1 );
      ev->setCustomProperty("KABC", "NAME-1", name_1 );
      ev->setCustomProperty("KABC", "EMAIL-1", email_1 );
      kdDebug(5800) << "ResourceKABC::doLoad: uid:" << uid_1 << " name: " << name_1
        << " email: " << email_1 << endl;
      ev->setSummary(summary);

      // Set the recurrence
      Recurrence *vRecurrence = ev->recurrence();
      vRecurrence->setStartDate( birthdate );
      vRecurrence->setYearly( 1 );
      if ( birthdate.month()==2 && birthdate.day()==29 ) {
        vRecurrence->addYearlyDay( 60 );
      }

      ev->clearAlarms();

      if ( mAlarm ) {
        // Set the alarm
        Alarm* vAlarm = ev->newAlarm();
        vAlarm->setText(summary);
        vAlarm->setTime(birthdate);
        // 24 hours before. duration is in seconds.
        vAlarm->setStartOffset( -86400 * mAlarmDays );
        vAlarm->setEnabled(true);
      }

      // insert category
      ev->setCategories(i18n("Birthday"));

      ev->setReadOnly( true );
      mCalendar.addEvent(ev);
      kdDebug(5800) << "imported " << birthdate.toString() << endl;
    }

    TQString anniversary_string = (*it).custom( "KADDRESSBOOK", "X-Anniversary" );
    if (anniversary_string.isEmpty() )
      continue;
    TQDateTime anniversary = TQDate::fromString( anniversary_string, Qt::ISODate );
    if ( !anniversary.isValid() )
      continue;

    TQString name = (*it).custom( "KADDRESSBOOK", "X-SpousesName" );
    if ( name.isEmpty() )
      anniversaries.append( *it );
    else {
      bool found = false;
      for ( addrIt = anniversaries.begin(); addrIt != anniversaries.end(); ++addrIt ) {
        if ( name == (*addrIt).realName() ) {
          TQDate spouseAnniversary =
            TQDate::fromString( (*addrIt).custom( "KADDRESSBOOK", "X-Anniversary" ), Qt::ISODate );
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
    TQDate anniversary = TQDate::fromString( (*addrIt).custom( "KADDRESSBOOK", "X-Anniversary" ), Qt::ISODate );
    kdDebug(5800) << "found a anniversary " << anniversary.toString() << endl;
    TQString name;
    TQString name_1 = (*addrIt).nickName();
    TQString uid_1 = (*addrIt).uid();
    TQString email_1 = (*addrIt).fullEmail();
    if ( name_1.isEmpty() )
      name_1 = (*addrIt).realName();


    TQString spouseName = (*addrIt).custom( "KADDRESSBOOK", "X-SpousesName" );
    TQString name_2,email_2,uid_2;
    if ( !spouseName.isEmpty() ) {
      TQString tname, temail;
      KPIM::getNameAndMail( spouseName, tname, temail );
      tname = KPIM::quoteNameIfNecessary( tname );
      if ( ( tname[0] == '"' ) && ( tname[tname.length() - 1] == '"' ) ) {
        tname.remove( 0, 1 );
        tname.truncate( tname.length() - 1 );
      }
      KABC::Addressee spouse;
      spouse.setNameFromString( tname );
      name_2 = spouse.nickName();
      uid_2 = spouse.uid();
      email_2 = spouse.fullEmail();
      if ( name_2.isEmpty() ) {
        name_2 = spouse.realName();
      }
      summary = i18n("insert names of both spouses",
                     "%1's & %2's anniversary").arg( name_1 ).arg( name_2 );
    } else {
      summary = i18n("only one spouse in addressbook, insert the name",
                     "%1's anniversary").arg( name_1 );
    }

    Event *ev = new Event();
    ev->setUid( uid_1+"_KABC_Anniversary" );

    ev->setDtStart(anniversary);
    ev->setDtEnd(anniversary);
    ev->setHasEndDate(true);
    ev->setFloats(true);

    ev->setSummary(summary);

    ev->setCustomProperty( "KABC", "BIRTHDAY", "YES" );

    ev->setCustomProperty( "KABC", "UID-1", uid_1 );
    ev->setCustomProperty( "KABC", "NAME-1", name_1 );
    ev->setCustomProperty( "KABC", "EMAIL-1", email_1 );
    ev->setCustomProperty( "KABC", "ANNIVERSARY", "YES" );
    if ( !spouseName.isEmpty() ) {
      ev->setCustomProperty("KABC", "UID-2", uid_2 );
      ev->setCustomProperty("KABC", "NAME-2", spouseName );
      ev->setCustomProperty("KABC", "EMAIL-2", email_2 );
    }
    // Set the recurrence
    Recurrence *vRecurrence = ev->recurrence();
    vRecurrence->setStartDate( anniversary );
    vRecurrence->setYearly( 1 );
    if ( anniversary.month()==2 && anniversary.day()==29 ) {
      vRecurrence->addYearlyDay( 60 );
    }

    ev->clearAlarms();

    if ( mAlarm ) {
      // Set the alarm
      Alarm* vAlarm = ev->newAlarm();
      vAlarm->setText(summary);
      vAlarm->setTime(anniversary);
      // 24 hours before. duration is in seconds.
      vAlarm->setStartOffset( -86400 * mAlarmDays );
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

void ResourceKABC::setCategories( const TQStringList &categories )
{
  mCategories = categories;
}

TQStringList ResourceKABC::categories() const
{
  return mCategories;
}

void ResourceKABC::setUseCategories( bool useCategories )
{
  mUseCategories = useCategories;
}

bool ResourceKABC::useCategories() const
{
  return mUseCategories;
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


bool ResourceKABC::addEvent( Event * )
{
  return false;
}

bool ResourceKABC::addEvent( Event *, const TQString & )
{
  return false;
}

bool ResourceKABC::deleteEvent( Event * )
{
  return false;
}


Event *ResourceKABC::event( const TQString &uid )
{
  return mCalendar.event( uid );
}

Event::List ResourceKABC::rawEventsForDate( const TQDate &date,
                                             EventSortField sortField,
                                             SortDirection sortDirection )
{
  return mCalendar.rawEventsForDate( date, sortField, sortDirection );
}

Event::List ResourceKABC::rawEvents( const TQDate &start, const TQDate &end,
                                          bool inclusive )
{
  return mCalendar.rawEvents( start, end, inclusive );
}

Event::List ResourceKABC::rawEventsForDate(const TQDateTime &qdt)
{
  return mCalendar.rawEventsForDate( qdt.date() );
}

Event::List ResourceKABC::rawEvents( EventSortField sortField, SortDirection sortDirection )
{
  return mCalendar.rawEvents( sortField, sortDirection );
}

bool ResourceKABC::addTodo( Todo * )
{
  return false;
}

bool ResourceKABC::addTodo( Todo *, const TQString & )
{
  return false;
}

bool ResourceKABC::deleteTodo( Todo * )
{
  return false;
}


Todo::List ResourceKABC::rawTodos( TodoSortField sortField, SortDirection sortDirection )
{
  return mCalendar.rawTodos( sortField, sortDirection );
}

Todo *ResourceKABC::todo( const TQString &uid )
{
  return mCalendar.todo( uid );
}

Todo::List ResourceKABC::rawTodosForDate( const TQDate &date )
{
  return mCalendar.rawTodosForDate( date );
}


bool ResourceKABC::addJournal( Journal * )
{
  return false;
}

bool ResourceKABC::addJournal( Journal *, const TQString & )
{
  return false;
}

bool ResourceKABC::deleteJournal( Journal * )
{
  return false;
}

Journal *ResourceKABC::journal(const TQString &uid)
{
  return mCalendar.journal( uid );
}

Journal::List ResourceKABC::rawJournals( JournalSortField sortField, SortDirection sortDirection )
{
  return mCalendar.rawJournals( sortField, sortDirection );
}

Journal::List ResourceKABC::rawJournalsForDate( const TQDate &date )
{
  return mCalendar.rawJournalsForDate( date );
}

Alarm::List ResourceKABC::alarmsTo( const TQDateTime &to )
{
  return mCalendar.alarmsTo( to );
}

Alarm::List ResourceKABC::alarms( const TQDateTime &from, const TQDateTime &to )
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
  emit resourceChanged( this );
}

void ResourceKABC::setTimeZoneId( const TQString& tzid )
{
  mCalendar.setTimeZoneId( tzid );
}

#include "resourcekabc.moc"
