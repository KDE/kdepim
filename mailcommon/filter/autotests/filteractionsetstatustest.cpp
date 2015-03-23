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

#include "filteractionsetstatustest.h"
#include "../filteractions/filteractionsetstatus.h"
#include <qtest.h>
#include <QWidget>

FilterActionSetStatusTest::FilterActionSetStatusTest(QObject *parent)
    : QObject(parent)
{

}

FilterActionSetStatusTest::~FilterActionSetStatusTest()
{

}

void FilterActionSetStatusTest::shouldHaveDefaultValue()
{
    MailCommon::FilterActionSetStatus filter;
    QWidget *w = filter.createParamWidget(0);
    QCOMPARE(w->objectName(), QLatin1String("combobox"));
}

void FilterActionSetStatusTest::shouldHaveSieveRequires()
{
    MailCommon::FilterActionSetStatus filter;
    QCOMPARE(filter.sieveRequires(), QStringList() << QLatin1String("imap4flags"));
}

QTEST_MAIN(FilterActionSetStatusTest)
