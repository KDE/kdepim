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

#include "followupreminderselectdatedialogtest.h"
#include "../src/followupreminder/followupreminderselectdatedialog.h"
#include <qtest.h>
#include <KDateComboBox>
#include <AkonadiWidgets/CollectionComboBox>

#include <QLineEdit>
#include <QPushButton>
#include <AkonadiCore/EntityTreeModel>
#include <QStandardItemModel>
#include <KCalCore/Todo>

FollowupReminderSelectDateDialogTest::FollowupReminderSelectDateDialogTest(QObject *parent)
    : QObject(parent)
{

}

FollowupReminderSelectDateDialogTest::~FollowupReminderSelectDateDialogTest()
{

}

QStandardItemModel *FollowupReminderSelectDateDialogTest::defaultItemModel()
{
    QStandardItemModel *model = new QStandardItemModel;
    for (int id = 42; id < 51; ++id) {
        Akonadi::Collection collection(id);
        collection.setRights(Akonadi::Collection::AllRights);
        collection.setName(QString::number(id));
        collection.setContentMimeTypes(QStringList() << KCalCore::Todo::todoMimeType());

        QStandardItem *item = new QStandardItem(collection.name());
        item->setData(QVariant::fromValue(collection),
                      Akonadi::EntityTreeModel::CollectionRole);
        item->setData(QVariant::fromValue(collection.id()),
                      Akonadi::EntityTreeModel::CollectionIdRole);

        model->appendRow(item);
    }
    return model;
}

void FollowupReminderSelectDateDialogTest::shouldHaveDefaultValue()
{
    MessageComposer::FollowUpReminderSelectDateDialog dlg(0, defaultItemModel());
    KDateComboBox *datecombobox = dlg.findChild<KDateComboBox *>(QStringLiteral("datecombobox"));
    QVERIFY(datecombobox);

    Akonadi::CollectionComboBox *combobox = dlg.findChild<Akonadi::CollectionComboBox *>(QStringLiteral("collectioncombobox"));
    QVERIFY(combobox);
    QDate currentDate = QDate::currentDate();
    QCOMPARE(datecombobox->date(), currentDate.addDays(1));

    QPushButton *okButton = dlg.findChild<QPushButton *>(QStringLiteral("ok_button"));
    QVERIFY(okButton);
    QVERIFY(okButton->isEnabled());
}

void FollowupReminderSelectDateDialogTest::shouldDisableOkButtonIfDateIsEmpty()
{
    MessageComposer::FollowUpReminderSelectDateDialog dlg(0, defaultItemModel());
    KDateComboBox *datecombobox = dlg.findChild<KDateComboBox *>(QStringLiteral("datecombobox"));
    QVERIFY(datecombobox);
    QPushButton *okButton = dlg.findChild<QPushButton *>(QStringLiteral("ok_button"));
    QVERIFY(okButton->isEnabled());
    datecombobox->lineEdit()->clear();
    QVERIFY(!okButton->isEnabled());
}

void FollowupReminderSelectDateDialogTest::shouldDisableOkButtonIfDateIsNotValid()
{
    MessageComposer::FollowUpReminderSelectDateDialog dlg(0, defaultItemModel());
    KDateComboBox *datecombobox = dlg.findChild<KDateComboBox *>(QStringLiteral("datecombobox"));
    QVERIFY(datecombobox);
    datecombobox->lineEdit()->setText(QStringLiteral(" "));
    QPushButton *okButton = dlg.findChild<QPushButton *>(QStringLiteral("ok_button"));
    QVERIFY(!okButton->isEnabled());
    const QDate date = QDate::currentDate();
    datecombobox->setDate(date);
    QVERIFY(okButton->isEnabled());
}

void FollowupReminderSelectDateDialogTest::shouldDisableOkButtonIfModelIsEmpty()
{
    MessageComposer::FollowUpReminderSelectDateDialog dlg(0, new QStandardItemModel(0));
    KDateComboBox *datecombobox = dlg.findChild<KDateComboBox *>(QStringLiteral("datecombobox"));
    QVERIFY(datecombobox);
    QPushButton *okButton = dlg.findChild<QPushButton *>(QStringLiteral("ok_button"));
    QVERIFY(!okButton->isEnabled());

    datecombobox->lineEdit()->setText(QStringLiteral(" "));
    QVERIFY(!okButton->isEnabled());
    const QDate date = QDate::currentDate();
    datecombobox->setDate(date);
    QVERIFY(!okButton->isEnabled());
}

QTEST_MAIN(FollowupReminderSelectDateDialogTest)
