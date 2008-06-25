/* keyringcategorysynctest.cc			KPilot
**
** Copyright (C) 2007 by Bertjan Broeksema <b.broeksema@kdemail.net>
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

#include "qtest_kde.h"

#include "options.h"

#include "testkeyringconduit.h"

class KeyringCategorySyncTest : public QObject
{
	Q_OBJECT

private:
	TestKeyringConduit *fConduit;

private slots:
	void initTestCase();
	void testFail();
	void cleanupTestCase();
};

void KeyringCategorySyncTest::initTestCase()
{
	Pilot::setupPilotCodec( CSL1( "ISO8859-15" ) );

	// NOTE: 2 == eHHOverrides, this is important for the solveConflict() method
	QVariantList args;
	args << CSL1( "--hotsync" ) << CSL1( "--conflictResolution 2" );
	
	fConduit = new TestKeyringConduit( args );
	fConduit->initDataProxies();
}

void KeyringCategorySyncTest::testFail()
{
	fConduit->hotSync();
	//Q_ASSERT( false );
}

void KeyringCategorySyncTest::cleanupTestCase()
{
	delete fConduit;
}

QTEST_KDEMAIN( KeyringCategorySyncTest, NoGUI )

#include "keyringcategorysynctest.moc"
