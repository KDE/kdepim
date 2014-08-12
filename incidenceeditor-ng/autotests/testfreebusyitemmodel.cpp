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

#include "testfreebusyitemmodel.h"
#include "modeltest.h"
#include "../freebusyitemmodel.h"
#include "../freebusyitem.h"

#include <KCalCore/Attendee>

#include <qtest.h>

QTEST_MAIN( FreeBusyItemModelTest )
using namespace IncidenceEditorNG;

void FreeBusyItemModelTest::testModelValidity()
{
  FreeBusyItemModel * model = new FreeBusyItemModel( this );
  new ModelTest( model, this );

  QVERIFY( model->rowCount() == 0 );

  const KDateTime dt1( QDate( 2010, 7, 24 ), QTime( 7, 0, 0 ), KDateTime::UTC );
  const KDateTime dt2( QDate( 2010, 7, 24 ), QTime( 10, 0, 0 ), KDateTime::UTC );
  KCalCore::Attendee::Ptr a1( new KCalCore::Attendee( "fred", "fred@example.com" ) );
  KCalCore::FreeBusy::Ptr fb1( new KCalCore::FreeBusy() );

  fb1->addPeriod( dt1, KCalCore::Duration( 60 * 60 ) );
  fb1->addPeriod( dt2, KCalCore::Duration( 60 * 60 ) );

  FreeBusyItem::Ptr item1( new FreeBusyItem( a1, 0 ) );
  item1->setFreeBusy( fb1 );

  model->addItem( item1 );
  QVERIFY( model->rowCount() == 1 );
  QVERIFY( model->containsAttendee( a1 ) );

  QModelIndex i = model->index( 0, 0 );
  QCOMPARE( a1->fullName(), model->data( i, Qt::DisplayRole ).toString() );
  QCOMPARE( a1,
            model->data( i, FreeBusyItemModel::AttendeeRole ).value<KCalCore::Attendee::Ptr>() );
  QCOMPARE( item1->freeBusy(),
            model->data( i, FreeBusyItemModel::FreeBusyRole ).value<KCalCore::FreeBusy::Ptr>() );

  QCOMPARE( model->rowCount( i ), 2 );

  model->removeRow( 0 );
  QVERIFY( model->rowCount() == 0 );

  model->addItem( item1 );
  QVERIFY( model->rowCount() == 1 );

  model->removeAttendee( a1 );
  QVERIFY( model->rowCount() == 0 );

  model->addItem( item1 );
  QVERIFY( model->rowCount() == 1 );

  model->removeItem( item1 );
  QVERIFY( model->rowCount() == 0 );

  model->addItem( item1 );
  QVERIFY( model->rowCount() == 1 );

  model->clear();
  QVERIFY( model->rowCount() == 0 );
}

