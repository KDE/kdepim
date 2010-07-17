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

#include "conflictresolver.h"
#include "../groupwareintegration.h"

#include <KUrl>
#include <KCal/Event>
#include <KCal/Period>
#include <KCal/Duration>
#include <qtest_kde.h>
#include <KDebug>

#include <QWidget>
#include <QVector>

#include <boost/shared_ptr.hpp>
#include <boost/concept_check.hpp>

using namespace IncidenceEditorsNG;
using namespace KCal;

void ConflictResolverTest::insertAttendees()
{
    foreach( FreeBusyItem *item, attendees ) {
        resolver->insertAttendee( item );
    }
}


void ConflictResolverTest::addAttendee( const QString& email, const FreeBusy &fb, Attendee::Role role )
{
    QString name = QString( "attendee %1" ).arg( attendees.count() );
    FreeBusyItem *item = new FreeBusyItem( Attendee( name, email, false, Attendee::Accepted, role ), 0 );
    item->setFreeBusy( new FreeBusy( fb ) );
    attendees << item;
}

void ConflictResolverTest::initTestCase()
{
    if ( !GroupwareIntegration::isActive() ) {
        GroupwareIntegration::activate();
    }
    parent = new QWidget;
    base = KDateTime::currentLocalDateTime().addDays( 1 );
    end = base.addSecs( 10 * 60 * 60 );
    init();
}

void ConflictResolverTest::init()
{
    resolver = new ConflictResolver( parent, parent );
}

void ConflictResolverTest::cleanup()
{
    delete resolver;
    resolver = 0;
}

void ConflictResolverTest::simpleTest()
{
  Period meeting( end.addSecs(-3*60*60), Duration( 2*60*60 ) );
  addAttendee("albert@einstein.net", FreeBusy( Period::List() << meeting ) );

  insertAttendees();

  static const int resolution = 15*60;
  resolver->setResolution(resolution);
  resolver->setEarliestDateTime( base );
  resolver->setLatestDateTime( end );
  resolver->findAllFreeSlots();

  QVERIFY( resolver->availableSlots().size() == 2 );



  Period first = resolver->availableSlots().at(0);
  QCOMPARE( first.start() , base );
  QCOMPARE( first.end() , meeting.start() );

  Period second = resolver->availableSlots().at(1);
  QCOMPARE( second.start(), meeting.end().addSecs( resolution ) ); //add 15 minutes because the free block doesn't start until the next timeslot
  QCOMPARE( second.end(), end );

}

void ConflictResolverTest::stillPrettySimpleTest()
{
  Period meeting1( base, Duration( 2*60*60 ) );
  Period meeting2( base.addSecs(60*60), Duration( 2*60*60 ) );
  Period meeting3( end.addSecs(-3*60*60), Duration( 2*60*60 ) );
  addAttendee("john.f@kennedy.com", FreeBusy( Period::List() << meeting1 << meeting3 ) );
  addAttendee("elvis@rock.com", FreeBusy( Period::List() << meeting2 << meeting3 ) );
  addAttendee("albert@einstein.net", FreeBusy( Period::List() << meeting3 ) );

  insertAttendees();

  static const int resolution = 15*60;
  resolver->setResolution(resolution);
  resolver->setEarliestDateTime( base );
  resolver->setLatestDateTime( end );
  resolver->findAllFreeSlots();

  QVERIFY( resolver->availableSlots().size() == 2 );

  Period first = resolver->availableSlots().at(0);
  QCOMPARE( first.start() , meeting2.end().addSecs( resolution ) );
  QCOMPARE( first.end() , meeting3.start() );

  Period second = resolver->availableSlots().at(1);
  QCOMPARE( second.start(), meeting3.end().addSecs( resolution ) ); //add 15 minutes because the free block doesn't start until the next timeslot
  QCOMPARE( second.end(), end );
}

#define _time(h,m) \
        KDateTime(base.date(), QTime(h,m) )

void ConflictResolverTest::akademy2010()
{
    // based off akademy 2010 schedule

    // first event was at 9:30, so lets align our start time there
    base.setTime( QTime( 9, 30 ) );
    end = base.addSecs( 8 * 60 * 60 );
    Period opening( _time( 9, 30 ), _time( 9, 45 ) );
    Period keynote( _time( 9, 45 ), _time( 10, 30 ) );

    Period sevenPrinciples( _time( 10, 30 ), _time( 11, 15 ) );
    Period commAsService( _time( 10, 30 ), _time( 11, 15 ) );

    Period kdeForums( _time( 11, 15 ), _time( 11, 45 ) );
    Period oviStore( _time( 11, 15 ), _time( 11, 45 ) );

    // 10 min break

    Period highlights( _time( 12, 0 ), _time( 12, 45 ) );
    Period styles( _time( 12, 0 ), _time( 12, 45 ) );

    Period wikimedia( _time( 12, 45 ), _time( 13, 15 ) );
    Period avalanche( _time( 12, 45 ), _time( 13, 15 ) );

    Period pimp( _time( 13, 15 ), _time( 13, 45 ) );
    Period direction( _time( 13, 15 ), _time( 13, 45 ) );

    // lunch 1 hr 25 min lunch

    Period blurr( _time( 15, 15 ), _time( 16, 00 ) );
    Period plasma( _time( 15, 15 ), _time( 16, 00 ) );

    for ( int i = 1; i < 80; ++i ) {
        // adds 80 people (adds the same 8 peopl 10 times)
        addAttendee( "akademyattendee1@email.com", FreeBusy( Period::List() << opening << keynote << oviStore << wikimedia << direction ) );
        addAttendee( "akademyattendee2@email.com", FreeBusy( Period::List() << opening << keynote << commAsService << highlights << pimp ) );
        addAttendee( "akademyattendee3@email.com", FreeBusy( Period::List() << opening << kdeForums << styles << pimp  << plasma ) );
        addAttendee( "akademyattendee4@email.com", FreeBusy( Period::List() << opening << keynote << oviStore << pimp << blurr ) );
        addAttendee( "akademyattendee5@email.com", FreeBusy( Period::List() << keynote << oviStore << highlights << avalanche ) );
        addAttendee( "akademyattendee6@email.com", FreeBusy( Period::List() << opening << keynote << commAsService << highlights ) );
        addAttendee( "akademyattendee7@email.com", FreeBusy( Period::List() << opening << kdeForums << styles << avalanche << pimp << plasma ) );
        addAttendee( "akademyattendee8@email.com", FreeBusy( Period::List() << opening << keynote << oviStore << wikimedia << blurr ) );
    }

    insertAttendees();

    const int resolution = 5 * 60;
    resolver->setResolution( resolution );
    resolver->setEarliestDateTime( base );
    resolver->setLatestDateTime( end );
//   QBENCHMARK {
    resolver->findAllFreeSlots();
//   }

    QVERIFY( resolver->availableSlots().size() == 3 );
    QCOMPARE( resolver->availableSlots().at( 0 ).duration(), Duration( 10*60 ) );
    QCOMPARE( resolver->availableSlots().at( 1 ).duration(), Duration( 1*60*60 + 25*60 ) );
    QVERIFY( resolver->availableSlots().at( 2 ).start() > plasma.end() );
}

QTEST_KDEMAIN( ConflictResolverTest, GUI );

#include "conflictresolvertest.moc"
