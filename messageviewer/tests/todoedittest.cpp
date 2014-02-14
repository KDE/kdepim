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

#include "todoedittest.h"
#include "messageviewer/globalsettings_base.h"
#include "widgets/todoedit.h"
#include <Akonadi/Collection>
#include <Akonadi/CollectionComboBox>
#include <Akonadi/EntityTreeModel>
#include <QStandardItemModel>
#include <kcalcore/todo.h>
#include <qtest_kde.h>
#include <qtestkeyboard.h>
#include <qtestmouse.h>

#include <QLineEdit>
#include <QToolButton>
#include <QHBoxLayout>
#include <QShortcut>
#include <QAction>

namespace MessageViewer {
extern MESSAGEVIEWER_EXPORT QAbstractItemModel *_k_todoEditStubModel;
}


Q_DECLARE_METATYPE(KMime::Message::Ptr)
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
    QVERIFY(!edit.collection().isValid());
    QVERIFY(!edit.message());
    QVERIFY(edit.messageUrlAkonadi().isEmpty());
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

    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    QVERIFY(noteedit);
    QCOMPARE(noteedit->text(), QString());

    QString subject = QLatin1String("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);

    QCOMPARE(noteedit->text(), QString::fromLatin1("Reply to \"%1\"").arg(subject));
}

