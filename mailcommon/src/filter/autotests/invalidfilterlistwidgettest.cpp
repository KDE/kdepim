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

#include "invalidfilterlistwidgettest.h"
#include <qtest.h>
#include "../filter/invalidfilters/invalidfilterlistview.h"
#include "../filter/invalidfilters/invalidfilterinfo.h"

InvalidFilterListWidgetTest::InvalidFilterListWidgetTest(QObject *parent)
    : QObject(parent)
{

}

InvalidFilterListWidgetTest::~InvalidFilterListWidgetTest()
{

}

void InvalidFilterListWidgetTest::shouldHaveDefaultValue()
{
    MailCommon::InvalidFilterListView w;
    QCOMPARE(w.model()->rowCount(), 0);
}

void InvalidFilterListWidgetTest::shouldAddInvalidFilters()
{
    MailCommon::InvalidFilterListView w;
    QVector<MailCommon::InvalidFilterInfo> lst;
    lst.append(MailCommon::InvalidFilterInfo(QStringLiteral("foo"), QStringLiteral("bla")));
    lst.append(MailCommon::InvalidFilterInfo(QStringLiteral("foo1"), QStringLiteral("bla1")));
    lst.append(MailCommon::InvalidFilterInfo(QStringLiteral("foo2"), QStringLiteral("bla2")));
    w.setInvalidFilters(lst);
    QCOMPARE(w.model()->rowCount(), 3);
}

QTEST_MAIN(InvalidFilterListWidgetTest)
