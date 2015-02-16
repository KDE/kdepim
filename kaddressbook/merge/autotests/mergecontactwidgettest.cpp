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

#include "mergecontactwidgettest.h"
#include <AkonadiCore/Item>
#include <KContacts/Addressee>
#include <AkonadiCore/EntityTreeModel>
#include "../manualmerge/mergecontactwidget.h"

#include <qtest.h>
#include <qtestmouse.h>
#include <QListWidget>
#include <QPushButton>
#include <QStandardItemModel>
#include <QSignalSpy>
#include <QSplitter>

#include <kaddressbook/merge/widgets/mergecontactinfowidget.h>
#include <kaddressbook/merge/widgets/mergecontactloseinformationwarning.h>
using namespace KABMergeContacts;

namespace KABMergeContacts
{
extern KADDRESSBOOK_EXPORT QAbstractItemModel *_k_mergeStubModel;
}

MergeContactWidgetTest::MergeContactWidgetTest()
{
    qRegisterMetaType<Akonadi::Item::List>();
    qRegisterMetaType<Akonadi::Item>();
    qRegisterMetaType<Akonadi::Collection>();

    QStandardItemModel *model = new QStandardItemModel;
    for (int id = 42; id < 51; ++id) {
        Akonadi::Collection collection(id);
        collection.setRights(Akonadi::Collection::AllRights);
        collection.setName(QString::number(id));
        collection.setContentMimeTypes(QStringList() << KContacts::Addressee::mimeType());

        QStandardItem *item = new QStandardItem(collection.name());
        item->setData(QVariant::fromValue(collection),
                      Akonadi::EntityTreeModel::CollectionRole);
        item->setData(QVariant::fromValue(collection.id()),
                      Akonadi::EntityTreeModel::CollectionIdRole);

        model->appendRow(item);
    }
    KABMergeContacts::_k_mergeStubModel = model;
}

Akonadi::Item::List MergeContactWidgetTest::createItems()
{
    Akonadi::Item::List lst;
    for (int i = 0; i < 10; ++i) {
        Akonadi::Item item(i);
        KContacts::Addressee address;
        item.setPayload(address);
        lst.append(item);
    }
    return lst;
}

void MergeContactWidgetTest::shouldHaveDefaultValueOnCreation()
{
    MergeContactWidget mergeWidget;
    QListWidget *listWidget = qFindChild<QListWidget *>(&mergeWidget, QLatin1String("listcontact"));
    QVERIFY(listWidget);
    QCOMPARE(listWidget->count(), 0);
    QPushButton *button = mergeWidget.findChild<QPushButton *>(QStringLiteral("mergebutton"));
    QVERIFY(button);
    QCOMPARE(button->isEnabled(), false);
    MergeContactLoseInformationWarning *warningWidget = qFindChild<MergeContactLoseInformationWarning *>(&mergeWidget, QLatin1String("mergecontactwarning"));
    QVERIFY(warningWidget);
    QVERIFY(warningWidget->isHidden());

    QWidget *selectContactWidget = qFindChild<QWidget *>(&mergeWidget, QLatin1String("selectcontactwidget"));
    QVERIFY(selectContactWidget);

    MergeContactInfoWidget *contactInfoWidget = qFindChild<MergeContactInfoWidget *>(&mergeWidget, QLatin1String("mergecontactinfowidget"));
    QVERIFY(contactInfoWidget);

    MergeContactLoseInformationWarning *warning = qFindChild<MergeContactLoseInformationWarning *>(&mergeWidget, QLatin1String("mergecontactwarning"));
    QVERIFY(warning);

    QSplitter *splitter = qFindChild<QSplitter *>(&mergeWidget, QLatin1String("splitter"));
    QVERIFY(splitter);
    QVERIFY(!splitter->childrenCollapsible());
    for (int i = 0; i < splitter->count(); ++i) {
        const QString objName = splitter->widget(i)->objectName();

        const bool hasName = (objName == QLatin1String("selectcontactwidget")) || (objName == QLatin1String("mergecontactinfowidget"));
        QVERIFY(hasName);
    }
}

