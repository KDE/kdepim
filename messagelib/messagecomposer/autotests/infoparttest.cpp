/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#include "infoparttest.h"
#include "../src/part/infopart.h"
#include <qtest.h>
InfoPartTest::InfoPartTest(QObject *parent)
    : QObject(parent)
{

}

InfoPartTest::~InfoPartTest()
{

}

void InfoPartTest::shouldHaveDefaultValue()
{
    MessageComposer::InfoPart infopart;
    QCOMPARE(infopart.transportId(), 0);
    QVERIFY(!infopart.urgent());
    QVERIFY(infopart.from().isEmpty());
    QVERIFY(infopart.to().isEmpty());
    QVERIFY(infopart.cc().isEmpty());
    QVERIFY(infopart.bcc().isEmpty());
    QVERIFY(infopart.replyTo().isEmpty());
    QVERIFY(infopart.subject().isEmpty());
    QVERIFY(infopart.fcc().isEmpty());
    QVERIFY(infopart.userAgent().isEmpty());
    QVERIFY(infopart.inReplyTo().isEmpty());
    QVERIFY(infopart.references().isEmpty());
    QVERIFY(infopart.extraHeaders().isEmpty());
}

QTEST_MAIN(InfoPartTest)
