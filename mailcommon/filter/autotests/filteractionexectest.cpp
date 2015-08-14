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

#include "filteractionexectest.h"
#include <qtest.h>
#include "../filteractions/filteractionexec.h"
#include "../filteractions/filteractionwithurl.h"

#include <KUrlRequester>

FilterActionExecTest::FilterActionExecTest(QObject *parent)
    : QObject(parent)
{

}

FilterActionExecTest::~FilterActionExecTest()
{

}

void FilterActionExecTest::shouldHaveDefaultValue()
{
    MailCommon::FilterActionExec filter;
    QWidget *w = filter.createParamWidget(0);
    KUrlRequester *requester = w->findChild<KUrlRequester *>(QStringLiteral("requester"));
    QVERIFY(requester);

    MailCommon::FilterActionWithUrlHelpButton *helpButton = w->findChild<MailCommon::FilterActionWithUrlHelpButton *>(QStringLiteral("helpbutton"));
    QVERIFY(helpButton);
}

void FilterActionExecTest::shouldHaveRequirePart()
{
    MailCommon::FilterActionExec filter;
    QCOMPARE(filter.requiredPart(), MailCommon::SearchRule::CompleteMessage);
}

QTEST_MAIN(FilterActionExecTest)
