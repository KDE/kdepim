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
#include "globalsettings_messageviewer.h"
#include "../createnoteplugin/noteedit.h"
#include <AkonadiCore/Collection>
#include <AkonadiWidgets/CollectionComboBox>
#include <AkonadiCore/EntityTreeModel>
#include <KMime/KMimeMessage>
#include <KMessageWidget>

#include <QAction>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QShortcut>
#include <QStandardItemModel>
#include <QSignalSpy>
#include <QTest>

namespace MessageViewer
{
extern MESSAGEVIEWER_EXPORT QAbstractItemModel *_k_noteEditStubModel;
}

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
    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("noteedit"));
    QPushButton *save = edit.findChild<QPushButton *>(QStringLiteral("save-button"));
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

    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("noteedit"));
    QVERIFY(noteedit);
    QCOMPARE(noteedit->text(), QString());

    QString subject = QStringLiteral("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);

    QCOMPARE(noteedit->text(), subject);
}

void NoteEditTest::shouldEmptySubjectWhenMessageIsNull()
{
    MessageViewer::NoteEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QStringLiteral("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("noteedit"));
    edit.setMessage(KMime::Message::Ptr());
    QCOMPARE(noteedit->text(), QString());
}

void NoteEditTest::shouldEmptySubjectWhenMessageHasNotSubject()
{
    MessageViewer::NoteEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QStringLiteral("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("noteedit"));
    KMime::Message::Ptr msgSubjectEmpty(new KMime::Message);
    edit.setMessage(msgSubjectEmpty);
    QCOMPARE(noteedit->text(), QString());
}

void NoteEditTest::shouldSelectLineWhenPutMessage()
{
    MessageViewer::NoteEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QStringLiteral("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("noteedit"));
    QVERIFY(noteedit->hasSelectedText());
    const QString selectedText = noteedit->selectedText();
    QCOMPARE(selectedText, subject);
}

void NoteEditTest::shouldEmitCollectionChangedWhenChangeComboboxItem()
{
    MessageViewer::NoteEdit edit;
    Akonadi::CollectionComboBox *akonadicombobox = edit.findChild<Akonadi::CollectionComboBox *>(QStringLiteral("akonadicombobox"));
    QVERIFY(akonadicombobox);
    QVERIFY(akonadicombobox->currentCollection().isValid());
}

void NoteEditTest::shouldEmitNotEmitNoteWhenTextIsEmpty()
{
    MessageViewer::NoteEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    edit.setMessage(msg);
    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("noteedit"));
    QSignalSpy spy(&edit, SIGNAL(createNote(KMime::Message::Ptr,Akonadi::Collection)));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(spy.count(), 0);
}

void NoteEditTest::shouldEmitNotEmitNoteWhenTextTrimmedIsEmpty()
{
    MessageViewer::NoteEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    edit.setMessage(msg);
    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("noteedit"));
    QSignalSpy spy(&edit, SIGNAL(createNote(KMime::Message::Ptr,Akonadi::Collection)));
    noteedit->setText(QStringLiteral("      "));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(spy.count(), 0);

    noteedit->setText(QStringLiteral("      F"));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(spy.count(), 1);
}

void NoteEditTest::shouldEmitNoteWhenPressEnter()
{
    MessageViewer::NoteEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QStringLiteral("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("noteedit"));
    QSignalSpy spy(&edit, SIGNAL(createNote(KMime::Message::Ptr,Akonadi::Collection)));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(spy.count(), 1);
}

void NoteEditTest::shouldNoteHasCorrectSubject()
{
    MessageViewer::NoteEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QStringLiteral("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("noteedit"));
    QSignalSpy spy(&edit, SIGNAL(createNote(KMime::Message::Ptr,Akonadi::Collection)));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(spy.count(), 1);
    KMime::Message::Ptr notePtr = spy.at(0).at(0).value<KMime::Message::Ptr>();
    QVERIFY((bool)notePtr);
    Akonadi::NoteUtils::NoteMessageWrapper note(notePtr);
    QCOMPARE(note.title(), subject);
}

void NoteEditTest::shouldClearAllWhenCloseWidget()
{
    MessageViewer::NoteEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QStringLiteral("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("noteedit"));
    edit.slotCloseWidget();
    QCOMPARE(noteedit->text(), QString());
    QVERIFY(!edit.message());
}

void NoteEditTest::shouldEmitCollectionChangedWhenCurrentCollectionWasChanged()
{
    MessageViewer::NoteEdit edit;
    Akonadi::CollectionComboBox *akonadicombobox = edit.findChild<Akonadi::CollectionComboBox *>(QStringLiteral("akonadicombobox"));
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
    Akonadi::CollectionComboBox *akonadicombobox = edit.findChild<Akonadi::CollectionComboBox *>(QStringLiteral("akonadicombobox"));
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QStringLiteral("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    akonadicombobox->setCurrentIndex(3);
    Akonadi::Collection col = akonadicombobox->currentCollection();
    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("noteedit"));
    QSignalSpy spy(&edit, SIGNAL(createNote(KMime::Message::Ptr,Akonadi::Collection)));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(1).value<Akonadi::Collection>(), col);
}

void NoteEditTest::shouldHideWidgetWhenClickOnCloseButton()
{
    MessageViewer::NoteEdit edit;
    edit.show();
    QTest::qWaitForWindowExposed(&edit);
    QVERIFY(edit.isVisible());
    QPushButton *close = edit.findChild<QPushButton *>(QStringLiteral("close-button"));
    QTest::mouseClick(close, Qt::LeftButton);
    QCOMPARE(edit.isVisible(), false);
}

void NoteEditTest::shouldHideWidgetWhenPressEscape()
{
    MessageViewer::NoteEdit edit;
    edit.show();
    QTest::qWaitForWindowExposed(&edit);
    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("noteedit"));
    noteedit->setFocus();
    QVERIFY(noteedit->hasFocus());
    QTest::keyPress(&edit, Qt::Key_Escape);
    QCOMPARE(edit.isVisible(), false);
}

void NoteEditTest::shouldHideWidgetWhenSaveClicked()
{
    MessageViewer::NoteEdit edit;
    edit.show();
    QTest::qWaitForWindowExposed(&edit);

    KMime::Message::Ptr msg(new KMime::Message);
    msg->subject(true)->fromUnicodeString(QStringLiteral("Test Note"), "us-ascii");
    edit.setMessage(msg);
    QPushButton *save = edit.findChild<QPushButton *>(QStringLiteral("save-button"));
    QTest::mouseClick(save, Qt::LeftButton);
    QCOMPARE(edit.isVisible(), false);
}

void NoteEditTest::shouldSaveCollectionSettings()
{
    MessageViewer::NoteEdit edit;
    Akonadi::CollectionComboBox *akonadicombobox = edit.findChild<Akonadi::CollectionComboBox *>(QStringLiteral("akonadicombobox"));
    akonadicombobox->setCurrentIndex(3);
    const Akonadi::Collection::Id id = akonadicombobox->currentCollection().id();
    QPushButton *close = edit.findChild<QPushButton *>(QStringLiteral("close-button"));
    QTest::mouseClick(close, Qt::LeftButton);
    QCOMPARE(MessageViewer::MessageViewerSettingsBase::self()->lastNoteSelectedFolder(), id);
}

void NoteEditTest::shouldSaveCollectionSettingsWhenCloseWidget()
{
    MessageViewer::NoteEdit edit;
    Akonadi::CollectionComboBox *akonadicombobox = edit.findChild<Akonadi::CollectionComboBox *>(QStringLiteral("akonadicombobox"));
    akonadicombobox->setCurrentIndex(4);
    const Akonadi::Collection::Id id = akonadicombobox->currentCollection().id();
    edit.writeConfig();
    QCOMPARE(MessageViewer::MessageViewerSettingsBase::self()->lastNoteSelectedFolder(), id);
}

void NoteEditTest::shouldSaveCollectionSettingsWhenDeleteWidget()
{
    MessageViewer::NoteEdit *edit = new MessageViewer::NoteEdit;
    Akonadi::CollectionComboBox *akonadicombobox = edit->findChild<Akonadi::CollectionComboBox *>(QStringLiteral("akonadicombobox"));
    akonadicombobox->setCurrentIndex(4);
    const Akonadi::Collection::Id id = akonadicombobox->currentCollection().id();
    delete edit;
    QCOMPARE(MessageViewer::MessageViewerSettingsBase::self()->lastNoteSelectedFolder(), id);
}

void NoteEditTest::shouldSetFocusWhenWeCallNoteEdit()
{
    MessageViewer::NoteEdit edit;
    edit.show();
    QTest::qWaitForWindowExposed(&edit);
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QStringLiteral("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("noteedit"));
    QCOMPARE(noteedit->hasFocus(), true);
    edit.setFocus();
    QCOMPARE(noteedit->hasFocus(), false);
    edit.showNoteEdit();
    QCOMPARE(noteedit->hasFocus(), true);
}

void NoteEditTest::shouldNotEmitNoteWhenMessageIsNull()
{
    MessageViewer::NoteEdit edit;
    QString subject = QStringLiteral("Test Note");
    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("noteedit"));
    noteedit->setText(subject);
    QSignalSpy spy(&edit, SIGNAL(createNote(KMime::Message::Ptr,Akonadi::Collection)));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(spy.count(), 0);
}

void NoteEditTest::shouldClearLineAfterEmitNewNote()
{
    MessageViewer::NoteEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QStringLiteral("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("noteedit"));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(noteedit->text(), QString());
}

void NoteEditTest::shouldHaveLineEditFocus()
{
    MessageViewer::NoteEdit edit;
    edit.show();
    QTest::qWaitForWindowExposed(&edit);
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QStringLiteral("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("noteedit"));
    QCOMPARE(noteedit->hasFocus(), true);
}

void NoteEditTest::shouldShouldEnabledSaveEditorButton()
{
    MessageViewer::NoteEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    msg->subject(true)->fromUnicodeString(QStringLiteral("Test note"), "us-ascii");
    edit.setMessage(msg);

    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("noteedit"));
    QPushButton *save = edit.findChild<QPushButton *>(QStringLiteral("save-button"));
    QCOMPARE(save->isEnabled(), true);
    noteedit->clear();

    QCOMPARE(save->isEnabled(), false);
    QVERIFY(noteedit->text().isEmpty());

    noteedit->setText(QStringLiteral("  "));
    QCOMPARE(save->isEnabled(), false);
}

QTEST_MAIN(NoteEditTest)
