/*
    This file is part of KOrganizer.

    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

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

#include <tqwidget.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kstandarddirs.h>

#include "alarmdialog.h"

int main(int argc,char **argv)
{
  KAboutData aboutData("testkabc",I18N_NOOP("TestKabc"),"0.1");
  KCmdLineArgs::init(argc,argv,&aboutData);

  KApplication app;

  KConfig c( locate( "config", "korganizerrc" ) );
  c.setGroup( "Time & Date" );
  TQString tz = c.readEntry( "TimeZoneId" );
  CalendarResources *mCalendar = new CalendarResources( tz );

  Event *e1 = new Event;
  e1->setSummary( "This is a summary." );
  TQDateTime now = TQDateTime::currentDateTime();
  e1->setDtStart( now );
  e1->setDtEnd( now.addDays( 1 ) );
  Alarm *a = e1->newAlarm();
//  a->setProcedureAlarm( "/usr/X11R6/bin/xeyes" );
  a->setAudioAlarm( "/data/kde/share/apps/korganizer/sounds/spinout.wav" );
  mCalendar->addEvent( e1 );

  Todo *t1 = new Todo;
  t1->setSummary( "To-do A" );
  t1->setDtDue( now );
  t1->newAlarm();
  mCalendar->addTodo( t1 );

  Event *e2 = new Event;
  e2->setSummary( "This is another summary." );
  e2->setDtStart( now.addDays( 1 ) );
  e2->setDtEnd( now.addDays( 2 ) );
  e2->newAlarm();
  mCalendar->addEvent( e2 );

  Event *e3 = new Event;
  e3->setSummary( "Meet with Fred" );
  e3->setDtStart( now.addDays( 2 ) );
  e3->setDtEnd( now.addDays( 3 ) );
  e3->newAlarm();
  mCalendar->addEvent( e3 );

  Todo *t2 = new Todo;
  t2->setSummary( "Something big is due today" );
  t2->setDtDue( now );
  t2->newAlarm();
  mCalendar->addTodo( t2 );

  Todo *t3 = new Todo;
  t3->setSummary( "Be lazy" );
  t3->setDtDue( now );
  t3->newAlarm();
  mCalendar->addTodo( t3 );

  Event *e4 = new Event;
  e4->setSummary( "Watch TV" );
  e4->setDtStart( now.addSecs( 120 ) );
  e4->setDtEnd( now.addSecs( 180 ) );
  e4->newAlarm();
  mCalendar->addEvent( e4 );

  AlarmDialog dlg( mCalendar, 0 );
  app.setMainWidget( &dlg );
  dlg.addIncidence( e2, TQDateTime::currentDateTime().addSecs( 60 ),
                    TQString() );
  dlg.addIncidence( t1, TQDateTime::currentDateTime().addSecs( 300 ),
                    TQString( "THIS IS DISPLAY TEXT" ) );
  dlg.addIncidence( e4, TQDateTime::currentDateTime().addSecs( 120 ),
                    TQString( "Fred and Barney get cloned" ) );
  dlg.addIncidence( e3, TQDateTime::currentDateTime().addSecs( 240 ),
                    TQString() );
  dlg.addIncidence( e1, TQDateTime::currentDateTime().addSecs( 180 ),
                    TQString() );
  dlg.addIncidence( t2, TQDateTime::currentDateTime().addSecs( 600 ),
                    TQString( "THIS IS DISPLAY TEXT" ) );
  dlg.addIncidence( t3, TQDateTime::currentDateTime().addSecs( 360 ),
                    TQString() );
  dlg.show();
  dlg.eventNotification();

  app.exec();
}
