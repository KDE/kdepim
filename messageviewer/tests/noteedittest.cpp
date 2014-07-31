/*
  Copyright (c) 2014 Sandro Knauß <knauss@kolabsys.com>

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

#include "noteedittest.h"
#include "messageviewer/globalsettings_base.h"
#include "widgets/noteedit.h"
#include <Akonadi/Collection>
#include <Akonadi/CollectionComboBox>
#include <Akonadi/EntityTreeModel>

#include <KMime/KMimeMessage>
#include <QStandardItemModel>
#include <KPushButton>
#include <KMessageWidget>
#include <qtest_kde.h>
#include <qtestkeyboard.h>
#include <qtestmouse.h>

#include <QLineEdit>
#include <QHBoxLayout>
#include <QShortcut>
#include <QAction>

namespace MessageViewer {
extern MESSAGEVIEWER_EXPORT QAbstractItemModel *_k_noteEditStubModel;
}


Q_DECLARE_METATYPE(KMime::Message::Ptr)
NoteEditTest::NoteEditTest()
{
    qRegisterMetaType<Akonadi::Collection>();
    qRegisterMetaType<KMime::Message::Ptr>();

    QStandardItemModel *model = new QStandardItemModel;
    for (int id = 42; id < 51; ++id) {
        Akonadi::Collection collection(id);
        collection.setRights(Akonadi::Collection::AllRights);
        collection.setName(QString::number(id));
        collection.setContentMimeTypes(QStringList() << Akonadi::NoteUtils::noteMimeType());

        QStandardItem *item = new QStandardItem(collection.name());
        item->setData(QVariant::fromValue(collection),
                      Akonadi::EntityTreeModel::CollectionRole);
        item->setData(QVariant::fromValue(collection.id()),
                      Akonadi::EntityTreeModel::CollectionIdRole);

        model->appendRow(item);
    }
    MessageViewer::_k_noteEditStubModel = model;
}

void NoteEditTest::shouldHaveDefaultValuesOnCreation()
{
    MessageViewer::NoteEdit edit;
    //We can't test if because it loads from settings and in Jenkins it doesn't exist but here it exists
    //QVERIFY(edit.collection().isValid());
    QVERIFY(!edit.message());
    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    KPushButton *save = qFindChild<KPushButton *>(&edit, QLatin1String("save-button"));
    QVERIFY(save);
    QCOMPARE(save->isEnabled(), false);
    QVERIFY(noteedit);
    QCOMPARE(noteedit->text(), QString());
}

void NoteEditTest::shouldEmitCollectionChanged()
{
    MessageViewer::NoteEdit edit;
    QSignalSpy spy(&edit, SIGNAL(collectionChanged(Akonadi::Collection)));
    edit.setCollection(Akonadi::Collection(42));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).value<Akonadi::Collection>(), Akonadi::Collection(42));
}

void NoteEditTest::shouldEmitMessageChanged()
{
    MessageViewer::NoteEdit edit;
    QSignalSpy spy(&edit, SIGNAL(messageChanged(KMime::Message::Ptr)));
    KMime::Message::Ptr msg(new KMime::Message);
    edit.setMessage(msg);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).value<KMime::Message::Ptr>(), msg);
}

void NoteEditTest::shouldNotEmitWhenCollectionIsNotChanged()
{
    MessageViewer::NoteEdit edit;
    edit.setCollection(Akonadi::Collection(42));
    QSignalSpy spy(&edit, SIGNAL(collectionChanged(Akonadi::Collection)));
    edit.setCollection(Akonadi::Collection(42));
    QCOMPARE(spy.count(), 0);
}


void NoteEditTest::shouldNotEmitWhenMessageIsNotChanged()
{
    MessageViewer::NoteEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    edit.setMessage(msg);
    QSignalSpy spy(&edit, SIGNAL(messageChanged(KMime::Message::Ptr)));
    edit.setMessage(msg);
    QCOMPARE(spy.count(), 0);
}

void NoteEditTest::shouldHaveSameValueAfterSet()
{
    MessageViewer::NoteEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    edit.setCollection(Akonadi::Collection(42));
    edit.setMessage(msg);
    QCOMPARE(edit.collection(), Akonadi::Collection(42));
    QCOMPARE(edit.message(), msg);
}

void NoteEditTest::shouldHaveASubject()
{
    MessageViewer::NoteEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);

    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    QVERIFY(noteedit);
    QCOMPARE(noteedit->text(), QString());

    QString subject = QLatin1String("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);

    QCOMPARE(noteedit->text(), subject);
}

void NoteEditTest::shouldEmptySubjectWhenMessageIsNull()
{
    MessageViewer::NoteEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QLatin1String("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    edit.setMessage(KMime::Message::Ptr());
    QCOMPARE(noteedit->text(), QString());
}

void NoteEditTest::shouldEmptySubjectWhenMessageHasNotSubject()
{
    MessageViewer::NoteEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QLatin1String("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    KMime::Message::Ptr msgSubjectEmpty(new KMime::Message);
    edit.setMessage(msgSubjectEmpty);
    QCOMPARE(noteedit->text(), QString());
}

void NoteEditTest::shouldSelectLineWhenPutMessage()
{
    MessageViewer::NoteEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QLatin1String("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    QVERIFY(noteedit->hasSelectedText());
    const QString selectedText = noteedit->selectedText();
    QCOMPARE(selectedText, subject);
}

void NoteEditTest::shouldEmitCollectionChangedWhenChangeComboboxItem()
{
    MessageViewer::NoteEdit edit;
    Akonadi::CollectionComboBox *akonadicombobox = qFindChild<Akonadi::CollectionComboBox *>(&edit, QLatin1String("akonadicombobox"));
    QVERIFY(akonadicombobox);
    QVERIFY(akonadicombobox->currentCollection().isValid());
}

void NoteEditTest::shouldEmitNotEmitNoteWhenTextIsEmpty()
{
    MessageViewer::NoteEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    edit.setMessage(msg);
    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    QSignalSpy spy(&edit, SIGNAL(createNote(KMime::Message::Ptr,Akonadi::Collection)));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(spy.count(), 0);
}

void NoteEditTest::shouldEmitNotEmitNoteWhenTextTrimmedIsEmpty()
{
    MessageViewer::NoteEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    edit.setMessage(msg);
    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    QSignalSpy spy(&edit, SIGNAL(createNote(KMime::Message::Ptr,Akonadi::Collection)));
    noteedit->setText(QLatin1String("      "));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(spy.count(), 0);

    noteedit->setText(QLatin1String("      F"));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(spy.count(), 1);
}

void NoteEditTest::shouldEmitNoteWhenPressEnter()
{
    MessageViewer::NoteEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QLatin1String("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    QSignalSpy spy(&edit, SIGNAL(createNote(KMime::Message::Ptr,Akonadi::Collection)));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(spy.count(), 1);
}

void NoteEditTest::shouldNoteHasCorrectSubject()
{
    MessageViewer::NoteEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QLatin1String("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    QSignalSpy spy(&edit, SIGNAL(createNote(KMime::Message::Ptr,Akonadi::Collection)));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(spy.count(), 1);
    KMime::Message::Ptr notePtr = spy.at(0).at(0).value<KMime::Message::Ptr>();
    QVERIFY(notePtr);
    Akonadi::NoteUtils::NoteMessageWrapper note(notePtr);
    QCOMPARE(note.title(), subject);
}

void NoteEditTest::shouldClearAllWhenCloseWidget()
{
    MessageViewer::NoteEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QLatin1String("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    edit.slotCloseWidget();
    QCOMPARE(noteedit->text(), QString());
    QVERIFY(!edit.message());
}

void NoteEditTest::shouldEmitCollectionChangedWhenCurrentCollectionWasChanged()
{
    MessageViewer::NoteEdit edit;
    Akonadi::CollectionComboBox *akonadicombobox = qFindChild<Akonadi::CollectionComboBox *>(&edit, QLatin1String("akonadicombobox"));
    akonadicombobox->setCurrentIndex(0);
    QCOMPARE(akonadicombobox->currentIndex(), 0);
    QSignalSpy spy(&edit, SIGNAL(collectionChanged(Akonadi::Collection)));
    akonadicombobox->setCurrentIndex(3);
    QCOMPARE(akonadicombobox->currentIndex(), 3);
    QCOMPARE(spy.count(), 1);
}

void NoteEditTest::shouldEmitCorrectCollection()
{
    MessageViewer::NoteEdit edit;
    Akonadi::CollectionComboBox *akonadicombobox = qFindChild<Akonadi::CollectionComboBox *>(&edit, QLatin1String("akonadicombobox"));
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QLatin1String("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    akonadicombobox->setCurrentIndex(3);
    Akonadi::Collection col = akonadicombobox->currentCollection();
    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    QSignalSpy spy(&edit, SIGNAL(createNote(KMime::Message::Ptr,Akonadi::Collection)));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(1).value<Akonadi::Collection>(), col);
}

void NoteEditTest::shouldHideWidgetWhenClickOnCloseButton()
{
    MessageViewer::NoteEdit edit;
    edit.show();
    QTest::qWaitForWindowShown(&edit);
    QVERIFY(edit.isVisible());
    KPushButton *close = qFindChild<KPushButton *>(&edit, QLatin1String("close-button"));
    QTest::mouseClick(close, Qt::LeftButton);
    QCOMPARE(edit.isVisible(), false);
}

void NoteEditTest::shouldHideWidgetWhenPressEscape()
{
    MessageViewer::NoteEdit edit;
    edit.show();
    QTest::qWaitForWindowShown(&edit);
    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    noteedit->setFocus();
    QVERIFY(noteedit->hasFocus());
    QTest::keyPress(&edit, Qt::Key_Escape);
    QCOMPARE(edit.isVisible(), false);
}

void NoteEditTest::shouldHideWidgetWhenSaveClicked()
{
    MessageViewer::NoteEdit edit;
    edit.show();
    QTest::qWaitForWindowShown(&edit);

    KMime::Message::Ptr msg(new KMime::Message);
    msg->subject(true)->fromUnicodeString(QLatin1String("Test Note"), "us-ascii");
    edit.setMessage(msg);
    KPushButton *save = qFindChild<KPushButton *>(&edit, QLatin1String("save-button"));
    QTest::mouseClick(save, Qt::LeftButton);
    QCOMPARE(edit.isVisible(), false);
}

void NoteEditTest::shouldSaveCollectionSettings()
{
    MessageViewer::NoteEdit edit;
    Akonadi::CollectionComboBox *akonadicombobox = qFindChild<Akonadi::CollectionComboBox *>(&edit, QLatin1String("akonadicombobox"));
    akonadicombobox->setCurrentIndex(3);
    const Akonadi::Collection::Id id = akonadicombobox->currentCollection().id();
    KPushButton *close = qFindChild<KPushButton *>(&edit, QLatin1String("close-button"));
    QTest::mouseClick(close, Qt::LeftButton);
    QCOMPARE(MessageViewer::GlobalSettingsBase::self()->lastNoteSelectedFolder(), id);
}

void NoteEditTest::shouldSaveCollectionSettingsWhenCloseWidget()
{
    MessageViewer::NoteEdit edit;
    Akonadi::CollectionComboBox *akonadicombobox = qFindChild<Akonadi::CollectionComboBox *>(&edit, QLatin1String("akonadicombobox"));
    akonadicombobox->setCurrentIndex(4);
    const Akonadi::Collection::Id id = akonadicombobox->currentCollection().id();
    edit.writeConfig();
    QCOMPARE(MessageViewer::GlobalSettingsBase::self()->lastNoteSelectedFolder(), id);
}

void NoteEditTest::shouldSaveCollectionSettingsWhenDeleteWidget()
{
    MessageViewer::NoteEdit *edit = new MessageViewer::NoteEdit;
    Akonadi::CollectionComboBox *akonadicombobox = qFindChild<Akonadi::CollectionComboBox *>(edit, QLatin1String("akonadicombobox"));
    akonadicombobox->setCurrentIndex(4);
    const Akonadi::Collection::Id id = akonadicombobox->currentCollection().id();
    delete edit;
    QCOMPARE(MessageViewer::GlobalSettingsBase::self()->lastNoteSelectedFolder(), id);
}

void NoteEditTest::shouldSetFocusWhenWeCallNoteEdit()
{
    MessageViewer::NoteEdit edit;
    edit.show();
    QTest::qWaitForWindowShown(&edit);
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QLatin1String("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    QCOMPARE(noteedit->hasFocus(), true);
    edit.setFocus();
    QCOMPARE(noteedit->hasFocus(), false);
    edit.showNoteEdit();
    QCOMPARE(noteedit->hasFocus(), true);
}


void NoteEditTest::shouldNotEmitNoteWhenMessageIsNull()
{
    MessageViewer::NoteEdit edit;
    QString subject = QLatin1String("Test Note");
    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    noteedit->setText(subject);
    QSignalSpy spy(&edit, SIGNAL(createNote(KMime::Message::Ptr,Akonadi::Collection)));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(spy.count(), 0);
}

void NoteEditTest::shouldClearLineAfterEmitNewNote()
{
    MessageViewer::NoteEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QLatin1String("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(noteedit->text(), QString());
}

void NoteEditTest::shouldHaveLineEditFocus()
{
    MessageViewer::NoteEdit edit;
    edit.show();
    QTest::qWaitForWindowShown(&edit);
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QLatin1String("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    QCOMPARE(noteedit->hasFocus(), true);
}

void NoteEditTest::shouldShouldEnabledSaveEditorButton()
{
    MessageViewer::NoteEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    msg->subject(true)->fromUnicodeString(QLatin1String("Test note"), "us-ascii");
    edit.setMessage(msg);

    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    KPushButton *save = qFindChild<KPushButton *>(&edit, QLatin1String("save-button"));
    QCOMPARE(save->isEnabled(), true);
    noteedit->clear();

    QCOMPARE(save->isEnabled(), false);
}



QTEST_KDEMAIN( NoteEditTest, GUI )
