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

#include "todoedit.h"
#include "messageviewer/globalsettings_messageviewer.h"
#include "messageviewer_debug.h"
#include <KLocalizedString>
#include <QLineEdit>
#include <QIcon>

#include <KMessageWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QEvent>
#include <QKeyEvent>
#include <QLabel>

#include <AkonadiWidgets/CollectionComboBox>

#include <incidenceeditor-ng/incidencedialog.h>
#include <incidenceeditor-ng/incidencedialogfactory.h>
#include <KGuiItem>
#include <KStandardGuiItem>

namespace MessageViewer
{
MESSAGEVIEWER_EXPORT QAbstractItemModel *_k_todoEditStubModel = 0;
}

using namespace MessageViewer;

TodoEdit::TodoEdit(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setMargin(5);
    vbox->setSpacing(2);
    setLayout(vbox);

    mMsgWidget = new KMessageWidget(this);
    mMsgWidget->setCloseButtonVisible(true);
    mMsgWidget->setMessageType(KMessageWidget::Positive);
    mMsgWidget->setObjectName(QStringLiteral("msgwidget"));
    mMsgWidget->setWordWrap(true);
    mMsgWidget->setVisible(false);
    vbox->addWidget(mMsgWidget);

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->setMargin(0);
    hbox->setSpacing(2);
    vbox->addLayout(hbox);

    QLabel *lab = new QLabel(i18n("Todo:"));
    hbox->addWidget(lab);

    mNoteEdit = new QLineEdit;
    mNoteEdit->setClearButtonEnabled(true);
    mNoteEdit->setObjectName(QStringLiteral("noteedit"));
    mNoteEdit->setFocus();
    connect(mNoteEdit, &QLineEdit::textChanged, this, &TodoEdit::slotTextEdited);
    connect(mNoteEdit, &QLineEdit::returnPressed, this, &TodoEdit::slotReturnPressed);
    hbox->addWidget(mNoteEdit, 1);

    hbox->addSpacing(5);

    mCollectionCombobox = new Akonadi::CollectionComboBox(_k_todoEditStubModel);
    mCollectionCombobox->setAccessRightsFilter(Akonadi::Collection::CanCreateItem);
    mCollectionCombobox->setMinimumWidth(250);
    mCollectionCombobox->setMimeTypeFilter(QStringList() << KCalCore::Todo::todoMimeType());
    mCollectionCombobox->setObjectName(QStringLiteral("akonadicombobox"));
#ifndef QT_NO_ACCESSIBILITY
    mCollectionCombobox->setAccessibleDescription(i18n("Todo list where the new task will be stored."));
#endif
    mCollectionCombobox->setToolTip(i18n("Todo list where the new task will be stored"));
    connect(mCollectionCombobox, static_cast<void (Akonadi::CollectionComboBox::*)(int)>(&Akonadi::CollectionComboBox::currentIndexChanged), this, &TodoEdit::slotCollectionChanged);
    connect(mCollectionCombobox, static_cast<void (Akonadi::CollectionComboBox::*)(int)>(&Akonadi::CollectionComboBox::activated), this, &TodoEdit::slotCollectionChanged);
    hbox->addWidget(mCollectionCombobox);

    hbox = new QHBoxLayout;
    hbox->setMargin(0);
    hbox->setSpacing(2);
    vbox->addLayout(hbox);

    hbox->addStretch(1);
    mSaveButton = new QPushButton(QIcon::fromTheme(QStringLiteral("task-new")), i18n("&Save"));
    mSaveButton->setObjectName(QStringLiteral("save-button"));
    mSaveButton->setEnabled(false);
#ifndef QT_NO_ACCESSIBILITY
    mSaveButton->setAccessibleDescription(i18n("Create new todo and close this widget."));
#endif
    connect(mSaveButton, &QPushButton::clicked, this, &TodoEdit::slotReturnPressed);
    hbox->addWidget(mSaveButton);

    mOpenEditorButton = new QPushButton(i18n("Open &editor..."));
    mOpenEditorButton->setObjectName(QStringLiteral("open-editor-button"));
#ifndef QT_NO_ACCESSIBILITY
    mOpenEditorButton->setAccessibleDescription(i18n("Open todo editor, where more details can be changed."));
#endif
    mOpenEditorButton->setEnabled(false);
    connect(mOpenEditorButton, &QPushButton::clicked, this, &TodoEdit::slotOpenEditor);
    hbox->addWidget(mOpenEditorButton);

    QPushButton *btn = new QPushButton;
    KGuiItem::assign(btn, KStandardGuiItem::cancel());
    btn->setObjectName(QStringLiteral("close-button"));
#ifndef QT_NO_ACCESSIBILITY
    btn->setAccessibleDescription(i18n("Close the widget for creating new todos."));
#endif
    connect(btn, &QPushButton::clicked, this, &TodoEdit::slotCloseWidget);
    hbox->addWidget(btn);

    readConfig();
    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
    mCollectionCombobox->installEventFilter(this);
    installEventFilter(this);
}

TodoEdit::~TodoEdit()
{
    writeConfig();
}

void TodoEdit::updateButtons(const QString &subject)
{
    const bool subjectIsNotEmpty = !subject.trimmed().isEmpty();
    const bool collectionComboboxEmpty = (mCollectionCombobox->count() < 1);
    mSaveButton->setEnabled(subjectIsNotEmpty && !collectionComboboxEmpty);
    mOpenEditorButton->setEnabled(subjectIsNotEmpty && !collectionComboboxEmpty);
}

