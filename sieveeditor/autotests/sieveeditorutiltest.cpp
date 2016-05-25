/*
  Copyright (c) 2014-2016 Montel Laurent <montel@kde.org>

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
#include "sieveeditorutiltest.h"
#include <qtest.h>
#include "../src/sieveeditorutil.h"

SieveEditorUtilTest::SieveEditorUtilTest(QObject *parent)
    : QObject(parent)
{

}

SieveEditorUtilTest::~SieveEditorUtilTest()
{

}

void SieveEditorUtilTest::shouldHaveDefaultValue()
{
    SieveEditorUtil::SieveServerConfig config;
    QVERIFY(config.userName.isEmpty());
    QVERIFY(config.password.isEmpty());
    QVERIFY(config.serverName.isEmpty());
    QCOMPARE(config.authenticationType, MailTransport::Transport::EnumAuthenticationType::PLAIN);
    QCOMPARE(config.port, -1);
    QVERIFY(config.enabled);
}

QTEST_APPLESS_MAIN(SieveEditorUtilTest)

