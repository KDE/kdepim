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
  : ResourceCalendar( config ), mCalendar( QString::fromLatin1( "UTC" ) ),
    mAlarmDays( 0 ), mAlarm( true ), mUseCategories( false )
{
  if ( config ) {
    readConfig( config );
  }

  init();
}

ResourceKABC::ResourceKABC()
  : ResourceCalendar( 0 ), mCalendar( QString::fromLatin1( "UTC" ) ),
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
  connect( mAddressbook, SIGNAL(addressBookChanged(AddressBook*)), SLOT( reload() ) );

  return true;
}

bool ResourceKABC::doLoad()
{
  kdDebug(5800) << "ResourceKABC::load()" << endl;

  mCalendar.close();

  // import from kabc
  QString summary;
  QStringList::ConstIterator strIt;
  const QStringList::ConstIterator endStrIt = mCategories.end();
  KABC::Addressee::List anniversaries;
  KABC::Addressee::List::Iterator addrIt;

  KABC::AddressBook::Iterator it;
  const KABC::AddressBook::Iterator endIt = mAddressbook->end();
  for ( it = mAddressbook->begin(); it != endIt; ++it ) {

    if ( mUseCategories ) {
      bool hasCategory = false;
      QStringList categories = (*it).categories();
      for ( strIt = mCategories.begin(); strIt != endStrIt; ++strIt )
        if ( categories.contains( *strIt ) ) {
          hasCategory = true;
          break;
        }

      if ( !hasCategory )
        continue;
    }

    QDate birthdate = (*it).birthday().date();
    QString name_1, email_1, uid_1;
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
    QDate anniversary = QDate::fromString( (*addrIt).custom( "KADDRESSBOOK", "X-Anniversary" ), Qt::ISODate );
    kdDebug(5800) << "found a anniversary " << anniversary.toString() << endl;
    QString name;
    QString name_1 = (*addrIt).nickName();
    QString uid_1 = (*addrIt).uid();
    QString email_1 = (*addrIt).fullEmail();
    if ( name_1.isEmpty() )
      name_1 = (*addrIt).realName();


    QString spouseName = (*addrIt).custom( "KADDRESSBOOK", "X-SpousesName" );
    QString name_2,email_2,uid_2;
    if ( !spouseName.isEmpty() ) {
      //TODO: find a KABC:Addressee of the spouse
      //      Probably easiest would be to use a QMap (as the spouse's entry was already searched above!
      KABC::Addressee spouse;
      spouse.setNameFromString( spouseName );
      uid_2 = spouse.uid();
      email_2 = spouse.fullEmail();
      name_2 = spouse.nickName();
      if ( name_2.isEmpty() )
        name_2 = spouse.givenName();
      summary = i18n("insert names of both spouses", "%1's & %2's anniversary").arg( name_1 ).arg( name_2 );
    } else {
      summary = i18n("only one spouse in addressbook, insert the name", "%1's anniversary").arg( name_1 );
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

void ResourceKABC::setCategories( const QStringList &categories )
{
  mCategories = categories;
}

QStringList ResourceKABC::categories() const
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


bool ResourceKABC::addEvent(Event*)
{
  return false;
}

bool ResourceKABC::deleteEvent(Event*)
{
  return false;
}


Event *ResourceKABC::event( const QString &uid )
{
  return mCalendar.event( uid );
}

Event::List ResourceKABC::rawEventsForDate( const QDate &date,
                                             EventSortField sortField,
                                             SortDirection sortDirection )
{
  return mCalendar.rawEventsForDate( date, sortField, sortDirection );
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

Event::List ResourceKABC::rawEvents( EventSortField sortField, SortDirection sortDirection )
{
  return mCalendar.rawEvents( sortField, sortDirection );
}

bool ResourceKABC::addTodo(Todo*)
{
  return false;
}

bool ResourceKABC::deleteTodo(Todo*)
{
  return false;
}


Todo::List ResourceKABC::rawTodos( TodoSortField sortField, SortDirection sortDirection )
{
  return mCalendar.rawTodos( sortField, sortDirection );
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

bool ResourceKABC::deleteJournal(Journal*)
{
  return false;
}

Journal *ResourceKABC::journal(const QString &uid)
{
  return mCalendar.journal( uid );
}

Journal::List ResourceKABC::rawJournals( JournalSortField sortField, SortDirection sortDirection )
{
  return mCalendar.rawJournals( sortField, sortDirection );
}

Journal::List ResourceKABC::rawJournalsForDate( const QDate &date )
{
  return mCalendar.rawJournalsForDate( date );
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
  emit resourceChanged( this );
}

void ResourceKABC::setTimeZoneId( const QString& tzid )
{
  mCalendar.setTimeZoneId( tzid );
}

#include "resourcekabc.moc"
