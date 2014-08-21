/*
  Copyright (C) 2010 Casey Link <unnamedrambler@gmail.com>
  Copyright (C) 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#include "conflictresolvertest.h"
#include "../conflictresolver.h"

#include <KCalCore/Event>
#include <KCalCore/Period>
#include <KCalCore/Duration>

#include <QDebug>

#include <QWidget>

#include <boost/shared_ptr.hpp>
#include <boost/concept_check.hpp>

#include <qtest.h>

using namespace IncidenceEditorNG;

void ConflictResolverTest::insertAttendees()
{
  foreach ( FreeBusyItem::Ptr item, attendees ) {
    resolver->insertAttendee( item );
  }
}

void ConflictResolverTest::addAttendee( const QString &email, const KCalCore::FreeBusy::Ptr &fb,
                                        KCalCore::Attendee::Role role )
{
  QString name = QString( "attendee %1" ).arg( attendees.count() );
  FreeBusyItem::Ptr item( new FreeBusyItem( KCalCore::Attendee::Ptr(
                                              new KCalCore::Attendee( name, email, false,
                                                                      KCalCore::Attendee::Accepted,
                                                                      role ) ), 0 ) );
  item->setFreeBusy( KCalCore::FreeBusy::Ptr( new KCalCore::FreeBusy( *fb.data() ) ) );
  attendees << item;
}

void ConflictResolverTest::initTestCase()
{
  parent = new QWidget;
  init();
}

void ConflictResolverTest::init()
{
  base = KDateTime::currentLocalDateTime().addDays( 1 );
  end = base.addSecs( 10 * 60 * 60 );
  resolver = new ConflictResolver( parent, parent );
}

void ConflictResolverTest::cleanup()
{
  delete resolver;
  resolver = 0;
  attendees.clear();
}

void ConflictResolverTest::simpleTest()
{
  KCalCore::Period meeting( end.addSecs( -3 * 60 * 60 ), KCalCore::Duration( 2 * 60 * 60 ) );
  addAttendee( "albert@einstein.net",
               KCalCore::FreeBusy::Ptr( new KCalCore::FreeBusy( KCalCore::Period::List()
                                                                << meeting ) ) );

  insertAttendees();

  static const int resolution = 15 * 60;
  resolver->setResolution( resolution );
  resolver->setEarliestDateTime( base );
  resolver->setLatestDateTime( end );
  resolver->findAllFreeSlots();

  QVERIFY( resolver->availableSlots().size() == 2 );

  KCalCore::Period first = resolver->availableSlots().at( 0 );
  QCOMPARE( first.start(), base );
  QCOMPARE( first.end(), meeting.start() );

  KCalCore::Period second = resolver->availableSlots().at( 1 );
  QEXPECT_FAIL("", "Got broken in revision f17b9a8c975588ad7cf4ce8b94ab8e32ac193ed8", Continue);
  QCOMPARE( second.start(), meeting.end().addSecs( resolution ) ); //add 15 minutes because the
                                                                   //free block doesn't start until
                                                                   //the next timeslot
  QCOMPARE( second.end(), end );

}

void ConflictResolverTest::stillPrettySimpleTest()
{
  KCalCore::Period meeting1( base, KCalCore::Duration( 2 * 60 * 60 ) );
  KCalCore::Period meeting2( base.addSecs( 60 * 60 ), KCalCore::Duration( 2 * 60 * 60 ) );
  KCalCore::Period meeting3( end.addSecs( -3 * 60 * 60 ), KCalCore::Duration( 2 * 60 * 60 ) );
  addAttendee( "john.f@kennedy.com",
               KCalCore::FreeBusy::Ptr( new KCalCore::FreeBusy( KCalCore::Period::List()
                                                                << meeting1 << meeting3 ) ) );
  addAttendee( "elvis@rock.com",
               KCalCore::FreeBusy::Ptr( new KCalCore::FreeBusy( KCalCore::Period::List()
                                                                << meeting2 << meeting3 ) ) );
  addAttendee( "albert@einstein.net",
               KCalCore::FreeBusy::Ptr( new KCalCore::FreeBusy( KCalCore::Period::List()
                                                                << meeting3 ) ) );

  insertAttendees();

  static const int resolution = 15 * 60;
  resolver->setResolution( resolution );
  resolver->setEarliestDateTime( base );
  resolver->setLatestDateTime( end );
  resolver->findAllFreeSlots();

  QVERIFY( resolver->availableSlots().size() == 2 );

  KCalCore::Period first = resolver->availableSlots().at( 0 );
  QEXPECT_FAIL("", "Got broken in revision f17b9a8c975588ad7cf4ce8b94ab8e32ac193ed8", Continue);
  QCOMPARE( first.start(), meeting2.end().addSecs( resolution ) );
  QCOMPARE( first.end(), meeting3.start() );

  KCalCore::Period second = resolver->availableSlots().at( 1 );
  QEXPECT_FAIL("", "Got broken in revision f17b9a8c975588ad7cf4ce8b94ab8e32ac193ed8", Continue);
  QCOMPARE( second.start(), meeting3.end().addSecs( resolution ) ); //add 15 minutes because the
                                                                    //free block doesn't start until
                                                                    //the next timeslot
  QCOMPARE( second.end(), end );
}

#define _time( h, m ) KDateTime( base.date(), QTime( h, m ) )

void ConflictResolverTest::akademy2010()
{
  // based off akademy 2010 schedule

  // first event was at 9:30, so lets align our start time there
  base.setTime( QTime( 9, 30 ) );
  end = base.addSecs( 8 * 60 * 60 );
  KCalCore::Period opening( _time( 9, 30 ), _time( 9, 45 ) );
  KCalCore::Period keynote( _time( 9, 45 ), _time( 10, 30 ) );

  KCalCore::Period sevenPrinciples( _time( 10, 30 ), _time( 11, 15 ) );
  KCalCore::Period commAsService( _time( 10, 30 ), _time( 11, 15 ) );

  KCalCore::Period kdeForums( _time( 11, 15 ), _time( 11, 45 ) );
  KCalCore::Period oviStore( _time( 11, 15 ), _time( 11, 45 ) );

  // 10 min break

  KCalCore::Period highlights( _time( 12, 0 ), _time( 12, 45 ) );
  KCalCore::Period styles( _time( 12, 0 ), _time( 12, 45 ) );

  KCalCore::Period wikimedia( _time( 12, 45 ), _time( 13, 15 ) );
  KCalCore::Period avalanche( _time( 12, 45 ), _time( 13, 15 ) );

  KCalCore::Period pimp( _time( 13, 15 ), _time( 13, 45 ) );
  KCalCore::Period direction( _time( 13, 15 ), _time( 13, 45 ) );

  // lunch 1 hr 25 min lunch

  KCalCore::Period blurr( _time( 15, 15 ), _time( 16, 00 ) );
  KCalCore::Period plasma( _time( 15, 15 ), _time( 16, 00 ) );

//  for ( int i = 1; i < 80; ++i ) {
    // adds 80 people (adds the same 8 peopl 10 times)
  addAttendee( "akademyattendee1@email.com",
               KCalCore::FreeBusy::Ptr( new KCalCore::FreeBusy( KCalCore::Period::List()
                                                                << opening << keynote
                                                                << oviStore << wikimedia
                                                                << direction ) ) );
  addAttendee( "akademyattendee2@email.com",
               KCalCore::FreeBusy::Ptr( new KCalCore::FreeBusy( KCalCore::Period::List()
                                                                << opening << keynote
                                                                << commAsService << highlights
                                                                << pimp ) ) );
  addAttendee( "akademyattendee3@email.com",
               KCalCore::FreeBusy::Ptr( new KCalCore::FreeBusy( KCalCore::Period::List()
                                                                << opening << kdeForums
                                                                << styles << pimp  << plasma ) ) );
  addAttendee( "akademyattendee4@email.com",
               KCalCore::FreeBusy::Ptr( new KCalCore::FreeBusy( KCalCore::Period::List()
                                                                << opening << keynote
                                                                << oviStore << pimp << blurr ) ) );
  addAttendee( "akademyattendee5@email.com",
               KCalCore::FreeBusy::Ptr( new KCalCore::FreeBusy( KCalCore::Period::List()
                                                                << keynote << oviStore
                                                                << highlights << avalanche ) ) );
  addAttendee( "akademyattendee6@email.com",
               KCalCore::FreeBusy::Ptr( new KCalCore::FreeBusy( KCalCore::Period::List()
                                                                << opening << keynote
                                                                << commAsService
                                                                << highlights ) ) );
  addAttendee( "akademyattendee7@email.com",
               KCalCore::FreeBusy::Ptr( new KCalCore::FreeBusy( KCalCore::Period::List()
                                                                << opening << kdeForums
                                                                << styles << avalanche
                                                                << pimp << plasma ) ) );
  addAttendee( "akademyattendee8@email.com",
               KCalCore::FreeBusy::Ptr( new KCalCore::FreeBusy( KCalCore::Period::List()
                                                                << opening << keynote
                                                                << oviStore << wikimedia
                                                                << blurr ) ) );
//  }

  insertAttendees();

  const int resolution = 5 * 60;
  resolver->setResolution( resolution );
  resolver->setEarliestDateTime( base );
  resolver->setLatestDateTime( end );
// QBENCHMARK {
  resolver->findAllFreeSlots();
// }

  QVERIFY( resolver->availableSlots().size() == 3 );
  QEXPECT_FAIL("", "Got broken in revision f17b9a8c975588ad7cf4ce8b94ab8e32ac193ed8", Abort);
  QCOMPARE( resolver->availableSlots().at( 0 ).duration(),
            KCalCore::Duration( 10 * 60 ) );
  QCOMPARE( resolver->availableSlots().at( 1 ).duration(),
            KCalCore::Duration( 1 * 60 * 60 + 25 * 60 ) );
  QVERIFY( resolver->availableSlots().at( 2 ).start() > plasma.end() );
}

void ConflictResolverTest::testPeriodIsLargerThenTimeframe()
{
  base.setDate( QDate( 2010, 7, 29 ) );
  base.setTime( QTime( 7, 30 ) );

  end.setDate( QDate( 2010, 7, 29 ) );
  end.setTime( QTime( 8, 30 ) );

  KCalCore::Period testEvent( _time( 5, 45 ), _time( 8, 45 ) );

  addAttendee( "kdabtest1@demo.kolab.org",
               KCalCore::FreeBusy::Ptr( new KCalCore::FreeBusy( KCalCore::Period::List()
                                                                << testEvent ) ) );
  addAttendee( "kdabtest2@demo.kolab.org",
               KCalCore::FreeBusy::Ptr( new KCalCore::FreeBusy( KCalCore::Period::List() ) ) );

  insertAttendees();
  resolver->setEarliestDateTime( base );
  resolver->setLatestDateTime( end );
  resolver->findAllFreeSlots();

  QCOMPARE( resolver->availableSlots().size(), 0 );
}

void ConflictResolverTest::testPeriodBeginsBeforeTimeframeBegins()
{
  base.setDate( QDate( 2010, 7, 29 ) );
  base.setTime( QTime( 7, 30 ) );

  end.setDate( QDate( 2010, 7, 29 ) );
  end.setTime( QTime( 9, 30 ) );

  KCalCore::Period testEvent( _time( 5, 45 ), _time( 8, 45 ) );

  addAttendee( "kdabtest1@demo.kolab.org",
               KCalCore::FreeBusy::Ptr( new KCalCore::FreeBusy( KCalCore::Period::List()
                                                                << testEvent ) ) );
  addAttendee( "kdabtest2@demo.kolab.org",
               KCalCore::FreeBusy::Ptr( new KCalCore::FreeBusy( KCalCore::Period::List() ) ) );

  insertAttendees();
  resolver->setEarliestDateTime( base );
  resolver->setLatestDateTime( end );
  resolver->findAllFreeSlots();

  QCOMPARE( resolver->availableSlots().size(), 1 );
  KCalCore::Period freeslot = resolver->availableSlots().at( 0 );
  QCOMPARE( freeslot.start(), _time( 8, 45 ) );
  QCOMPARE( freeslot.end(), end );
}

void ConflictResolverTest::testPeriodEndsAfterTimeframeEnds()
{
  base.setDate( QDate( 2010, 7, 29 ) );
  base.setTime( QTime( 7, 30 ) );

  end.setDate( QDate( 2010, 7, 29 ) );
  end.setTime( QTime( 9, 30 ) );

  KCalCore::Period testEvent( _time( 8, 00 ), _time( 9, 45 ) );

  addAttendee( "kdabtest1@demo.kolab.org",
               KCalCore::FreeBusy::Ptr( new KCalCore::FreeBusy( KCalCore::Period::List()
                                                                << testEvent ) ) );
  addAttendee( "kdabtest2@demo.kolab.org",
               KCalCore::FreeBusy::Ptr( new KCalCore::FreeBusy( KCalCore::Period::List() ) ) );

  insertAttendees();
  resolver->setEarliestDateTime( base );
  resolver->setLatestDateTime( end );
  resolver->findAllFreeSlots();

  QCOMPARE( resolver->availableSlots().size(), 1 );
  KCalCore::Period freeslot = resolver->availableSlots().at( 0 );
  QCOMPARE( freeslot.duration(), KCalCore::Duration( 30 * 60 ) );
  QCOMPARE( freeslot.start(), base );
  QCOMPARE( freeslot.end(), _time( 8, 00 ) );
}

void ConflictResolverTest::testPeriodEndsAtSametimeAsTimeframe()
{
  base.setDate( QDate( 2010, 7, 29 ) );
  base.setTime( QTime( 7, 45 ) );

  end.setDate( QDate( 2010, 7, 29 ) );
  end.setTime( QTime( 8, 45 ) );

  KCalCore::Period testEvent( _time( 5, 45 ), _time( 8, 45 ) );

  addAttendee( "kdabtest1@demo.kolab.org",
               KCalCore::FreeBusy::Ptr( new KCalCore::FreeBusy( KCalCore::Period::List()
                                                                << testEvent ) ) );
  addAttendee( "kdabtest2@demo.kolab.org",
               KCalCore::FreeBusy::Ptr( new KCalCore::FreeBusy( KCalCore::Period::List() ) ) );

  insertAttendees();
  resolver->setEarliestDateTime( base );
  resolver->setLatestDateTime( end );
  resolver->findAllFreeSlots();

  QCOMPARE( resolver->availableSlots().size(), 0 );
}

QTEST_MAIN( ConflictResolverTest )

