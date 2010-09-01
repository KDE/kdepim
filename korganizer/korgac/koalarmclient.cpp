/*
    KOrganizer Alarm Daemon Client.

    This file is part of KOrganizer.

    Copyright (c) 2002,2003 Cornelius Schumacher

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "koalarmclient.h"

#include "alarmdockwindow.h"
#include "alarmdialog.h"

#include <libkcal/calendarresources.h>

#include <kstandarddirs.h>
#include <kdebug.h>
#include <klocale.h>
#include <kapplication.h>
#include <kwin.h>

#include <tqpushbutton.h>

KOAlarmClient::KOAlarmClient( TQObject *parent, const char *name )
  : DCOPObject( "ac" ), TQObject( parent, name ), mDialog( 0 )
{
  kdDebug(5890) << "KOAlarmClient::KOAlarmClient()" << endl;

  mDocker = new AlarmDockWindow;
  mDocker->show();
  connect( this, TQT_SIGNAL( reminderCount( int ) ), mDocker, TQT_SLOT( slotUpdate( int ) ) );
  connect( mDocker, TQT_SIGNAL( quitSignal() ), TQT_SLOT( slotQuit() ) );

  KConfig c( locate( "config", "korganizerrc" ) );
  c.setGroup( "Time & Date" );
  TQString tz = c.readEntry( "TimeZoneId" );
  kdDebug(5890) << "TimeZone: " << tz << endl;

  mCalendar = new CalendarResources( tz );
  mCalendar->readConfig();
  mCalendar->load();

  connect( &mCheckTimer, TQT_SIGNAL( timeout() ), TQT_SLOT( checkAlarms() ) );

  KConfig *config = kapp->config();
  config->setGroup( "Alarms" );
  int interval = config->readNumEntry( "Interval", 60 );
  kdDebug(5890) << "KOAlarmClient check interval: " << interval << " seconds."
                << endl;
  mLastChecked = config->readDateTimeEntry( "CalendarsLastChecked" );

  // load reminders that were active when quitting
  config->setGroup( "General" );
  int numReminders = config->readNumEntry( "Reminders", 0 );
  for ( int i = 1; i <= numReminders; ++i ) {
    TQString group( TQString( "Incidence-%1" ).arg( i ) );
    config->setGroup( group );
    TQString uid = config->readEntry( "UID" );
    TQDateTime dt = config->readDateTimeEntry( "RemindAt" );
    if ( !uid.isEmpty() ) {
      Incidence *i = mCalendar->incidence( uid );
      if ( i && !i->alarms().isEmpty() ) {
        createReminder( mCalendar, i, dt, TQString() );
      }
    }
    config->deleteGroup( group );
  }
  config->setGroup( "General" );
  if (numReminders) {
     config->writeEntry( "Reminders", 0 );
     config->sync();
  }

  checkAlarms();
  mCheckTimer.start( 1000 * interval );  // interval in seconds
}

KOAlarmClient::~KOAlarmClient()
{
  delete mCalendar;
  delete mDocker;
  delete mDialog;
}

void KOAlarmClient::checkAlarms()
{
  KConfig *cfg = kapp->config();

  cfg->setGroup( "General" );
  if ( !cfg->readBoolEntry( "Enabled", true ) ) return;

  TQDateTime from = mLastChecked.addSecs( 1 );
  mLastChecked = TQDateTime::currentDateTime();

  kdDebug(5891) << "Check: " << from.toString() << " - " << mLastChecked.toString() << endl;

  TQValueList<Alarm *> alarms = mCalendar->alarms( from, mLastChecked );

  TQValueList<Alarm *>::ConstIterator it;
  for( it = alarms.begin(); it != alarms.end(); ++it ) {
    kdDebug(5891) << "REMINDER: " << (*it)->parent()->summary() << endl;
    Incidence *incidence = mCalendar->incidence( (*it)->parent()->uid() );
    createReminder( mCalendar, incidence, from, (*it)->text() );
  }
}

void KOAlarmClient::createReminder( KCal::CalendarResources *calendar,
                                    KCal::Incidence *incidence,
                                    const TQDateTime &dt,
                                    const TQString &displayText )
{
  if ( !incidence )
    return;

  if ( !mDialog ) {
    mDialog = new AlarmDialog( calendar );
    connect( mDialog, TQT_SIGNAL(reminderCount(int)), mDocker, TQT_SLOT(slotUpdate(int)) );
    connect( mDocker, TQT_SIGNAL(suspendAllSignal()), mDialog, TQT_SLOT(suspendAll()) );
    connect( mDocker, TQT_SIGNAL(dismissAllSignal()), mDialog, TQT_SLOT(dismissAll()) );
    connect( this, TQT_SIGNAL( saveAllSignal() ), mDialog, TQT_SLOT( slotSave() ) );
  }

  mDialog->addIncidence( incidence, dt, displayText );
  mDialog->wakeUp();
  saveLastCheckTime();
}

void KOAlarmClient::slotQuit()
{
  emit saveAllSignal();
  saveLastCheckTime();
  quit();
}

void KOAlarmClient::saveLastCheckTime()
{
  KConfigGroup cg( KGlobal::config(), "Alarms");
  cg.writeEntry( "CalendarsLastChecked", mLastChecked );
  KGlobal::config()->sync();
}

void KOAlarmClient::quit()
{
  kdDebug(5890) << "KOAlarmClient::quit()" << endl;
  kapp->quit();
}

bool KOAlarmClient::commitData( QSessionManager& )
{
  emit saveAllSignal();
  saveLastCheckTime();
  return true;
}

void KOAlarmClient::forceAlarmCheck()
{
  checkAlarms();
  saveLastCheckTime();
}

void KOAlarmClient::dumpDebug()
{
  KConfig *cfg = kapp->config();

  cfg->setGroup( "Alarms" );
  TQDateTime lastChecked = cfg->readDateTimeEntry( "CalendarsLastChecked" );

  kdDebug(5890) << "Last Check: " << lastChecked << endl;
}

TQStringList KOAlarmClient::dumpAlarms()
{
  TQDateTime start = TQDateTime( TQDateTime::currentDateTime().date(),
                               TQTime( 0, 0 ) );
  TQDateTime end = start.addDays( 1 ).addSecs( -1 );

  TQStringList lst;
  // Don't translate, this is for debugging purposes.
  lst << TQString("AlarmDeamon::dumpAlarms() from ") + start.toString()+ " to " +
         end.toString();

  TQValueList<Alarm*> alarms = mCalendar->alarms( start, end );
  TQValueList<Alarm*>::ConstIterator it;
  for( it = alarms.begin(); it != alarms.end(); ++it ) {
    Alarm *a = *it;
    lst << TQString("  ") + a->parent()->summary() + " ("
              + a->time().toString() + ")";
  }

  return lst;
}

void KOAlarmClient::debugShowDialog()
{
//   showAlarmDialog();
}

#include "koalarmclient.moc"