void FreeBusyItemModelTest::testModelValidity2()
{
  FreeBusyItemModel * model = new FreeBusyItemModel( this );
  new ModelTest( model, this );

  const KDateTime dt1( QDate( 2010, 7, 24 ), QTime( 7, 0, 0 ), KDateTime::UTC );
  const KDateTime dt2( QDate( 2010, 7, 24 ), QTime( 10, 0, 0 ), KDateTime::UTC );
  const KDateTime dt3( QDate( 2010, 7, 24 ), QTime( 12, 0, 0 ), KDateTime::UTC );
  const KDateTime dt4( QDate( 2010, 7, 24 ), QTime( 14, 0, 0 ), KDateTime::UTC );

  KCalCore::Attendee::Ptr a1( new KCalCore::Attendee( "fred", "fred@example.com" ) );
  KCalCore::Attendee::Ptr a2( new KCalCore::Attendee( "joe", "joe@example.com" ) );
  KCalCore::Attendee::Ptr a3( new KCalCore::Attendee( "max", "max@example.com" ) );
  KCalCore::FreeBusy::Ptr fb1( new KCalCore::FreeBusy() );
  KCalCore::FreeBusy::Ptr fb2( new KCalCore::FreeBusy() );
  KCalCore::FreeBusy::Ptr fb3( new KCalCore::FreeBusy() );

  fb1->addPeriod( dt1, KCalCore::Duration( 60 * 60 ) );
  fb1->addPeriod( dt2, KCalCore::Duration( 60 * 60 ) );

  fb2->addPeriod( dt1, KCalCore::Duration( 60 * 60 ) );
  fb2->addPeriod( dt2, KCalCore::Duration( 60 * 60 ) );
  fb2->addPeriod( dt3, KCalCore::Duration( 60 * 60 ) );

  fb3->addPeriod( dt1, KCalCore::Duration( 60 * 60 ) );
  fb3->addPeriod( dt2, KCalCore::Duration( 60 * 60 ) );
  fb3->addPeriod( dt4, KCalCore::Duration( 60 * 60 * 2 ) );

  FreeBusyItem::Ptr item1( new FreeBusyItem( a1, 0 ) );
  item1->setFreeBusy( fb1 );
  FreeBusyItem::Ptr item2( new FreeBusyItem( a2, 0 ) );
  FreeBusyItem::Ptr item3( new FreeBusyItem( a3, 0 ) );

  model->addItem( item1 );
  model->addItem( item2 );
  model->addItem( item3 );

  QCOMPARE( model->rowCount(), 3 );

  QVERIFY( model->containsAttendee( a1 ) );
  QVERIFY( model->containsAttendee( a2 ) );
  QVERIFY( model->containsAttendee( a3 ) );

  QModelIndex i1 = model->index( 0, 0 );
  QCOMPARE( a1->fullName(), model->data( i1, Qt::DisplayRole ).toString() );
  QCOMPARE( a1,
            model->data( i1, FreeBusyItemModel::AttendeeRole ).value<KCalCore::Attendee::Ptr>() );
  QCOMPARE( item1->freeBusy(),
            model->data( i1, FreeBusyItemModel::FreeBusyRole ).value<KCalCore::FreeBusy::Ptr>() );

  QModelIndex i2 = model->index( 1, 0 );
  QCOMPARE( a2->fullName(), model->data( i2, Qt::DisplayRole ).toString() );
  QCOMPARE( a2,
            model->data( i2, FreeBusyItemModel::AttendeeRole ).value<KCalCore::Attendee::Ptr>() );
  QVERIFY( model->rowCount( i2 ) == 0 );
  QVERIFY( model->data( i2, FreeBusyItemModel::FreeBusyRole ).isValid() == false );

  QModelIndex i3 = model->index( 2, 0 );
  QCOMPARE( a3->fullName(),
            model->data( i3, Qt::DisplayRole ).toString() );
  QCOMPARE( a3,
            model->data( i3, FreeBusyItemModel::AttendeeRole ).value<KCalCore::Attendee::Ptr>() );
  QVERIFY( model->rowCount( i3 ) == 0 );
  QVERIFY( model->data( i3, FreeBusyItemModel::FreeBusyRole ).isValid() == false );

  model->slotInsertFreeBusy( fb2, "joe@example.com" );
  QCOMPARE( item2->freeBusy(),
            model->data( i2, FreeBusyItemModel::FreeBusyRole ).value<KCalCore::FreeBusy::Ptr>() );
  QVERIFY( model->rowCount( i2 ) == fb2->fullBusyPeriods().size() );

  QModelIndex i2_0 = model->index( 0, 0, i2 );
  QCOMPARE( fb2->fullBusyPeriods().first(),
            model->data(
              i2_0, FreeBusyItemModel::FreeBusyPeriodRole ).value<KCalCore::FreeBusyPeriod>() );
  QModelIndex i2_1 = model->index( 1, 0, i2 );
  QCOMPARE( fb2->fullBusyPeriods().at( 1 ),
            model->data(
              i2_1, FreeBusyItemModel::FreeBusyPeriodRole ).value<KCalCore::FreeBusyPeriod>() );
  QModelIndex i2_2 = model->index( 2, 0, i2 );
  QCOMPARE( fb2->fullBusyPeriods().last(),
            model->data(
              i2_2, FreeBusyItemModel::FreeBusyPeriodRole ).value<KCalCore::FreeBusyPeriod>() );

  model->slotInsertFreeBusy( fb3, "max@example.com" );
  QCOMPARE( item3->freeBusy(),
            model->data( i3, FreeBusyItemModel::FreeBusyRole ).value<KCalCore::FreeBusy::Ptr>() );
  QVERIFY( model->rowCount( i3 ) == fb3->fullBusyPeriods().size() );

  QModelIndex i3_0 = model->index( 0, 0, i3 );
  QCOMPARE( fb3->fullBusyPeriods().first(),
            model->data(
              i3_0, FreeBusyItemModel::FreeBusyPeriodRole ).value<KCalCore::FreeBusyPeriod>() );
  QModelIndex i3_1 = model->index( 1, 0, i3 );
  QCOMPARE( fb3->fullBusyPeriods().at( 1 ),
            model->data(
              i3_1, FreeBusyItemModel::FreeBusyPeriodRole ).value<KCalCore::FreeBusyPeriod>() );
  QModelIndex i3_2 = model->index( 2, 0, i3 );
  QCOMPARE( fb3->fullBusyPeriods().last(),
            model->data(
              i3_2, FreeBusyItemModel::FreeBusyPeriodRole ).value<KCalCore::FreeBusyPeriod>() );

  model->removeAttendee( a2 );

  QCOMPARE( 2, model->rowCount() );

  QVERIFY( model->containsAttendee( a1 ) == true );
  QVERIFY( model->containsAttendee( a2 ) == false );
  QVERIFY( model->containsAttendee( a3 ) == true );

  i3_0 = model->index( 0, 0, i3 );
  QCOMPARE( fb3->fullBusyPeriods().first(),
            model->data(
              i3_0, FreeBusyItemModel::FreeBusyPeriodRole ).value<KCalCore::FreeBusyPeriod>() );
  i3_1 = model->index( 1, 0, i3 );
  QCOMPARE( fb3->fullBusyPeriods().at( 1 ),
            model->data(
              i3_1, FreeBusyItemModel::FreeBusyPeriodRole ).value<KCalCore::FreeBusyPeriod>() );
  i3_2 = model->index( 2, 0, i3 );
  QCOMPARE( fb3->fullBusyPeriods().last(),
            model->data(
              i3_2, FreeBusyItemModel::FreeBusyPeriodRole ).value<KCalCore::FreeBusyPeriod>() );
}

