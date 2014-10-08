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

#include "testfreebusyganttproxymodel.h"
#include "modeltest.h"
#include "freebusymodel/freebusyitem.h"
#include "freebusymodel/freebusyitemmodel.h"
#include "../freebusyganttproxymodel.h"

#include <kdgantt2/kdganttgraphicsview.h>

#include <KCalCore/Attendee>

#include <qtest_kde.h>

QTEST_KDEMAIN( FreeBusyGanttProxyModelTest, NoGUI )
using namespace IncidenceEditorNG;

void FreeBusyGanttProxyModelTest::testModelValidity()
{
  FreeBusyItemModel * fbModel = new FreeBusyItemModel();
  FreeBusyGanttProxyModel *ganttModel = new FreeBusyGanttProxyModel();
  ganttModel->setSourceModel( fbModel );
  ModelTest * modelTest = new ModelTest( ganttModel );

  Q_UNUSED( modelTest );

  QVERIFY( ganttModel->rowCount() == 0 );

  const KDateTime dt1( QDate( 2010, 8, 24 ), QTime( 7, 0, 0 ), KDateTime::UTC );
  const KDateTime dt2( QDate( 2010, 8, 24 ), QTime( 16, 0, 0 ), KDateTime::UTC );
  KCalCore::Attendee::Ptr a1( new KCalCore::Attendee( "fred", "fred@example.com" ) );
  KCalCore::FreeBusy::Ptr fb1( new KCalCore::FreeBusy() );

  fb1->addPeriod( dt1, KCalCore::Duration( 60 * 60 ) );
  fb1->addPeriod( dt2, KCalCore::Duration( 60 * 60 ) );

  FreeBusyItem::Ptr item1( new FreeBusyItem( a1, 0 ) );
  item1->setFreeBusy( fb1 );

  const KDateTime dt3( QDate( 2010, 8, 25 ), QTime( 7, 0, 0 ), KDateTime::UTC );
  const KDateTime dt4( QDate( 2010, 8, 25 ), QTime( 16, 0, 0 ), KDateTime::UTC );
  KCalCore::Attendee::Ptr a2( new KCalCore::Attendee( "joe", "joe@example.com" ) );
  KCalCore::FreeBusy::Ptr fb2( new KCalCore::FreeBusy() );

  fb2->addPeriod( dt3, KCalCore::Duration( 60 * 60 ) );
  fb2->addPeriod( dt4, KCalCore::Duration( 60 * 60 ) );

  FreeBusyItem::Ptr item2( new FreeBusyItem( a2, 0 ) );
  item2->setFreeBusy( fb2 );

  fbModel->addItem( item1 );
  fbModel->addItem( item2 );

  QCOMPARE( ganttModel->rowCount(), 2 );

  QModelIndex parent0 = ganttModel->index( 0, 0 );
  QModelIndex parent1 = ganttModel->index( 1, 0 );
  QModelIndex parent2 = ganttModel->index( 2, 0 );
  QVERIFY( parent0.isValid() );
  QVERIFY( parent1.isValid() );
  QVERIFY( parent2.isValid() == false );

  QModelIndex source_parent0 = fbModel->index( 0, 0 );
  QCOMPARE( parent0.data(), source_parent0.data() );
  QCOMPARE( parent0.data( KDGantt::ItemTypeRole ).toInt(), (int) KDGantt::TypeMulti );

  QModelIndex source_parent1 = fbModel->index( 1, 0 );
  QCOMPARE( parent1.data(), source_parent1.data() );
  QCOMPARE( parent1.data( KDGantt::ItemTypeRole ).toInt(), (int) KDGantt::TypeMulti );

  QModelIndex child0_0 = ganttModel->index( 0, 0, parent0 );
  QModelIndex child0_1 = ganttModel->index( 1, 0, parent0 );
  QVERIFY( child0_0.isValid() );
  QVERIFY( child0_1.isValid() );

  QCOMPARE( child0_0.data( KDGantt::ItemTypeRole ).toInt(), (int) KDGantt::TypeTask );
  QCOMPARE( child0_0.data( KDGantt::StartTimeRole ).value<QDateTime>(), dt1.dateTime() );
  QCOMPARE( child0_1.data( KDGantt::ItemTypeRole ).toInt(), (int) KDGantt::TypeTask );
  QCOMPARE( child0_1.data( KDGantt::StartTimeRole ).value<QDateTime>(), dt2.dateTime() );

  QModelIndex child1_0 = ganttModel->index( 0, 0, parent1 );
  QModelIndex child1_1 = ganttModel->index( 1, 0, parent1 );
  QVERIFY( child1_0.isValid() );
  QVERIFY( child1_1.isValid() );

  QCOMPARE( child1_0.data( KDGantt::ItemTypeRole ).toInt(), (int) KDGantt::TypeTask );
  QCOMPARE( child1_0.data( KDGantt::StartTimeRole ).value<QDateTime>(), dt3.dateTime() );
  QCOMPARE( child1_1.data( KDGantt::ItemTypeRole ).toInt(), (int) KDGantt::TypeTask );
  QCOMPARE( child1_1.data( KDGantt::StartTimeRole ).value<QDateTime>(), dt4.dateTime() );
}

