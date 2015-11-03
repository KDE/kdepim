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

#include "todoedittest.h"
#include "globalsettings_messageviewer.h"
#include "../createtodoplugin/todoedit.h"
#include <AkonadiCore/Collection>
#include <AkonadiWidgets/CollectionComboBox>
#include <AkonadiCore/EntityTreeModel>
#include <QStandardItemModel>
#include <QPushButton>
#include <KMessageWidget>
#include <qtest.h>
#include <qtestkeyboard.h>
#include <qtestmouse.h>

#include <QLineEdit>
#include <QHBoxLayout>
#include <QShortcut>
#include <QAction>
#include <QSignalSpy>

namespace MessageViewer
{
extern MESSAGEVIEWER_EXPORT QAbstractItemModel *_k_todoEditStubModel;
}

TodoEditTest::TodoEditTest()
{
    qRegisterMetaType<Akonadi::Collection>();
    qRegisterMetaType<KMime::Message::Ptr>();
    qRegisterMetaType<KCalCore::Todo::Ptr>();

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
    MessageViewer::_k_todoEditStubModel = model;
}

void TodoEditTest::shouldHaveDefaultValuesOnCreation()
{
    MessageViewer::TodoEdit edit;
    //We can't test if because it loads from settings and in Jenkins it doesn't exist but here it exists
    //QVERIFY(edit.collection().isValid());
    QVERIFY(!edit.message());
    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("noteedit"));
    QPushButton *openEditor = edit.findChild<QPushButton *>(QStringLiteral("open-editor-button"));
    QPushButton *save = edit.findChild<QPushButton *>(QStringLiteral("save-button"));
    QVERIFY(openEditor);
    QVERIFY(save);
    QCOMPARE(openEditor->isEnabled(), false);
    QCOMPARE(save->isEnabled(), false);
    QVERIFY(noteedit);
    QCOMPARE(noteedit->text(), QString());
}

void TodoEditTest::shouldEmitCollectionChanged()
{
    MessageViewer::TodoEdit edit;
    QSignalSpy spy(&edit, SIGNAL(collectionChanged(Akonadi::Collection)));
    edit.setCollection(Akonadi::Collection(42));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).value<Akonadi::Collection>(), Akonadi::Collection(42));
}

void TodoEditTest::shouldEmitMessageChanged()
{
    MessageViewer::TodoEdit edit;
    QSignalSpy spy(&edit, SIGNAL(messageChanged(KMime::Message::Ptr)));
    KMime::Message::Ptr msg(new KMime::Message);
    edit.setMessage(msg);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).value<KMime::Message::Ptr>(), msg);
}

void TodoEditTest::shouldNotEmitWhenCollectionIsNotChanged()
{
    MessageViewer::TodoEdit edit;
    edit.setCollection(Akonadi::Collection(42));
    QSignalSpy spy(&edit, SIGNAL(collectionChanged(Akonadi::Collection)));
    edit.setCollection(Akonadi::Collection(42));
    QCOMPARE(spy.count(), 0);
}

void TodoEditTest::shouldNotEmitWhenMessageIsNotChanged()
{
    MessageViewer::TodoEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    edit.setMessage(msg);
    QSignalSpy spy(&edit, SIGNAL(messageChanged(KMime::Message::Ptr)));
    edit.setMessage(msg);
    QCOMPARE(spy.count(), 0);
}

void TodoEditTest::shouldHaveSameValueAfterSet()
{
    MessageViewer::TodoEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    edit.setCollection(Akonadi::Collection(42));
    edit.setMessage(msg);
    QCOMPARE(edit.collection(), Akonadi::Collection(42));
    QCOMPARE(edit.message(), msg);
}

void TodoEditTest::shouldHaveASubject()
{
    MessageViewer::TodoEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);

    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("noteedit"));
    QVERIFY(noteedit);
    QCOMPARE(noteedit->text(), QString());

    QString subject = QStringLiteral("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);

    QCOMPARE(noteedit->text(), QStringLiteral("Reply to \"%1\"").arg(subject));
}

