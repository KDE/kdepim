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

#include "eventedittest.h"
#include "../createeventplugin/eventedit.h"
#include "../createeventplugin/eventdatetimewidget.h"
#include "globalsettings_messageviewer.h"

#include <AkonadiCore/Collection>
#include <AkonadiWidgets/CollectionComboBox>
#include <AkonadiCore/EntityTreeModel>
#include <QStandardItemModel>
#include <KCalCore/Event>
#include <QPushButton>

#include <qtest.h>
#include <qtestkeyboard.h>
#include <qtestmouse.h>

#include <QSignalSpy>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QShortcut>
#include <QAction>

namespace MessageViewer
{
extern MESSAGEVIEWER_EXPORT QAbstractItemModel *_k_eventEditStubModel;
}

EventEditTest::EventEditTest()
{
    qRegisterMetaType<Akonadi::Collection>();
    qRegisterMetaType<KMime::Message::Ptr>();
    qRegisterMetaType<KCalCore::Event::Ptr>();

    QStandardItemModel *model = new QStandardItemModel;
    for (int id = 42; id < 51; ++id) {
        Akonadi::Collection collection(id);
        collection.setRights(Akonadi::Collection::AllRights);
        collection.setName(QString::number(id));
        collection.setContentMimeTypes(QStringList() << KCalCore::Event::eventMimeType());

        QStandardItem *item = new QStandardItem(collection.name());
        item->setData(QVariant::fromValue(collection),
                      Akonadi::EntityTreeModel::CollectionRole);
        item->setData(QVariant::fromValue(collection.id()),
                      Akonadi::EntityTreeModel::CollectionIdRole);

        model->appendRow(item);
    }
    MessageViewer::_k_eventEditStubModel = model;
}

void EventEditTest::shouldHaveDefaultValuesOnCreation()
{
    MessageViewer::EventEdit edit;
    //We can't test it. Collection value is stored in settings here, and not in jenkins so disable it
    QVERIFY(edit.collection().isValid());
    QVERIFY(!edit.message());
    QLineEdit *eventedit = edit.findChild<QLineEdit *>(QStringLiteral("eventedit"));
    QVERIFY(eventedit);
    QCOMPARE(eventedit->text(), QString());

    QPushButton *openEditor = edit.findChild<QPushButton *>(QStringLiteral("open-editor-button"));
    QPushButton *save = edit.findChild<QPushButton *>(QStringLiteral("save-button"));
    QVERIFY(openEditor);
    QVERIFY(save);
    QCOMPARE(openEditor->isEnabled(), false);
    QCOMPARE(save->isEnabled(), false);

    QDateTime currentDateTime = QDateTime::currentDateTime();
    MessageViewer::EventDateTimeWidget *startDateTime = edit.findChild<MessageViewer::EventDateTimeWidget *>(QStringLiteral("startdatetimeedit"));
    QVERIFY(startDateTime);
    QCOMPARE(startDateTime->dateTime().date(), currentDateTime.date());
    QCOMPARE(startDateTime->dateTime().time().hour(), currentDateTime.time().hour());
    QCOMPARE(startDateTime->dateTime().time().minute(), currentDateTime.time().minute());

    MessageViewer::EventDateTimeWidget *endDateTime = edit.findChild<MessageViewer::EventDateTimeWidget *>(QStringLiteral("enddatetimeedit"));
    QVERIFY(endDateTime);
    currentDateTime = currentDateTime.addSecs(3600); // +1hour
    QCOMPARE(endDateTime->dateTime().date(), currentDateTime.date());
    QCOMPARE(endDateTime->dateTime().time().hour(), currentDateTime.time().hour());
    QCOMPARE(endDateTime->dateTime().time().minute(), currentDateTime.time().minute());
}

void EventEditTest::shouldEmitCollectionChanged()
{
    MessageViewer::EventEdit edit;
    QSignalSpy spy(&edit, SIGNAL(collectionChanged(Akonadi::Collection)));
    edit.setCollection(Akonadi::Collection(42));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).value<Akonadi::Collection>(), Akonadi::Collection(42));
}

void EventEditTest::shouldEmitMessageChanged()
{
    MessageViewer::EventEdit edit;
    QSignalSpy spy(&edit, SIGNAL(messageChanged(KMime::Message::Ptr)));
    KMime::Message::Ptr msg(new KMime::Message);
    edit.setMessage(msg);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).value<KMime::Message::Ptr>(), msg);
}

void EventEditTest::shouldNotEmitWhenCollectionIsNotChanged()
{
    MessageViewer::EventEdit edit;
    edit.setCollection(Akonadi::Collection(42));
    QSignalSpy spy(&edit, SIGNAL(collectionChanged(Akonadi::Collection)));
    edit.setCollection(Akonadi::Collection(42));
    QCOMPARE(spy.count(), 0);
}