void FreeBusyItemModelTest::testInsertFreeBusy()
{
  FreeBusyItemModel * model = new FreeBusyItemModel( this );
  new ModelTest( model, this );

  const KDateTime dt1( QDate( 2010, 7, 24 ), QTime( 7, 0, 0 ), KDateTime::UTC );
  const KDateTime dt2( QDate( 2010, 7, 24 ), QTime( 10, 0, 0 ), KDateTime::UTC );
  KCalCore::Attendee::Ptr a1( new KCalCore::Attendee( "fred", "fred@example.com" ) );
  KCalCore::FreeBusy::Ptr fb1( new KCalCore::FreeBusy() );
  fb1->addPeriod( dt1, KCalCore::Duration( 60 * 60 ) );
  fb1->addPeriod( dt2, KCalCore::Duration( 60 * 60 ) );

  const KDateTime dt3( QDate( 2010, 7, 24 ), QTime( 12, 0, 0 ), KDateTime::UTC );
  const KDateTime dt4( QDate( 2010, 7, 24 ), QTime( 14, 0, 0 ), KDateTime::UTC );
  KCalCore::FreeBusy::Ptr fb2( new KCalCore::FreeBusy() );
  fb2->addPeriod( dt1, KCalCore::Duration( 60 * 60 ) );
  fb2->addPeriod( dt2, KCalCore::Duration( 60 * 60 ) );
  fb2->addPeriod( dt3, KCalCore::Duration( 60 * 60 ) );
  fb2->addPeriod( dt4, KCalCore::Duration( 60 * 60 * 2 ) );

  FreeBusyItem::Ptr item1( new FreeBusyItem( a1, 0 ) );
  item1->setFreeBusy( fb1 );

  model->addItem( item1 );

  QModelIndex i = model->index( 0, 0 );
  QCOMPARE( model->rowCount( i ), 2 );

  model->slotInsertFreeBusy( fb2, "fred@example.com" );

  QCOMPARE( model->rowCount( i ), 4 );
}

