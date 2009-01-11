/*
  This file is part of libkabc.
  Copyright (c) 2008 Kevin Krammer <kevin.krammer@gmx.at>

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

#include <qtest_kde.h>

#include "contactgroup.h"
#include "contactgrouptool.h"

#include <QBuffer>

using namespace KPIM;

class ContactGroupTest : public QObject
{
  Q_OBJECT

  private Q_SLOTS:
    void testGroupRoundTrip();
    void testGroupListRoundTrip();
};

QTEST_KDEMAIN( ContactGroupTest, NoGUI )

void ContactGroupTest::testGroupRoundTrip()
{
  // TODO should also test empty group

  ContactGroup group( "TestGroup" );
  group.append( ContactGroup::Reference( "Xggdjetw" ) );
  group.append( ContactGroup::Data( "Tobias Koenig", "tokoe@kde.org" ) );
  group.append( ContactGroup::Data( "Kevin Krammer", "kevin.krammer@gmx.at" ) );

  QBuffer buffer;
  buffer.open( QIODevice::WriteOnly );

  QString errorMessage;
  bool result = ContactGroupTool::convertToXml( group, &buffer, &errorMessage );

  QVERIFY( result );
  QVERIFY( errorMessage.isEmpty() );
  buffer.close();
  QVERIFY( buffer.size() > 0 );
  buffer.open( QIODevice::ReadOnly );

  ContactGroup group2;
  result = ContactGroupTool::convertFromXml( &buffer, group2, &errorMessage );
  QVERIFY( result );
  QVERIFY( errorMessage.isEmpty() );
  QCOMPARE( group, group2 );
}

void ContactGroupTest::testGroupListRoundTrip()
{
  // TODO should also test empty list

  ContactGroup::List list;

  ContactGroup group1( "TestGroup1" );
  group1.append( ContactGroup::Reference( "Xggdjetw" ) );
  group1.append( ContactGroup::Data( "Tobias Koenig", "tokoe@kde.org" ) );
  group1.append( ContactGroup::Data( "Kevin Krammer", "kevin.krammer@gmx.at" ) );

  list.append( group1 );

  ContactGroup group2( "TestGroup2" );
  group2.append( ContactGroup::Reference( "Xggdjetw" ) );
  group2.append( ContactGroup::Data( "Tobias Koenig", "tokoe@kde.org" ) );
  group2.append( ContactGroup::Data( "Kevin Krammer", "kevin.krammer@gmx.at" ) );

  list.append( group2 );

  QBuffer buffer;
  buffer.open( QIODevice::WriteOnly );

  QString errorMessage;
  bool result = ContactGroupTool::convertToXml( list, &buffer, &errorMessage );

  QVERIFY( result );
  QVERIFY( errorMessage.isEmpty() );
  buffer.close();
  QVERIFY( buffer.size() > 0 );

  buffer.open( QIODevice::ReadOnly );

  ContactGroup::List list2;
  result = ContactGroupTool::convertFromXml( &buffer, list2, &errorMessage );
  QVERIFY( result );
  QVERIFY( errorMessage.isEmpty() );
  QVERIFY( list2.size() == 2 );
  QCOMPARE( list2[0], group1 );
  QCOMPARE( list2[1], group2 );
}

#include "contactgrouptest.moc"

// kate: space-indent on; indent-width 2; replace-tabs on;