void EventEditTest::shouldNotEmitWhenMessageIsNotChanged()
{
    MessageViewer::EventEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    edit.setMessage(msg);
    QSignalSpy spy(&edit, SIGNAL(messageChanged(KMime::Message::Ptr)));
    edit.setMessage(msg);
    QCOMPARE(spy.count(), 0);
}

void EventEditTest::shouldEmitEventWhenPressEnter()
{
    MessageViewer::EventEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QStringLiteral("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *eventedit = edit.findChild<QLineEdit *>(QStringLiteral("eventedit"));
    eventedit->setFocus();
    QSignalSpy spy(&edit, SIGNAL(createEvent(KCalCore::Event::Ptr,Akonadi::Collection)));
    QTest::keyClick(eventedit, Qt::Key_Enter);
    QCOMPARE(spy.count(), 1);
}

void EventEditTest::shouldHideWidgetWhenPressEnter()
{
    MessageViewer::EventEdit edit;
    edit.show();
    QTest::qWaitForWindowExposed(&edit);
    QVERIFY(edit.isVisible());

    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QStringLiteral("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *eventedit = edit.findChild<QLineEdit *>(QStringLiteral("eventedit"));
    QTest::keyClick(eventedit, Qt::Key_Enter);
    QCOMPARE(edit.isVisible(), false);
}

void EventEditTest::shouldHideWidgetWhenPressEscape()
{
    MessageViewer::EventEdit edit;
    edit.show();
    QTest::qWaitForWindowExposed(&edit);
    QLineEdit *eventedit = edit.findChild<QLineEdit *>(QStringLiteral("eventedit"));
    eventedit->setFocus();
    QVERIFY(eventedit->hasFocus());
    QTest::keyPress(&edit, Qt::Key_Escape);
    QCOMPARE(edit.isVisible(), false);
}

void EventEditTest::shouldHideWidgetWhenSaveClicked()
{
    MessageViewer::EventEdit edit;
    edit.show();
    QTest::qWaitForWindowExposed(&edit);

    KMime::Message::Ptr msg(new KMime::Message);
    msg->subject(true)->fromUnicodeString(QStringLiteral("Test Note"), "us-ascii");
    edit.setMessage(msg);
    QPushButton *save = edit.findChild<QPushButton *>(QStringLiteral("save-button"));
    QTest::mouseClick(save, Qt::LeftButton);
    QCOMPARE(edit.isVisible(), false);
}

void EventEditTest::shouldSaveCollectionSettings()
{
    MessageViewer::EventEdit edit;
    edit.show();
    QTest::qWaitForWindowExposed(&edit);
    Akonadi::CollectionComboBox *akonadicombobox = edit.findChild<Akonadi::CollectionComboBox *>(QStringLiteral("akonadicombobox"));
    akonadicombobox->setCurrentIndex(3);
    const Akonadi::Collection::Id id = akonadicombobox->currentCollection().id();
    QPushButton *close = edit.findChild<QPushButton *>(QStringLiteral("close-button"));
    QTest::mouseClick(close, Qt::LeftButton);
    QCOMPARE(MessageViewer::MessageViewerSettingsBase::self()->lastEventSelectedFolder(), id);
}

void EventEditTest::shouldSaveCollectionSettingsWhenCloseWidget()
{
    MessageViewer::EventEdit edit;
    edit.show();
    QTest::qWaitForWindowExposed(&edit);
    Akonadi::CollectionComboBox *akonadicombobox = edit.findChild<Akonadi::CollectionComboBox *>(QStringLiteral("akonadicombobox"));
    akonadicombobox->setCurrentIndex(4);
    const Akonadi::Collection::Id id = akonadicombobox->currentCollection().id();
    edit.writeConfig();
    QCOMPARE(MessageViewer::MessageViewerSettingsBase::self()->lastEventSelectedFolder(), id);
}

void EventEditTest::shouldSaveCollectionSettingsWhenDeleteWidget()
{
    MessageViewer::EventEdit *edit = new MessageViewer::EventEdit;
    Akonadi::CollectionComboBox *akonadicombobox = edit->findChild<Akonadi::CollectionComboBox *>(QStringLiteral("akonadicombobox"));
    akonadicombobox->setCurrentIndex(5);
    const Akonadi::Collection::Id id = akonadicombobox->currentCollection().id();
    delete edit;
    QCOMPARE(MessageViewer::MessageViewerSettingsBase::self()->lastEventSelectedFolder(), id);
}

void EventEditTest::shouldNotEmitCreateEventWhenDateIsInvalid()
{
    MessageViewer::EventEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);

    MessageViewer::EventDateTimeWidget *startDateTime = edit.findChild<MessageViewer::EventDateTimeWidget *>(QStringLiteral("startdatetimeedit"));
    startDateTime->setDateTime(QDateTime());

    MessageViewer::EventDateTimeWidget *endDateTime = edit.findChild<MessageViewer::EventDateTimeWidget *>(QStringLiteral("enddatetimeedit"));
    endDateTime->setDateTime(QDateTime());

    QString subject = QStringLiteral("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *eventedit = edit.findChild<QLineEdit *>(QStringLiteral("eventedit"));
    QSignalSpy spy(&edit, SIGNAL(createEvent(KCalCore::Event::Ptr,Akonadi::Collection)));
    QTest::keyClick(eventedit, Qt::Key_Enter);
    QCOMPARE(spy.count(), 0);
}

void EventEditTest::shouldEventHasCorrectSubject()
{
    MessageViewer::EventEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QStringLiteral("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("eventedit"));
    QVERIFY(noteedit);
    QSignalSpy spy(&edit, SIGNAL(createEvent(KCalCore::Event::Ptr,Akonadi::Collection)));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(spy.count(), 1);
    KCalCore::Event::Ptr eventPtr = spy.at(0).at(0).value<KCalCore::Event::Ptr>();
    QVERIFY(eventPtr);
    QCOMPARE(eventPtr->summary(), subject);
}

void EventEditTest::shouldSelectLineWhenPutMessage()
{
    MessageViewer::EventEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QStringLiteral("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("eventedit"));
    QVERIFY(noteedit);
    QVERIFY(noteedit->hasSelectedText());
    const QString selectedText = noteedit->selectedText();
    QCOMPARE(selectedText, subject);
}

void EventEditTest::shouldHaveCorrectStartEndDateTime()
{
    MessageViewer::EventEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QStringLiteral("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);

    QDateTime currentDateTime = QDateTime::currentDateTime();
    MessageViewer::EventDateTimeWidget *startDateTime = edit.findChild<MessageViewer::EventDateTimeWidget *>(QStringLiteral("startdatetimeedit"));
    startDateTime->setDateTime(currentDateTime);

    QDateTime endDt = currentDateTime.addDays(1);
    MessageViewer::EventDateTimeWidget *endDateTime = edit.findChild<MessageViewer::EventDateTimeWidget *>(QStringLiteral("enddatetimeedit"));
    endDateTime->setDateTime(endDt);

    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("eventedit"));
    QVERIFY(noteedit);
    QSignalSpy spy(&edit, SIGNAL(createEvent(KCalCore::Event::Ptr,Akonadi::Collection)));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(spy.count(), 1);
    KCalCore::Event::Ptr eventPtr = spy.at(0).at(0).value<KCalCore::Event::Ptr>();
    QVERIFY(eventPtr);
    QCOMPARE(eventPtr->dtStart().date(), currentDateTime.date());
    QCOMPARE(eventPtr->dtStart().time().hour(), currentDateTime.time().hour());
    QCOMPARE(eventPtr->dtStart().time().minute(), currentDateTime.time().minute());

    QCOMPARE(eventPtr->dtEnd().date(), endDt.date());
    QCOMPARE(eventPtr->dtEnd().time().hour(), endDt.time().hour());
    QCOMPARE(eventPtr->dtEnd().time().minute(), endDt.time().minute());
}

void EventEditTest::shouldSetFocusWhenWeCallTodoEdit()
{
    MessageViewer::EventEdit edit;
    edit.show();
    QTest::qWaitForWindowExposed(&edit);
    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("eventedit"));
    QVERIFY(noteedit);
    edit.setFocus();
    edit.showEventEdit();
    QVERIFY(noteedit->hasFocus());
}

void EventEditTest::shouldEnsureEndDateIsNotBeforeStartDate()
{
    MessageViewer::EventEdit edit;
    MessageViewer::EventDateTimeWidget *startDateTime = edit.findChild<MessageViewer::EventDateTimeWidget *>(QStringLiteral("startdatetimeedit"));
    MessageViewer::EventDateTimeWidget *endDateTime = edit.findChild<MessageViewer::EventDateTimeWidget *>(QStringLiteral("enddatetimeedit"));

    QDateTime startDt = startDateTime->dateTime();
    QVERIFY(startDt < endDateTime->dateTime());

    startDt.setTime(QTime(5, 12));
    endDateTime->setDateTime(startDt.addSecs(3600));
    startDt = startDt.addDays(1);
    startDateTime->setDateTime(startDt);
    QCOMPARE(startDt.date(), endDateTime->date());
    QVERIFY(startDt.time() < endDateTime->time());

    startDt = startDt.addSecs(2 * 3600);
    startDateTime->setDateTime(startDt);
    QCOMPARE(startDt.time(), endDateTime->time());
}

void EventEditTest::shouldEnabledSaveOpenEditorButton()
{
    MessageViewer::EventEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    msg->subject(true)->fromUnicodeString(QStringLiteral("Test note"), "us-ascii");
    edit.setMessage(msg);

    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("eventedit"));
    QVERIFY(noteedit);
    QPushButton *openEditor = edit.findChild<QPushButton *>(QStringLiteral("open-editor-button"));
    QPushButton *save = edit.findChild<QPushButton *>(QStringLiteral("save-button"));
    QCOMPARE(openEditor->isEnabled(), true);
    QCOMPARE(save->isEnabled(), true);
    noteedit->clear();

    QCOMPARE(openEditor->isEnabled(), false);
    QCOMPARE(save->isEnabled(), false);

    noteedit->setText(QStringLiteral(" "));
    QCOMPARE(openEditor->isEnabled(), false);
    QCOMPARE(save->isEnabled(), false);

    noteedit->setText(QStringLiteral(" foo"));
    QCOMPARE(openEditor->isEnabled(), true);
    QCOMPARE(save->isEnabled(), true);
}

void EventEditTest::shouldUpdateStartEndDateWhenReopenIt()
{
    MessageViewer::EventEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    msg->subject(true)->fromUnicodeString(QStringLiteral("Test note"), "us-ascii");
    edit.setMessage(msg);

    QDateTime currentDateTime = QDateTime::currentDateTime();
    MessageViewer::EventDateTimeWidget *startDateTime = edit.findChild<MessageViewer::EventDateTimeWidget *>(QStringLiteral("startdatetimeedit"));
    QCOMPARE(startDateTime->dateTime().date(), currentDateTime.date());
    QCOMPARE(startDateTime->dateTime().time().hour(), currentDateTime.time().hour());
    QCOMPARE(startDateTime->dateTime().time().minute(), currentDateTime.time().minute());

    MessageViewer::EventDateTimeWidget *endDateTime = edit.findChild<MessageViewer::EventDateTimeWidget *>(QStringLiteral("enddatetimeedit"));
    auto endTime = currentDateTime.addSecs(3600);
    QCOMPARE(endDateTime->dateTime().date(), endTime.date());
    QCOMPARE(endDateTime->dateTime().time().hour(), endTime.time().hour());
    QCOMPARE(endDateTime->dateTime().time().minute(), endTime.time().minute());

    QDateTime newDateTime = currentDateTime;
    newDateTime = newDateTime.addSecs(60 * 60); //+1h
    startDateTime->setDateTime(newDateTime);
    endDateTime->setDateTime(newDateTime);

    QCOMPARE(startDateTime->dateTime().time().hour(), newDateTime.time().hour());
    QCOMPARE(startDateTime->dateTime().time().minute(), newDateTime.time().minute());
    QCOMPARE(startDateTime->dateTime().date(), newDateTime.date());

    QCOMPARE(endDateTime->dateTime().time().hour(), newDateTime.time().hour());
    QCOMPARE(endDateTime->dateTime().time().minute(), newDateTime.time().minute());
    QCOMPARE(endDateTime->dateTime().date(), newDateTime.date());

    edit.slotCloseWidget();

    edit.showEventEdit();
    QCOMPARE(startDateTime->dateTime().date(), currentDateTime.date());
    QCOMPARE(startDateTime->dateTime().time().hour(), currentDateTime.time().hour());
    QCOMPARE(startDateTime->dateTime().time().minute(), currentDateTime.time().minute());

    QCOMPARE(endDateTime->dateTime().date(), endTime.date());
    QCOMPARE(endDateTime->dateTime().time().hour(), endTime.time().hour());
    QCOMPARE(endDateTime->dateTime().time().minute(), endTime.time().minute());

}

void EventEditTest::shouldDisabledSaveOpenEditorButtonWhenCollectionComboBoxIsEmpty()
{
    MessageViewer::EventEdit edit;
    Akonadi::CollectionComboBox *akonadicombobox = edit.findChild<Akonadi::CollectionComboBox *>(QStringLiteral("akonadicombobox"));
    //Create an empty combobox
    akonadicombobox->setModel(new QStandardItemModel());
    KMime::Message::Ptr msg(new KMime::Message);
    msg->subject(true)->fromUnicodeString(QStringLiteral("Test note"), "us-ascii");
    edit.setMessage(msg);

    QPushButton *openEditor = edit.findChild<QPushButton *>(QStringLiteral("open-editor-button"));
    QPushButton *save = edit.findChild<QPushButton *>(QStringLiteral("save-button"));
    QCOMPARE(openEditor->isEnabled(), false);
    QCOMPARE(save->isEnabled(), false);
}

QTEST_MAIN(EventEditTest)
