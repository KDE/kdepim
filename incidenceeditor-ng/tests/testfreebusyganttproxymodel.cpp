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
#include "testfreebusyganttproxymodel.moc"

#include "modeltest.h"

#include "../freebusyitem.h"
#include "../freebusyitemmodel.h"
#include "../freebusyganttproxymodel.h"

#include <kcalcore/attendee.h>

#include <qtest_kde.h>

QTEST_KDEMAIN( FreeBusyGanttProxyModelTest, NoGUI )
using namespace IncidenceEditorsNG;

void FreeBusyGanttProxyModelTest::testModelValidity()
{
    FreeBusyItemModel * fbModel = new FreeBusyItemModel();
    FreeBusyGanttProxyModel *ganttModel = new FreeBusyGanttProxyModel();
    ganttModel->setSourceModel( fbModel );
    ModelTest * modelTest = new ModelTest( ganttModel );

    QVERIFY( ganttModel->rowCount() == 0 );

    const KDateTime dt1( QDate( 2010, 7, 24 ), QTime( 7, 0, 0 ), KDateTime::UTC );
    const KDateTime dt2( QDate( 2010, 7, 24 ), QTime( 10, 0, 0 ), KDateTime::UTC );
    KCalCore::Attendee::Ptr a1( new KCalCore::Attendee( "fred", "fred@example.com" ) );
    KCalCore::FreeBusy::Ptr fb1( new KCalCore::FreeBusy() );

    fb1->addPeriod( dt1, KCalCore::Duration( 60*60 ) );
    fb1->addPeriod( dt2, KCalCore::Duration( 60*60 ) );

    FreeBusyItem::Ptr item1( new FreeBusyItem( a1, 0 ) );
    item1->setFreeBusy( fb1 );

    fbModel->addItem( item1 );

    // fails
    QCOMPARE( ganttModel->rowCount(), 2 );


}