void TodoEditTest::shouldEmptySubjectWhenMessageIsNull()
{
    MessageViewer::TodoEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QStringLiteral("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("noteedit"));
    edit.setMessage(KMime::Message::Ptr());
    QCOMPARE(noteedit->text(), QString());
}

void TodoEditTest::shouldEmptySubjectWhenMessageHasNotSubject()
{
    MessageViewer::TodoEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QStringLiteral("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("noteedit"));
    KMime::Message::Ptr msgSubjectEmpty(new KMime::Message);
    edit.setMessage(msgSubjectEmpty);
    QCOMPARE(noteedit->text(), QString());
}

void TodoEditTest::shouldSelectLineWhenPutMessage()
{
    MessageViewer::TodoEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QStringLiteral("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("noteedit"));
    QVERIFY(noteedit->hasSelectedText());
    const QString selectedText = noteedit->selectedText();
    QCOMPARE(selectedText, QStringLiteral("Reply to \"%1\"").arg(subject));
}

void TodoEditTest::shouldEmitCollectionChangedWhenChangeComboboxItem()
{
    MessageViewer::TodoEdit edit;
    Akonadi::CollectionComboBox *akonadicombobox = edit.findChild<Akonadi::CollectionComboBox *>(QStringLiteral("akonadicombobox"));
    QVERIFY(akonadicombobox);
    QVERIFY(akonadicombobox->currentCollection().isValid());
}

void TodoEditTest::shouldEmitNotEmitTodoWhenTextIsEmpty()
{
    MessageViewer::TodoEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    edit.setMessage(msg);
    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("noteedit"));
    QSignalSpy spy(&edit, SIGNAL(createTodo(KCalCore::Todo::Ptr,Akonadi::Collection)));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(spy.count(), 0);
}

void TodoEditTest::shouldEmitNotEmitTodoWhenTextTrimmedIsEmpty()
{
    MessageViewer::TodoEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    edit.setMessage(msg);
    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("noteedit"));
    QSignalSpy spy(&edit, SIGNAL(createTodo(KCalCore::Todo::Ptr,Akonadi::Collection)));
    noteedit->setText(QStringLiteral("      "));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(spy.count(), 0);

    noteedit->setText(QStringLiteral("      F"));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(spy.count(), 1);
}

void TodoEditTest::shouldEmitTodoWhenPressEnter()
{
    MessageViewer::TodoEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QStringLiteral("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("noteedit"));
    QSignalSpy spy(&edit, SIGNAL(createTodo(KCalCore::Todo::Ptr,Akonadi::Collection)));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(spy.count(), 1);
}

void TodoEditTest::shouldTodoHasCorrectSubject()
{
    MessageViewer::TodoEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QStringLiteral("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("noteedit"));
    QSignalSpy spy(&edit, SIGNAL(createTodo(KCalCore::Todo::Ptr,Akonadi::Collection)));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(spy.count(), 1);
    KCalCore::Todo::Ptr todoPtr = spy.at(0).at(0).value<KCalCore::Todo::Ptr>();
    QVERIFY(todoPtr);
    QCOMPARE(todoPtr->summary(), QStringLiteral("Reply to \"%1\"").arg(subject));
}

void TodoEditTest::shouldClearAllWhenCloseWidget()
{
    MessageViewer::TodoEdit edit;
    edit.show();
    QTest::qWaitForWindowExposed(&edit);

    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QStringLiteral("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);

    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("noteedit"));
    edit.slotCloseWidget();
    QCOMPARE(noteedit->text(), QString());
    QVERIFY(!edit.message());
}

void TodoEditTest::shouldEmitCollectionChangedWhenCurrentCollectionWasChanged()
{
    MessageViewer::TodoEdit edit;
    Akonadi::CollectionComboBox *akonadicombobox = edit.findChild<Akonadi::CollectionComboBox *>(QStringLiteral("akonadicombobox"));
    akonadicombobox->setCurrentIndex(0);
    QCOMPARE(akonadicombobox->currentIndex(), 0);
    QSignalSpy spy(&edit, SIGNAL(collectionChanged(Akonadi::Collection)));
    akonadicombobox->setCurrentIndex(3);
    QCOMPARE(akonadicombobox->currentIndex(), 3);
    QCOMPARE(spy.count(), 1);
}

void TodoEditTest::shouldEmitCorrectCollection()
{
    MessageViewer::TodoEdit edit;
    Akonadi::CollectionComboBox *akonadicombobox = edit.findChild<Akonadi::CollectionComboBox *>(QStringLiteral("akonadicombobox"));
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QStringLiteral("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    akonadicombobox->setCurrentIndex(3);
    Akonadi::Collection col = akonadicombobox->currentCollection();
    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("noteedit"));
    QSignalSpy spy(&edit, SIGNAL(createTodo(KCalCore::Todo::Ptr,Akonadi::Collection)));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(1).value<Akonadi::Collection>(), col);
}

void TodoEditTest::shouldHideWidgetWhenClickOnCloseButton()
{
    MessageViewer::TodoEdit edit;
    edit.show();
    QTest::qWaitForWindowExposed(&edit);
    QVERIFY(edit.isVisible());
    QPushButton *close = edit.findChild<QPushButton *>(QStringLiteral("close-button"));
    QTest::mouseClick(close, Qt::LeftButton);
    QCOMPARE(edit.isVisible(), false);
}

void TodoEditTest::shouldHideWidgetWhenPressEscape()
{
    MessageViewer::TodoEdit edit;
    edit.show();
    QTest::qWaitForWindowExposed(&edit);
    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("noteedit"));
    noteedit->setFocus();
    QVERIFY(noteedit->hasFocus());
    QTest::keyPress(&edit, Qt::Key_Escape);
    QCOMPARE(edit.isVisible(), false);
}

void TodoEditTest::shouldHideWidgetWhenSaveClicked()
{
    MessageViewer::TodoEdit edit;
    edit.show();
    QTest::qWaitForWindowExposed(&edit);

    KMime::Message::Ptr msg(new KMime::Message);
    msg->subject(true)->fromUnicodeString(QStringLiteral("Test Note"), "us-ascii");
    edit.setMessage(msg);
    QPushButton *save = edit.findChild<QPushButton *>(QStringLiteral("save-button"));
    QTest::mouseClick(save, Qt::LeftButton);
    QCOMPARE(edit.isVisible(), true);
}

void TodoEditTest::shouldSaveCollectionSettings()
{
    MessageViewer::TodoEdit edit;
    Akonadi::CollectionComboBox *akonadicombobox = edit.findChild<Akonadi::CollectionComboBox *>(QStringLiteral("akonadicombobox"));
    akonadicombobox->setCurrentIndex(3);
    const Akonadi::Collection::Id id = akonadicombobox->currentCollection().id();
    QPushButton *close = edit.findChild<QPushButton *>(QStringLiteral("close-button"));
    QTest::mouseClick(close, Qt::LeftButton);
    QCOMPARE(MessageViewer::MessageViewerSettingsBase::self()->lastSelectedFolder(), id);
}

void TodoEditTest::shouldSaveCollectionSettingsWhenCloseWidget()
{
    MessageViewer::TodoEdit edit;
    Akonadi::CollectionComboBox *akonadicombobox = edit.findChild<Akonadi::CollectionComboBox *>(QStringLiteral("akonadicombobox"));
    akonadicombobox->setCurrentIndex(4);
    const Akonadi::Collection::Id id = akonadicombobox->currentCollection().id();
    edit.writeConfig();
    QCOMPARE(MessageViewer::MessageViewerSettingsBase::self()->lastSelectedFolder(), id);
}

void TodoEditTest::shouldSaveCollectionSettingsWhenDeleteWidget()
{
    MessageViewer::TodoEdit *edit = new MessageViewer::TodoEdit;
    Akonadi::CollectionComboBox *akonadicombobox = edit->findChild<Akonadi::CollectionComboBox *>(QStringLiteral("akonadicombobox"));
    akonadicombobox->setCurrentIndex(4);
    const Akonadi::Collection::Id id = akonadicombobox->currentCollection().id();
    delete edit;
    QCOMPARE(MessageViewer::MessageViewerSettingsBase::self()->lastSelectedFolder(), id);
}

void TodoEditTest::shouldSetFocusWhenWeCallTodoEdit()
{
    MessageViewer::TodoEdit edit;
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
    edit.showToDoWidget();
    QCOMPARE(noteedit->hasFocus(), true);
}

void TodoEditTest::shouldNotEmitTodoWhenMessageIsNull()
{
    MessageViewer::TodoEdit edit;
    QString subject = QStringLiteral("Test Note");
    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("noteedit"));
    noteedit->setText(subject);
    QSignalSpy spy(&edit, SIGNAL(createTodo(KCalCore::Todo::Ptr,Akonadi::Collection)));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(spy.count(), 0);
}

