/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "migrateapplicationfilestest.h"
#include "../migrateapplicationfiles.h"
#include <qtest.h>
using namespace PimCommon;
MigrateApplicationFilesTest::MigrateApplicationFilesTest(QObject *parent)
    : QObject(parent)
{

}

MigrateApplicationFilesTest::~MigrateApplicationFilesTest()
{

}

void MigrateApplicationFilesTest::shouldHaveDefaultValue()
{
    MigrateApplicationFiles migrate;
    QVERIFY(!migrate.start());
    QVERIFY(migrate.configFileName().isEmpty());
}

void MigrateApplicationFilesTest::shouldVerifyIfCheckIsNecessary()
{
    MigrateApplicationFiles migrate;
    //Invalid before config file is not set.
    QVERIFY(!migrate.checkIfNecessary());
}

QTEST_MAIN(MigrateApplicationFilesTest)
