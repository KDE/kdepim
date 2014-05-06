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

#include "todoedit.h"
#include "messageviewer/globalsettings_base.h"

#include <KLocalizedString>
#include <KLineEdit>
#include <KIcon>
#include <KPushButton>
#include <KMessageWidget>

#include <QHBoxLayout>
#include <QEvent>
#include <QKeyEvent>
#include <QLabel>

#include <Akonadi/CollectionComboBox>

#include <incidenceeditor-ng/incidencedialog.h>
#include <incidenceeditor-ng/incidencedialogfactory.h>


namespace MessageViewer {
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
    mMsgWidget->setObjectName(QLatin1String("msgwidget"));
    mMsgWidget->setVisible(false);
    vbox->addWidget(mMsgWidget);

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->setMargin(0);
    hbox->setSpacing(2);
    vbox->addLayout(hbox);

    QLabel *lab = new QLabel(i18n("Todo:"));
    hbox->addWidget(lab);

    mNoteEdit = new KLineEdit;
    mNoteEdit->setClearButtonShown(true);
    mNoteEdit->setObjectName(QLatin1String("noteedit"));
    mNoteEdit->setFocus();
    connect(mNoteEdit, SIGNAL(textChanged(QString)), SLOT(slotTextEdited()));
    connect(mNoteEdit, SIGNAL(returnPressed()), SLOT(slotReturnPressed()));
    hbox->addWidget(mNoteEdit, 1);

    hbox->addSpacing(5);

    mCollectionCombobox = new Akonadi::CollectionComboBox(_k_todoEditStubModel);
    mCollectionCombobox->setAccessRightsFilter(Akonadi::Collection::CanCreateItem);
    mCollectionCombobox->setMinimumWidth(250);
    mCollectionCombobox->setMimeTypeFilter( QStringList() << KCalCore::Todo::todoMimeType() );
    mCollectionCombobox->setObjectName(QLatin1String("akonadicombobox"));
#ifndef QT_NO_ACCESSIBILITY
    mCollectionCombobox->setAccessibleDescription( i18n("Todo list where the new task will be stored.") );
#endif
    mCollectionCombobox->setToolTip( i18n("Todo list where the new task will be stored.") );
    connect(mCollectionCombobox, SIGNAL(currentIndexChanged(int)), SLOT(slotCollectionChanged(int)));
    connect(mCollectionCombobox, SIGNAL(activated(int)), SLOT(slotCollectionChanged(int)));
    hbox->addWidget(mCollectionCombobox);

    hbox = new QHBoxLayout;
    hbox->setMargin(0);
    hbox->setSpacing(2);
    vbox->addLayout(hbox);

    hbox->addStretch(1);
    KPushButton *btn = new KPushButton(KIcon(QLatin1String("todo-new")), i18n("&Save"));
    btn->setObjectName(QLatin1String("save-button"));
#ifndef QT_NO_ACCESSIBILITY
    btn->setAccessibleDescription(i18n("Create new todo and close this widget."));
#endif
    connect(btn, SIGNAL(clicked(bool)), this, SLOT(slotReturnPressed()));
    hbox->addWidget(btn);

    btn = new KPushButton(i18n("Open &editor..."));
#ifndef QT_NO_ACCESSIBILITY
    btn->setAccessibleDescription(i18n("Open todo editor, where more details can be changed."));
#endif
    connect(btn, SIGNAL(clicked(bool)), this, SLOT(slotOpenEditor()));
    hbox->addWidget(btn);

    btn = new KPushButton(i18n("&Cancel"));
    btn->setObjectName(QLatin1String("close-button"));
#ifndef QT_NO_ACCESSIBILITY
    btn->setAccessibleDescription(i18n("Close the widget for creating new todos."));
#endif
    connect(btn, SIGNAL(clicked(bool)), this, SLOT(slotCloseWidget()));
    hbox->addWidget(btn);


    readConfig();
    setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
    mCollectionCombobox->installEventFilter(this);
    installEventFilter(this);
}

TodoEdit::~TodoEdit()
{
    writeConfig();
}

void TodoEdit::showToDoWidget()
{
    mNoteEdit->setFocus();
    show();
}

void TodoEdit::writeConfig()
{
    MessageViewer::GlobalSettingsBase::self()->setLastSelectedFolder(mCollectionCombobox->currentCollection().id());
    MessageViewer::GlobalSettingsBase::self()->writeConfig();
}

void TodoEdit::readConfig()
{
    const qint64 id = MessageViewer::GlobalSettingsBase::self()->lastSelectedFolder();
    if (id!=-1) {
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
        const KMime::Headers::Subject * const subject = mMessage ? mMessage->subject(false) : 0;
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
    writeConfig();
    mNoteEdit->clear();
    mMessage = KMime::Message::Ptr();
    mMsgWidget->hide();
    hide();
}

void TodoEdit::slotReturnPressed()
{
    if (!mMessage) {
        kDebug()<<" Message is null";
        return;
    }
    const Akonadi::Collection collection = mCollectionCombobox->currentCollection();
    if (!collection.isValid()) {
        kDebug()<<" Collection is not valid";
        return;
    }

    if (!mNoteEdit->text().trimmed().isEmpty()) {
        mMsgWidget->setText(i18nc("%1 is summary of the todo, %2 is name of the folder in which it is stored",
                                  "New todo '%1' was added to task list '%2'", mNoteEdit->text(), collection.displayName()));
        KCalCore::Todo::Ptr todo( new KCalCore::Todo );
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
    if (shortCutOverride || e->type() == QEvent::KeyPress ) {
        QKeyEvent* kev = static_cast<QKeyEvent* >(e);
        if (kev->key() == Qt::Key_Escape) {
            e->accept();
            slotCloseWidget();
            return true;
        } else if ( kev->key() == Qt::Key_Enter ||
                    kev->key() == Qt::Key_Return ||
                    kev->key() == Qt::Key_Space) {
            e->accept();
            if ( shortCutOverride ) {
                return true;
            }
            if (object == mCollectionCombobox) {
                mCollectionCombobox->showPopup();
                return true;
            }
        }
    }
    return QWidget::eventFilter(object,e);
}

void TodoEdit::slotOpenEditor()
{
    const KMime::Headers::Subject * const subject = mMessage->subject(false);
    IncidenceEditorNG::IncidenceDialog *dlg = IncidenceEditorNG::IncidenceDialogFactory::createTodoEditor(
        mNoteEdit->text(), QString(),
        QStringList() << QString::fromLatin1(mMessage->encodedContent().toBase64()),
        QStringList(),  // attendees
        QStringList() << KMime::Message::mimeType(),
        QStringList() << (subject ? subject->asUnicodeString() : QString()),
        false, mCollection, false, this);
    connect(dlg, SIGNAL(finished()), this, SLOT(slotCloseWidget()));
    dlg->open();
}

void TodoEdit::slotTextEdited()
{
    if (mMsgWidget->isVisible()) {
        mMsgWidget->hide();
    }
}