void MergeContactWidgetTest::shouldFillList()
{
    MergeContactWidget mergeWidget;
    mergeWidget.setContacts(createItems());
    QListWidget *listWidget = qFindChild<QListWidget *>(&mergeWidget, QLatin1String("listcontact"));
    QCOMPARE(listWidget->count(), 10);
    QCOMPARE(listWidget->selectedItems().count(), 0);
    QPushButton *button = mergeWidget.findChild<QPushButton *>(QStringLiteral("mergebutton"));
    QCOMPARE(button->isEnabled(), false);
}

void MergeContactWidgetTest::shouldFillListWithValidItem()
{
    Akonadi::Item::List lst = createItems();
    QCOMPARE(lst.count(), 10);
    lst.append(Akonadi::Item(25));
    MergeContactWidget mergeWidget;
    mergeWidget.setContacts(lst);
    QListWidget *listWidget = qFindChild<QListWidget *>(&mergeWidget, QLatin1String("listcontact"));
    QCOMPARE(listWidget->count(), 10);
}

void MergeContactWidgetTest::shouldEnableButton()
{
    MergeContactWidget mergeWidget;
    mergeWidget.setContacts(createItems());
    QListWidget *listWidget = qFindChild<QListWidget *>(&mergeWidget, QLatin1String("listcontact"));
    QPushButton *button = qFindChild<QPushButton *>(&mergeWidget, QLatin1String("mergebutton"));
    mergeWidget.show();
    QTest::qWaitForWindowExposed(&mergeWidget);
    listWidget->item(0)->setCheckState(Qt::Checked);
    listWidget->item(1)->setCheckState(Qt::Checked);
    QCOMPARE(button->isEnabled(), true);
}

void MergeContactWidgetTest::shouldEmitSignalsWhenThereIsElementSelected()
{
#if 0 //FIXME
    MergeContactWidget mergeWidget;
    mergeWidget.setContacts(createItems());
    QListWidget *listWidget = qFindChild<QListWidget *>(&mergeWidget, QLatin1String("listcontact"));
    QPushButton *button = qFindChild<QPushButton *>(&mergeWidget, QLatin1String("mergebutton"));
    mergeWidget.show();
    QTest::qWaitForWindowExposed(&mergeWidget);
    listWidget->item(0)->setCheckState(Qt::Checked);
    listWidget->item(1)->setCheckState(Qt::Checked);
    QSignalSpy spy(&mergeWidget, SIGNAL(mergeContact(Akonadi::Item::List,Akonadi::Collection)));
    QTest::mouseClick(button, Qt::LeftButton);
    QCOMPARE(spy.count(), 1);
    listWidget->item(1)->setCheckState(Qt::Unchecked);
    QTest::mouseClick(button, Qt::LeftButton);
    QCOMPARE(spy.count(), 1); //No new signal emited when we are not 2 items checked
#endif
}

void MergeContactWidgetTest::shouldEmitSignalsWhenThereIsTwoElementsSelected()
{
#if 0 //FIXME
    MergeContactWidget mergeWidget;
    mergeWidget.setContacts(createItems());
    QListWidget *listWidget = qFindChild<QListWidget *>(&mergeWidget, QLatin1String("listcontact"));
    QPushButton *button = qFindChild<QPushButton *>(&mergeWidget, QLatin1String("mergebutton"));
    mergeWidget.show();
    QTest::qWaitForWindowExposed(&mergeWidget);
    listWidget->item(0)->setCheckState(Qt::Checked);
    QSignalSpy spy(&mergeWidget, SIGNAL(mergeContact(Akonadi::Item::List,Akonadi::Collection)));
    QTest::mouseClick(button, Qt::LeftButton);
    QCOMPARE(spy.count(), 0);
    listWidget->item(1)->setCheckState(Qt::Checked);
    QTest::mouseClick(button, Qt::LeftButton);
    QCOMPARE(spy.count(), 1);
#endif
}

void MergeContactWidgetTest::shouldEmitSignalsWhenSelectContact()
{
#if 0 //FIXME
    MergeContactWidget mergeWidget;
    mergeWidget.setContacts(createItems());
    QListWidget *listWidget = qFindChild<QListWidget *>(&mergeWidget, QLatin1String("listcontact"));
    mergeWidget.show();
    QSignalSpy spy(&mergeWidget, SIGNAL(contactSelected(Akonadi::Item)));
    listWidget->item(1)->setSelected(true);
    QCOMPARE(spy.count(), 1);

    listWidget->clearSelection();
    QCOMPARE(spy.count(), 2);
#endif
}

QTEST_MAIN(MergeContactWidgetTest)
