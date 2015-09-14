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

#include "filteractionmovetest.h"
#include <qtest.h>
#include "../filteractions/filteractionmove.h"
#include "folder/folderrequester.h"

FilterActionMoveTest::FilterActionMoveTest(QObject *parent)
    : QObject(parent)
{

}

FilterActionMoveTest::~FilterActionMoveTest()
{

}

void FilterActionMoveTest::shouldHaveSieveRequires()
{
    MailCommon::FilterActionMove filter;
    QCOMPARE(filter.sieveRequires(), QStringList() << QStringLiteral("fileinto"));
}

void FilterActionMoveTest::shouldHaveRequiresPart()
{
    MailCommon::FilterActionMove filter;
    QCOMPARE(filter.requiredPart(), MailCommon::SearchRule::Envelope);
}

void FilterActionMoveTest::shouldHaveDefaultValue()
{
    MailCommon::FilterActionMove filter;
    QWidget *w = filter.createParamWidget(0);
    QVERIFY(w);
    MailCommon::FolderRequester *requester = dynamic_cast<MailCommon::FolderRequester *>(w);
    QVERIFY(requester);
    QCOMPARE(requester->objectName(), QStringLiteral("folderrequester"));
}

QTEST_MAIN(FilterActionMoveTest)