void TodoEditTest::shouldClearLineAfterEmitNewNote()
{
    MessageViewer::TodoEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QStringLiteral("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("noteedit"));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(noteedit->text(), QString());
}

void TodoEditTest::shouldHaveLineEditFocus()
{
    MessageViewer::TodoEdit edit;
    edit.show();
    QTest::qWaitForWindowExposed(&edit);
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QStringLiteral("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("noteedit"));
    QCOMPARE(noteedit->hasFocus(), true);
}

void TodoEditTest::shouldShowMessageWidget()
{
    MessageViewer::TodoEdit edit;
    edit.show();
    QTest::qWaitForWindowExposed(&edit);

    KMime::Message::Ptr msg(new KMime::Message);
    msg->subject(true)->fromUnicodeString(QStringLiteral("Test note"), "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("noteedit"));
    noteedit->setText(QStringLiteral("Test Note"));
    KMessageWidget *msgwidget = edit.findChild<KMessageWidget *>(QStringLiteral("msgwidget"));
    QCOMPARE(msgwidget->isVisible(), false);
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(msgwidget->isVisible(), true);
}

void TodoEditTest::shouldHideMessageWidget()
{
    MessageViewer::TodoEdit edit;
    edit.show();
    QTest::qWaitForWindowExposed(&edit);
    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("noteedit"));
    noteedit->setText(QStringLiteral("Test note"));

    KMime::Message::Ptr msg(new KMime::Message);
    msg->subject(true)->fromUnicodeString(QStringLiteral("Test note"), "us-ascii");
    edit.setMessage(msg);
    KMessageWidget *msgwidget = edit.findChild<KMessageWidget *>(QStringLiteral("msgwidget"));
    msgwidget->show();
    QCOMPARE(msgwidget->isVisible(), true);
    noteedit->setText(QStringLiteral("Another note"));
    QCOMPARE(msgwidget->isVisible(), false);
}

void TodoEditTest::shouldHideMessageWidgetWhenAddNewMessage()
{
    MessageViewer::TodoEdit edit;
    edit.show();
    QTest::qWaitForWindowExposed(&edit);

    KMime::Message::Ptr msg(new KMime::Message);
    msg->subject(true)->fromUnicodeString(QStringLiteral("Test note"), "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("noteedit"));
    noteedit->setText(QStringLiteral("Test Note"));
    KMessageWidget *msgwidget = edit.findChild<KMessageWidget *>(QStringLiteral("msgwidget"));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(msgwidget->isVisible(), true);

    KMime::Message::Ptr msg2(new KMime::Message);
    msg2->subject(true)->fromUnicodeString(QStringLiteral("Test note 2"), "us-ascii");
    edit.setMessage(msg2);
    QCOMPARE(msgwidget->isVisible(), false);
}

void TodoEditTest::shouldHideMessageWidgetWhenCloseWidget()
{
    MessageViewer::TodoEdit edit;
    edit.show();
    QTest::qWaitForWindowExposed(&edit);

    KMime::Message::Ptr msg(new KMime::Message);
    msg->subject(true)->fromUnicodeString(QStringLiteral("Test note"), "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("noteedit"));
    noteedit->setText(QStringLiteral("Test Note"));
    KMessageWidget *msgwidget = edit.findChild<KMessageWidget *>(QStringLiteral("msgwidget"));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(msgwidget->isVisible(), true);
    edit.slotCloseWidget();

    QCOMPARE(msgwidget->isHidden(), true);
}

void TodoEditTest::shouldEnabledSaveOpenEditorButton()
{
    MessageViewer::TodoEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    msg->subject(true)->fromUnicodeString(QStringLiteral("Test note"), "us-ascii");
    edit.setMessage(msg);

    QLineEdit *noteedit = edit.findChild<QLineEdit *>(QStringLiteral("noteedit"));
    QPushButton *openEditor = edit.findChild<QPushButton *>(QStringLiteral("open-editor-button"));
    QPushButton *save = edit.findChild<QPushButton *>(QStringLiteral("save-button"));
    QCOMPARE(openEditor->isEnabled(), true);
    QCOMPARE(save->isEnabled(), true);
    noteedit->clear();

    QCOMPARE(openEditor->isEnabled(), false);
    QCOMPARE(save->isEnabled(), false);
    noteedit->setText(QStringLiteral("test 2"));
    QCOMPARE(openEditor->isEnabled(), true);
    QCOMPARE(save->isEnabled(), true);

    noteedit->setText(QStringLiteral(" "));
    QCOMPARE(openEditor->isEnabled(), false);
    QCOMPARE(save->isEnabled(), false);

    noteedit->setText(QStringLiteral(" foo"));
    QCOMPARE(openEditor->isEnabled(), true);
    QCOMPARE(save->isEnabled(), true);
}

void TodoEditTest::shouldDisabledSaveOpenEditorButtonWhenCollectionComboBoxIsEmpty()
{
    MessageViewer::TodoEdit edit;
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

QTEST_MAIN(TodoEditTest)
