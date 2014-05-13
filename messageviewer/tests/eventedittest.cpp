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


#include "eventedittest.h"
#include "widgets/eventedit.h"
#include "messageviewer/globalsettings_base.h"

#include <AkonadiCore/Collection>
#include <AkonadiWidgets/CollectionComboBox>
#include <AkonadiCore/EntityTreeModel>
#include <QStandardItemModel>
#include <KCalCore/Event>
#include <KDateTimeEdit>
#include <KPushButton>

#include <qtest_kde.h>
#include <qtestkeyboard.h>
#include <qtestmouse.h>

#include <QLineEdit>
#include <QHBoxLayout>
#include <QShortcut>
#include <QAction>

namespace MessageViewer {
extern MESSAGEVIEWER_EXPORT QAbstractItemModel *_k_eventEditStubModel;
}


Q_DECLARE_METATYPE(KMime::Message::Ptr)
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
    //QVERIFY(edit.collection().isValid());
    QVERIFY(!edit.message());
    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    QVERIFY(noteedit);
    QCOMPARE(noteedit->text(), QString());

    KPushButton *openEditor = qFindChild<KPushButton *>(&edit, QLatin1String("open-editor-button"));
    KPushButton *save = qFindChild<KPushButton *>(&edit, QLatin1String("save-button"));
    QVERIFY(openEditor);
    QVERIFY(save);
    QCOMPARE(openEditor->isEnabled(), false);
    QCOMPARE(save->isEnabled(), false);

    KDateTime currentDateTime = KDateTime::currentDateTime(KDateTime::LocalZone);
    KDateTimeEdit *startDateTime = qFindChild<KDateTimeEdit *>(&edit, QLatin1String("startdatetimeedit"));
    QVERIFY(startDateTime);
    QCOMPARE(startDateTime->dateTime().date(), currentDateTime.date());
    QCOMPARE(startDateTime->dateTime().time().hour(), currentDateTime.time().hour());
    QCOMPARE(startDateTime->dateTime().time().minute(), currentDateTime.time().minute());

    KDateTimeEdit *endDateTime = qFindChild<KDateTimeEdit *>(&edit, QLatin1String("enddatetimeedit"));
    QVERIFY(endDateTime);
    QCOMPARE(endDateTime->dateTime().date(), currentDateTime.date());
    QCOMPARE(endDateTime->dateTime().time().hour(), currentDateTime.time().hour() + 1);
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
    QString subject = QLatin1String("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    QSignalSpy spy(&edit, SIGNAL(createEvent(KCalCore::Event::Ptr,Akonadi::Collection)));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(spy.count(), 1);
}

