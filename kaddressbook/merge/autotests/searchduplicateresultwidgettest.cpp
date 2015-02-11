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
#include "../searchduplicate/resultduplicatetreewidget.h"
#include "../widgets/mergecontactloseinformationwarning.h"
#include <kaddressbookgrantlee/widget/grantleecontactviewer.h>
#include <QSplitter>
#include <QTreeWidget>
#include <QLabel>
#include <qtest.h>
#include <QPushButton>
#include <qstandarditemmodel.h>
#include <AkonadiWidgets/CollectionComboBox>
#include <AkonadiCore/EntityTreeModel>
#include <KContacts/Addressee>

using namespace KContacts;
namespace KABMergeContacts
{
KADDRESSBOOK_EXPORT QAbstractItemModel *_k_searchDuplicateResultStubModel = 0;
}

SearchDuplicateResultWidgetTest::SearchDuplicateResultWidgetTest(QObject *parent)
    : QObject(parent)
{
    QStandardItemModel *model = new QStandardItemModel;
    for (int id = 42; id < 51; ++id) {
        Akonadi::Collection collection(id);
        collection.setRights(Akonadi::Collection::AllRights);
        collection.setName(QString::number(id));
        collection.setContentMimeTypes(QStringList() << Addressee::mimeType());

        QStandardItem *item = new QStandardItem(collection.name());
        item->setData(QVariant::fromValue(collection),
                      Akonadi::EntityTreeModel::CollectionRole);
        item->setData(QVariant::fromValue(collection.id()),
                      Akonadi::EntityTreeModel::CollectionIdRole);

        model->appendRow(item);
    }
    KABMergeContacts::_k_searchDuplicateResultStubModel = model;
}

SearchDuplicateResultWidgetTest::~SearchDuplicateResultWidgetTest()
{

}

void SearchDuplicateResultWidgetTest::shouldHaveDefaultValue()
{
    KABMergeContacts::SearchDuplicateResultWidget w;
    KABMergeContacts::ResultDuplicateTreeWidget *tree = qFindChild<KABMergeContacts::ResultDuplicateTreeWidget *>(&w, QLatin1String("result_treewidget"));
    QVERIFY(tree);
    QCOMPARE(tree->topLevelItemCount(), 0);
    QSplitter *splitter = qFindChild<QSplitter *>(&w, QLatin1String("splitter"));
    QVERIFY(splitter);
    QVERIFY(!splitter->childrenCollapsible());
    KAddressBookGrantlee::GrantleeContactViewer *viewer = qFindChild<KAddressBookGrantlee::GrantleeContactViewer *>(&w, QLatin1String("contact_viewer"));
    QVERIFY(viewer);
    QLabel *lab = qFindChild<QLabel *>(&w, QLatin1String("select_addressbook_label"));
    lab->setObjectName(QLatin1String("select_addressbook_label"));
    QPushButton *pushButton = qFindChild<QPushButton *>(&w, QLatin1String("merge_contact_button"));
    QVERIFY(pushButton);
    QVERIFY(!pushButton->isEnabled());

    KABMergeContacts::MergeContactLoseInformationWarning *warning = qFindChild<KABMergeContacts::MergeContactLoseInformationWarning *>(&w, QLatin1String("mergecontactwarning"));
    QVERIFY(warning);

    Akonadi::CollectionComboBox *combobox = qFindChild<Akonadi::CollectionComboBox *>(&w, QLatin1String("akonadicombobox"));
    QVERIFY(combobox);
}

void SearchDuplicateResultWidgetTest::shouldHaveMergeButtonEnabled()
{
    KABMergeContacts::SearchDuplicateResultWidget w;
    KABMergeContacts::ResultDuplicateTreeWidget *tree = qFindChild<KABMergeContacts::ResultDuplicateTreeWidget *>(&w, QLatin1String("result_treewidget"));
    QVERIFY(tree);
    QCOMPARE(tree->topLevelItemCount(), 0);
    Akonadi::Item::List lst;
    lst << Akonadi::Item(42);
    lst << Akonadi::Item(43);
    lst << Akonadi::Item(44);
    QList<Akonadi::Item::List> itemLst;
#if 0 //FIXME
    tree->setContacts(itemLst);
    QVERIFY(tree->topLevelItemCount() > 0);

    KPushButton *pushButton = qFindChild<KPushButton *>(&w, QLatin1String("merge_contact_button"));
    QVERIFY(pushButton);
    QVERIFY(!pushButton->isEnabled());

    Akonadi::CollectionComboBox *combobox = qFindChild<Akonadi::CollectionComboBox *>(&w, QLatin1String("akonadicombobox"));
    QVERIFY(combobox);
#endif
}

QTEST_MAIN(SearchDuplicateResultWidgetTest)
