/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#include "mergecontactduplicatecontactdialogtest.h"

#include "../mergecontactduplicatecontactdialog.h"

#include <qtest_kde.h>

#include <QStackedWidget>
using namespace KABMergeContacts;

MergeContactDuplicateContactDialogTest::MergeContactDuplicateContactDialogTest()
{
}

void MergeContactDuplicateContactDialogTest::shouldHaveDefaultValueOnCreation()
{
    Akonadi::Item::List lst;
    MergeContactDuplicateContactDialog dlg(lst);
    dlg.show();
    QStackedWidget *stackedWidget = qFindChild<QStackedWidget *>(&dlg, QLatin1String("stackedwidget"));
    QVERIFY(stackedWidget);
    QCOMPARE(stackedWidget->currentWidget()->objectName(), QLatin1String("nocontactselected"));
}

void MergeContactDuplicateContactDialogTest::shouldShowNoEnoughPageWhenSelectOneContact()
{
    Akonadi::Item::List lst;
    lst << Akonadi::Item(42);
    MergeContactDuplicateContactDialog dlg(lst);
    dlg.show();
    QStackedWidget *stackedWidget = qFindChild<QStackedWidget *>(&dlg, QLatin1String("stackedwidget"));
    QVERIFY(stackedWidget);
    QCOMPARE(stackedWidget->currentWidget()->objectName(), QLatin1String("noenoughcontactselected"));
}


QTEST_KDEMAIN(MergeContactDuplicateContactDialogTest, GUI)