void EventEditTest::shouldHideWidgetWhenPressEnter()
{
    MessageViewer::EventEdit edit;
    edit.show();
    QTest::qWaitForWindowShown(&edit);
    QVERIFY(edit.isVisible());

    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QLatin1String("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(edit.isVisible(), false);
}

void EventEditTest::shouldHideWidgetWhenPressEscape()
{
    MessageViewer::EventEdit edit;
    edit.show();
    QTest::qWaitForWindowShown(&edit);
    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    noteedit->setFocus();
    QVERIFY(noteedit->hasFocus());
    QTest::keyPress(&edit, Qt::Key_Escape);
    QCOMPARE(edit.isVisible(), false);
}

void EventEditTest::shouldHideWidgetWhenSaveClicked()
{
    MessageViewer::EventEdit edit;
    edit.show();
    QTest::qWaitForWindowShown(&edit);

    KMime::Message::Ptr msg(new KMime::Message);
    msg->subject(true)->fromUnicodeString(QLatin1String("Test Note"), "us-ascii");
    edit.setMessage(msg);
    KPushButton *save = qFindChild<KPushButton*>(&edit, QLatin1String("save-button"));
    QTest::mouseClick(save, Qt::LeftButton);
    QCOMPARE(edit.isVisible(), false);
}


void EventEditTest::shouldSaveCollectionSettings()
{
    MessageViewer::EventEdit edit;
    Akonadi::CollectionComboBox *akonadicombobox = qFindChild<Akonadi::CollectionComboBox *>(&edit, QLatin1String("akonadicombobox"));
    akonadicombobox->setCurrentIndex(3);
    const Akonadi::Collection::Id id = akonadicombobox->currentCollection().id();
    KPushButton *close = qFindChild<KPushButton *>(&edit, QLatin1String("close-button"));
    QTest::mouseClick(close, Qt::LeftButton);
    QCOMPARE(MessageViewer::GlobalSettingsBase::self()->lastEventSelectedFolder(), id);
}

void EventEditTest::shouldSaveCollectionSettingsWhenCloseWidget()
{
    MessageViewer::EventEdit edit;
    Akonadi::CollectionComboBox *akonadicombobox = qFindChild<Akonadi::CollectionComboBox *>(&edit, QLatin1String("akonadicombobox"));
    akonadicombobox->setCurrentIndex(4);
    const Akonadi::Collection::Id id = akonadicombobox->currentCollection().id();
    edit.writeConfig();
    QCOMPARE(MessageViewer::GlobalSettingsBase::self()->lastEventSelectedFolder(), id);
}

void EventEditTest::shouldSaveCollectionSettingsWhenDeleteWidget()
{
    MessageViewer::EventEdit *edit = new MessageViewer::EventEdit;
    Akonadi::CollectionComboBox *akonadicombobox = qFindChild<Akonadi::CollectionComboBox *>(edit, QLatin1String("akonadicombobox"));
    akonadicombobox->setCurrentIndex(5);
    const Akonadi::Collection::Id id = akonadicombobox->currentCollection().id();
    delete edit;
    QCOMPARE(MessageViewer::GlobalSettingsBase::self()->lastEventSelectedFolder(), id);
}

void EventEditTest::shouldNotEmitCreateEventWhenDateIsInvalid()
{
    MessageViewer::EventEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);

    KDateTimeEdit *startDateTime = qFindChild<KDateTimeEdit *>(&edit, QLatin1String("startdatetimeedit"));
    startDateTime->setDateTime(KDateTime());

    KDateTimeEdit *endDateTime = qFindChild<KDateTimeEdit *>(&edit, QLatin1String("enddatetimeedit"));
    endDateTime->setDateTime(KDateTime());


    QString subject = QLatin1String("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    QSignalSpy spy(&edit, SIGNAL(createEvent(KCalCore::Event::Ptr,Akonadi::Collection)));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(spy.count(), 0);
}

void EventEditTest::shouldEventHasCorrectSubject()
{
    MessageViewer::EventEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QLatin1String("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    QSignalSpy spy(&edit, SIGNAL(createEvent(KCalCore::Event::Ptr,Akonadi::Collection)));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(spy.count(), 1);
    KCalCore::Event::Ptr eventPtr = spy.at(0).at(0).value<KCalCore::Event::Ptr>();
    QVERIFY(eventPtr);
    QCOMPARE(eventPtr->summary(), QString::fromLatin1("Reply to \"%1\"").arg(subject));
}

void EventEditTest::shouldSelectLineWhenPutMessage()
{
    MessageViewer::EventEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QLatin1String("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    QVERIFY(noteedit->hasSelectedText());
    const QString selectedText = noteedit->selectedText();
    QCOMPARE(selectedText, QString::fromLatin1("Reply to \"%1\"").arg(subject));
}

void EventEditTest::shouldHaveCorrectStartEndDateTime()
{
    MessageViewer::EventEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QLatin1String("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);

    KDateTime currentDateTime = KDateTime::currentDateTime(KDateTime::LocalZone);
    KDateTimeEdit *startDateTime = qFindChild<KDateTimeEdit *>(&edit, QLatin1String("startdatetimeedit"));
    startDateTime->setDateTime(currentDateTime);

    KDateTime endDt = currentDateTime.addDays(1);
    KDateTimeEdit *endDateTime = qFindChild<KDateTimeEdit *>(&edit, QLatin1String("enddatetimeedit"));
    endDateTime->setDateTime(endDt);

    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    QSignalSpy spy(&edit, SIGNAL(createEvent(KCalCore::Event::Ptr,Akonadi::Collection)));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(spy.count(), 1);
    KCalCore::Event::Ptr eventPtr = spy.at(0).at(0).value<KCalCore::Event::Ptr>();
    QVERIFY(eventPtr);
    QCOMPARE(eventPtr->dtStart(), currentDateTime);
    QCOMPARE(eventPtr->dtEnd(), endDt);
}

void EventEditTest::shouldSetFocusWhenWeCallTodoEdit()
{
    MessageViewer::EventEdit edit;
    edit.show();
    QTest::qWaitForWindowShown(&edit);
    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    noteedit->setFocus();
    QVERIFY(noteedit->hasFocus());
    edit.setFocus();
    edit.showEventEdit();
    QVERIFY(noteedit->hasFocus());
}

void EventEditTest::shouldEnsureEndDateIsNotBeforeStartDate()
{
    MessageViewer::EventEdit edit;
    KDateTimeEdit *startDateTime = qFindChild<KDateTimeEdit *>(&edit, QLatin1String("startdatetimeedit"));
    KDateTimeEdit *endDateTime = qFindChild<KDateTimeEdit *>(&edit, QLatin1String("enddatetimeedit"));

    KDateTime startDt = startDateTime->dateTime();
    QVERIFY(startDt < endDateTime->dateTime());

    startDt = startDt.addDays(1);
    startDateTime->setDateTime(startDt);
    QCOMPARE(startDt.date(), endDateTime->date());
    QVERIFY(startDt.time() < endDateTime->time());

    startDt = startDt.addSecs(2*3600);
    startDateTime->setDateTime(startDt);
    QCOMPARE(startDt.time(), endDateTime->time());
}

void EventEditTest::shouldShouldEnabledSaveOpenEditorButton()
{
    MessageViewer::EventEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    msg->subject(true)->fromUnicodeString(QLatin1String("Test note"), "us-ascii");
    edit.setMessage(msg);

    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    KPushButton *openEditor = qFindChild<KPushButton *>(&edit, QLatin1String("open-editor-button"));
    KPushButton *save = qFindChild<KPushButton *>(&edit, QLatin1String("save-button"));
    QCOMPARE(openEditor->isEnabled(), true);
    QCOMPARE(save->isEnabled(), true);
    noteedit->clear();

    QCOMPARE(openEditor->isEnabled(), false);
    QCOMPARE(save->isEnabled(), false);
}


QTEST_KDEMAIN( EventEditTest, GUI )