void TodoEditTest::shouldEmptySubjectWhenMessageIsNull()
{
    MessageViewer::TodoEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QLatin1String("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    edit.setMessage(KMime::Message::Ptr());
    QCOMPARE(noteedit->text(), QString());
}

void TodoEditTest::shouldEmptySubjectWhenMessageHasNotSubject()
{
    MessageViewer::TodoEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QLatin1String("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    KMime::Message::Ptr msgSubjectEmpty(new KMime::Message);
    edit.setMessage(msgSubjectEmpty);
    QCOMPARE(noteedit->text(), QString());
}

void TodoEditTest::shouldSelectLineWhenPutMessage()
{
    MessageViewer::TodoEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QLatin1String("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    QVERIFY(noteedit->hasSelectedText());
    const QString selectedText = noteedit->selectedText();
    QCOMPARE(selectedText, QString::fromLatin1("Reply to \"%1\"").arg(subject));
}

void TodoEditTest::shouldEmitCollectionChangedWhenChangeComboboxItem()
{
    MessageViewer::TodoEdit edit;
    Akonadi::CollectionComboBox *akonadicombobox = qFindChild<Akonadi::CollectionComboBox *>(&edit, QLatin1String("akonadicombobox"));
    QVERIFY(akonadicombobox);
    QVERIFY(akonadicombobox->currentCollection().isValid());
}

void TodoEditTest::shouldEmitNotEmitTodoWhenTextIsEmpty()
{
    MessageViewer::TodoEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    edit.setMessage(msg);
    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    QSignalSpy spy(&edit, SIGNAL(createTodo(KCalCore::Todo::Ptr,Akonadi::Collection)));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(spy.count(), 0);
}

void TodoEditTest::shouldEmitTodoWhenPressEnter()
{
    MessageViewer::TodoEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QLatin1String("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    QSignalSpy spy(&edit, SIGNAL(createTodo(KCalCore::Todo::Ptr,Akonadi::Collection)));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(spy.count(), 1);
}

void TodoEditTest::shouldTodoHasCorrectSubject()
{
    MessageViewer::TodoEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QLatin1String("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    QSignalSpy spy(&edit, SIGNAL(createTodo(KCalCore::Todo::Ptr,Akonadi::Collection)));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(spy.count(), 1);
    KCalCore::Todo::Ptr todoPtr = spy.at(0).at(0).value<KCalCore::Todo::Ptr>();
    QVERIFY(todoPtr);
    QCOMPARE(todoPtr->summary(), QString::fromLatin1("Reply to \"%1\"").arg(subject));
}

void TodoEditTest::shouldClearAllWhenCloseWidget()
{
    MessageViewer::TodoEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QLatin1String("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    edit.slotCloseWidget();
    QCOMPARE(noteedit->text(), QString());
    QVERIFY(!edit.message());
}

void TodoEditTest::shouldEmitCollectionChangedWhenCurrentCollectionWasChanged()
{
    MessageViewer::TodoEdit edit;
    Akonadi::CollectionComboBox *akonadicombobox = qFindChild<Akonadi::CollectionComboBox *>(&edit, QLatin1String("akonadicombobox"));
    QSignalSpy spy(&edit, SIGNAL(collectionChanged(Akonadi::Collection)));
    QCOMPARE(akonadicombobox->currentIndex(), 0);
    akonadicombobox->setCurrentIndex(3);
    QCOMPARE(akonadicombobox->currentIndex(), 3);
    QCOMPARE(spy.count(), 1);
}

void TodoEditTest::shouldEmitCorrectCollection()
{
    MessageViewer::TodoEdit edit;
    Akonadi::CollectionComboBox *akonadicombobox = qFindChild<Akonadi::CollectionComboBox *>(&edit, QLatin1String("akonadicombobox"));
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QLatin1String("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    akonadicombobox->setCurrentIndex(3);
    Akonadi::Collection col = akonadicombobox->currentCollection();
    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    QSignalSpy spy(&edit, SIGNAL(createTodo(KCalCore::Todo::Ptr,Akonadi::Collection)));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(1).value<Akonadi::Collection>(), col);
}

void TodoEditTest::shouldHideWidgetWhenClickOnCloseButton()
{
    MessageViewer::TodoEdit edit;
    edit.show();
    QTest::qWaitForWindowShown(&edit);
    QVERIFY(edit.isVisible());
    QToolButton *close = qFindChild<QToolButton *>(&edit, QLatin1String("close-button"));
    QTest::mouseClick(close, Qt::LeftButton);
    QCOMPARE(edit.isVisible(), false);
}

void TodoEditTest::shouldHideWidgetWhenPressEscape()
{
    MessageViewer::TodoEdit edit;
    edit.show();
    QTest::qWaitForWindowShown(&edit);
    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    noteedit->setFocus();
    QVERIFY(noteedit->hasFocus());
    QTest::keyPress(&edit, Qt::Key_Escape);
    QCOMPARE(edit.isVisible(), false);
}

void TodoEditTest::shouldSaveCollectionSettings()
{
    MessageViewer::TodoEdit edit;
    Akonadi::CollectionComboBox *akonadicombobox = qFindChild<Akonadi::CollectionComboBox *>(&edit, QLatin1String("akonadicombobox"));
    akonadicombobox->setCurrentIndex(3);
    const Akonadi::Collection::Id id = akonadicombobox->currentCollection().id();
    QToolButton *close = qFindChild<QToolButton *>(&edit, QLatin1String("close-button"));
    QTest::mouseClick(close, Qt::LeftButton);
    QCOMPARE(MessageViewer::GlobalSettingsBase::self()->lastSelectedFolder(), id);
}

void TodoEditTest::shouldSaveCollectionSettingsWhenCloseWidget()
{
    MessageViewer::TodoEdit edit;
    Akonadi::CollectionComboBox *akonadicombobox = qFindChild<Akonadi::CollectionComboBox *>(&edit, QLatin1String("akonadicombobox"));
    akonadicombobox->setCurrentIndex(3);
    const Akonadi::Collection::Id id = akonadicombobox->currentCollection().id();
    edit.writeConfig();
    QCOMPARE(MessageViewer::GlobalSettingsBase::self()->lastSelectedFolder(), id);
}

void TodoEditTest::shouldNotEmitTodoWhenMessageIsNull()
{
    MessageViewer::TodoEdit edit;
    QString subject = QLatin1String("Test Note");
    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    noteedit->setText(subject);
    QSignalSpy spy(&edit, SIGNAL(createTodo(KCalCore::Todo::Ptr,Akonadi::Collection)));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(spy.count(), 0);
}

void TodoEditTest::shouldClearLineAfterEmitNewNote()
{
    MessageViewer::TodoEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QLatin1String("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    QTest::keyClick(noteedit, Qt::Key_Enter);
    QCOMPARE(noteedit->text(), QString());
}

void TodoEditTest::shouldHaveLineEditFocus()
{
    MessageViewer::TodoEdit edit;
    edit.show();
    QTest::qWaitForWindowShown(&edit);
    KMime::Message::Ptr msg(new KMime::Message);
    QString subject = QLatin1String("Test Note");
    msg->subject(true)->fromUnicodeString(subject, "us-ascii");
    edit.setMessage(msg);
    QLineEdit *noteedit = qFindChild<QLineEdit *>(&edit, QLatin1String("noteedit"));
    QCOMPARE(noteedit->hasFocus(), true);
}

void TodoEditTest::shouldClearUrlMessageWhenSwitchMessage()
{
    MessageViewer::TodoEdit edit;
    KMime::Message::Ptr msg(new KMime::Message);
    edit.setMessage(msg);
    edit.setMessageUrlAkonadi(QLatin1String("test"));
    QCOMPARE(edit.messageUrlAkonadi(), QLatin1String("test"));
    KMime::Message::Ptr msg2(new KMime::Message);
    edit.setMessage(msg2);
    QCOMPARE(edit.messageUrlAkonadi(), QString());
}


QTEST_KDEMAIN( TodoEditTest, GUI )
