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
#include "../filter/invalidfilters/invalidfilterlistview.h"
#include "../filter/invalidfilters/invalidfilterinfo.h"
#include <qtest_kde.h>

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
    lst.append(MailCommon::InvalidFilterInfo(QLatin1String("foo"), QLatin1String("bla")));
    lst.append(MailCommon::InvalidFilterInfo(QLatin1String("foo1"), QLatin1String("bla1")));
    lst.append(MailCommon::InvalidFilterInfo(QLatin1String("foo2"), QLatin1String("bla2")));
    w.setInvalidFilters(lst);
    QCOMPARE(w.model()->rowCount(), 3);
}

QTEST_KDEMAIN(InvalidFilterListWidgetTest, GUI)
