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

#include "mergecontactselectinformationdialogtest.h"
#include "mergecontactselectinformationdialog.h"
#include "mergecontactshowresulttabwidget.h"
#include <qtest.h>


using namespace KABMergeContacts;

MergeContactSelectInformationDialogTest::MergeContactSelectInformationDialogTest()
{
}

void MergeContactSelectInformationDialogTest::shouldHaveDefaultValueOnCreation()
{
    Akonadi::Item::List lst;
    MergeContactSelectInformationDialog dlg(lst);
    dlg.show();
    KABMergeContacts::MergeContactShowResultTabWidget *tabWidget = qFindChild<KABMergeContacts::MergeContactShowResultTabWidget *>(&dlg, QLatin1String("tabwidget"));
    QVERIFY(tabWidget);
    QCOMPARE(tabWidget->count(), 0);
    QCOMPARE(tabWidget->tabBarVisible(), false);

}

QTEST_MAIN(MergeContactSelectInformationDialogTest )
