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

#include "searchduplicateresultwidgettest.h"
#include "../searchduplicate/searchduplicateresultwidget.h"
#include <kaddressbookgrantlee/widget/grantleecontactviewer.h>
#include <QSplitter>
#include <QTreeWidget>
#include <qtest_kde.h>

SearchDuplicateResultWidgetTest::SearchDuplicateResultWidgetTest(QObject *parent)
    : QObject(parent)
{

}

SearchDuplicateResultWidgetTest::~SearchDuplicateResultWidgetTest()
{

}

void SearchDuplicateResultWidgetTest::shouldHaveDefaultValue()
{
    KABMergeContacts::SearchDuplicateResultWidget w;
    QTreeWidget *tree = qFindChild<QTreeWidget *>(&w, QLatin1String("result_treewidget"));
    QVERIFY(tree);
    QSplitter *splitter = qFindChild<QSplitter *>(&w, QLatin1String("splitter"));
    QVERIFY(splitter);
    KAddressBookGrantlee::GrantleeContactViewer *viewer = qFindChild<KAddressBookGrantlee::GrantleeContactViewer *>(&w, QLatin1String("contact_viewer"));
    QVERIFY(viewer);
}

QTEST_KDEMAIN(SearchDuplicateResultWidgetTest, GUI)