void TodoEdit::showToDoWidget()
{
    mNoteEdit->setFocus();
    show();
}

void TodoEdit::writeConfig()
{
    const Akonadi::Collection col = mCollectionCombobox->currentCollection();
    // col might not be valid if the collection wasn't found yet (the combo is async), skip saving in that case
    if (col.isValid() && col.id() != MessageViewer::GlobalSettingsBase::self()->lastSelectedFolder()) {
        MessageViewer::GlobalSettingsBase::self()->setLastSelectedFolder(col.id());
        MessageViewer::GlobalSettingsBase::self()->save();
    }
}

void TodoEdit::readConfig()
{
    const qint64 id = MessageViewer::GlobalSettingsBase::self()->lastSelectedFolder();
    if (id != -1) {
        mCollectionCombobox->setDefaultCollection(Akonadi::Collection(id));
    }
}

Akonadi::Collection TodoEdit::collection() const
{
    return mCollection;
}

void TodoEdit::slotCollectionChanged(int /*index*/)
{
    setCollection(mCollectionCombobox->currentCollection());
}

void TodoEdit::setCollection(const Akonadi::Collection &value)
{
    if (mCollection != value) {
        mCollection = value;
        Q_EMIT collectionChanged(mCollection);
    }
}

KMime::Message::Ptr TodoEdit::message() const
{
    return mMessage;
}

void TodoEdit::setMessage(const KMime::Message::Ptr &value)
{
    if (mMessage != value) {
        mMessage = value;
        const KMime::Headers::Subject *const subject = mMessage ? mMessage->subject(false) : 0;
        if (subject) {
            mNoteEdit->setText(i18n("Reply to \"%1\"", subject->asUnicodeString()));
            mNoteEdit->selectAll();
            mNoteEdit->setFocus();
        } else {
            mNoteEdit->clear();
        }
        Q_EMIT messageChanged(mMessage);
    }
}

void TodoEdit::slotCloseWidget()
{
    if (isVisible()) {
        writeConfig();
        mNoteEdit->clear();
        mMessage = KMime::Message::Ptr();
        mMsgWidget->hide();
        hide();
    }
}

void TodoEdit::slotReturnPressed()
{
    if (!mMessage) {
        qCDebug(MESSAGEVIEWER_LOG) << " Message is null";
        return;
    }
    const Akonadi::Collection collection = mCollectionCombobox->currentCollection();
    if (!collection.isValid()) {
        qCDebug(MESSAGEVIEWER_LOG) << " Collection is not valid";
        return;
    }

    if (!mNoteEdit->text().trimmed().isEmpty()) {
        mMsgWidget->setText(i18nc("%1 is summary of the todo, %2 is name of the folder in which it is stored",
                                  "New todo '%1' was added to task list '%2'", mNoteEdit->text(), collection.displayName()));
        KCalCore::Todo::Ptr todo(new KCalCore::Todo);
        todo->setSummary(mNoteEdit->text());
        mNoteEdit->clear();

        // We don't hide the widget here, so that multiple todo's can be added
        Q_EMIT createTodo(todo, collection);

        mMsgWidget->animatedShow();
    }
}

bool TodoEdit::eventFilter(QObject *object, QEvent *e)
{
    // Close the bar when pressing Escape.
    // Not using a QShortcut for this because it could conflict with
    // window-global actions (e.g. Emil Sedgh binds Esc to "close tab").
    // With a shortcut override we can catch this before it gets to kactions.
    const bool shortCutOverride = (e->type() == QEvent::ShortcutOverride);
    if (shortCutOverride || e->type() == QEvent::KeyPress) {
        QKeyEvent *kev = static_cast<QKeyEvent * >(e);
        if (kev->key() == Qt::Key_Escape) {
            e->accept();
            slotCloseWidget();
            return true;
        } else if (kev->key() == Qt::Key_Enter ||
                   kev->key() == Qt::Key_Return ||
                   kev->key() == Qt::Key_Space) {
            e->accept();
            if (shortCutOverride) {
                return true;
            }
            if (object == mCollectionCombobox) {
                mCollectionCombobox->showPopup();
                return true;
            }
        }
    }
    return QWidget::eventFilter(object, e);
}

void TodoEdit::slotOpenEditor()
{
    const KMime::Headers::Subject *const subject = mMessage->subject(false);
    IncidenceEditorNG::IncidenceDialog *dlg = IncidenceEditorNG::IncidenceDialogFactory::createTodoEditor(
                mNoteEdit->text(), QString(),
                QStringList() << QString::fromLatin1(mMessage->encodedContent().toBase64()),
                QStringList(),  // attendees
                QStringList() << KMime::Message::mimeType(),
                QStringList() << (subject ? subject->asUnicodeString() : QString()),
                false, mCollection, false, this);
    connect(dlg, &IncidenceEditorNG::IncidenceDialog::finished, this, &TodoEdit::slotCloseWidget);
    dlg->open();
}

void TodoEdit::slotTextEdited(const QString &subject)
{
    updateButtons(subject);
    if (mMsgWidget->isVisible()) {
        mMsgWidget->hide();
    }
}
