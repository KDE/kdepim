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

#include "mailfiltertest.h"
#include "../mailfilter.h"
#include <qtest.h>

MailFilterTest::MailFilterTest(QObject *parent)
    : QObject(parent)
{

}

MailFilterTest::~MailFilterTest()
{

}

void MailFilterTest::shouldHaveDefaultValue()
{
    MailCommon::MailFilter mailfilter;
    QVERIFY(mailfilter.isEmpty());
    QVERIFY(mailfilter.isEnabled());
    QVERIFY(mailfilter.applyOnInbound());
    QVERIFY(!mailfilter.applyBeforeOutbound());
    QVERIFY(mailfilter.applyOnExplicit());
    QVERIFY(mailfilter.stopProcessingHere());
    QVERIFY(!mailfilter.configureShortcut());
    QVERIFY(!mailfilter.configureToolbar());
    QVERIFY(mailfilter.isAutoNaming());
    QCOMPARE(mailfilter.applicability(), MailCommon::MailFilter::All);
    QVERIFY(mailfilter.actions()->isEmpty());
    QVERIFY(mailfilter.icon().isEmpty());

}

void MailFilterTest::shouldApplySettings()
{
    MailCommon::MailFilter mailfilter;
    bool value = true;
    mailfilter.setApplyBeforeOutbound(value);
    QCOMPARE(mailfilter.applyBeforeOutbound(), value);
    value = false;
    mailfilter.setApplyBeforeOutbound(value);
    QCOMPARE(mailfilter.applyBeforeOutbound(), value);
}

QTEST_MAIN(MailFilterTest)
