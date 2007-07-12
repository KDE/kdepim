/* recordconduittest.cc			KPilot
**
** Copyright (C) 2007 by Bertjan Broeksema
** Copyright (C) 2007 by Jason "vanRijn" Kasper
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <QtTest>
#include <QtCore>

#include "qtest_kde.h"

#include "options.h"

#include "testrecordconduit.h"
#include "testrecord.h"


class RecordConduitTest : public QObject
{
	Q_OBJECT
	
private slots:
	void testSyncFields();
};

void RecordConduitTest::testSyncFields()
{
	QStringList args = QStringList() << CSL1( "--hotsync" );
	
	qDebug() << args.size();
	
	TestRecordConduit conduit( args );
	conduit.initDataProxies();
	
	QStringList fields = QStringList() << CSL1( "f1" ) << CSL1( "f2" );
	
	TestRecord *rec1 = new TestRecord( fields );
	rec1->setValue( CSL1( "f1" ), CSL1( "A test value" ) );
	rec1->setValue( CSL1( "f2" ), CSL1( "Another test value" ) );
	
	TestRecord *rec2 = new TestRecord( fields );
	rec2->setValue( CSL1( "f1" ), CSL1( "And more test value" ) );
	rec2->setValue( CSL1( "f2" ), CSL1( "Yet another one" ) );
	
	//                             ( from, to )
	QVERIFY( conduit.syncFieldsTest( rec1, rec2 ) );
	QVERIFY( rec2->value( CSL1( "f1" ) ) == CSL1( "A test value" ) );
	QVERIFY( rec2->value( CSL1( "f2" ) ) == CSL1( "Another test value" ) );
}

QTEST_KDEMAIN(RecordConduitTest, NoGUI)

#include "recordconduittest.moc"
