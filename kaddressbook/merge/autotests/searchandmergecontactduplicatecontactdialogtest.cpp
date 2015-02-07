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

#include "searchandmergecontactduplicatecontactdialogtest.h"

#include "../searchduplicate/searchandmergecontactduplicatecontactdialog.h"

#include <qtest.h>

#include <QStackedWidget>
using namespace KABMergeContacts;

SearchAndMergeContactDuplicateContactDialogTest::SearchAndMergeContactDuplicateContactDialogTest()
{
}

void SearchAndMergeContactDuplicateContactDialogTest::shouldHaveDefaultValueOnCreation()
{
    SearchAndMergeContactDuplicateContactDialog dlg;
    dlg.show();
    QStackedWidget *stackedWidget = dlg.findChild<QStackedWidget *>(QStringLiteral("stackedwidget"));
    QVERIFY(stackedWidget);
    QCOMPARE(stackedWidget->currentWidget()->objectName(), QStringLiteral("nocontactselected"));

    for(int i = 0; i < stackedWidget->count(); ++i) {
        QWidget *w = stackedWidget->widget(i);
        bool hasGoodNamePage = (w->objectName() == QLatin1String("mergecontact") ||
                                w->objectName() == QLatin1String("nocontactselected") ||
                                w->objectName() == QLatin1String("noduplicatecontactfound") ||
                                w->objectName() == QLatin1String("noenoughcontactselected") ||
                                w->objectName() == QLatin1String("mergecontactresult"));
        QVERIFY(hasGoodNamePage);
    }

}

void SearchAndMergeContactDuplicateContactDialogTest::shouldShowNoEnoughPageWhenSelectOneContact()
{
    SearchAndMergeContactDuplicateContactDialog dlg;
    Akonadi::Item::List lst;
    lst << Akonadi::Item(42);
    dlg.searchPotentialDuplicateContacts(lst);
    dlg.show();
    QStackedWidget *stackedWidget = dlg.findChild<QStackedWidget *>(QStringLiteral("stackedwidget"));
    QVERIFY(stackedWidget);
    QCOMPARE(stackedWidget->currentWidget()->objectName(), QStringLiteral("noenoughcontactselected"));
}

void SearchAndMergeContactDuplicateContactDialogTest::shouldShowNoContactWhenListIsEmpty()
{
    SearchAndMergeContactDuplicateContactDialog dlg;
    Akonadi::Item::List lst;
    dlg.searchPotentialDuplicateContacts(lst);
    dlg.show();
    QStackedWidget *stackedWidget = qFindChild<QStackedWidget *>(&dlg, QLatin1String("stackedwidget"));
    QVERIFY(stackedWidget);
    QCOMPARE(stackedWidget->currentWidget()->objectName(), QLatin1String("nocontactselected"));
}


QTEST_MAIN(SearchAndMergeContactDuplicateContactDialogTest)
