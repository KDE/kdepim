/*
  Copyright (c) 2014 Sandro Knauß <knauss@kolabsys.com>*

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

#include "noteedit.h"

#include "messageviewer/globalsettings_base.h"

#include <KLocalizedString>
#include <KLineEdit>
#include <KIcon>
#include <KDateTimeEdit>

#include <QHBoxLayout>
#include <QEvent>
#include <QKeyEvent>
#include <QLabel>

#include <Akonadi/CollectionComboBox>
#include <KPushButton>

#include <incidenceeditor-ng/incidencedialogfactory.h>
#include <incidenceeditor-ng/incidencedialog.h>

namespace MessageViewer {
MESSAGEVIEWER_EXPORT QAbstractItemModel *_k_noteEditStubModel = 0;
}

using namespace MessageViewer;

NoteEdit::NoteEdit(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setMargin(5);
    vbox->setSpacing(2);
    setLayout(vbox);

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->setMargin(0);
    hbox->setSpacing(2);
    vbox->addLayout(hbox);

    QLabel *lab = new QLabel(i18n("Note:"));
    hbox->addWidget(lab);

    mNoteEdit = new KLineEdit;
    mNoteEdit->setClearButtonShown(true);
    mNoteEdit->setObjectName(QLatin1String("noteedit"));
    mNoteEdit->setFocus();
    connect(mNoteEdit, SIGNAL(returnPressed()), SLOT(slotReturnPressed()));
    connect(mNoteEdit, SIGNAL(textChanged(QString)), SLOT(slotUpdateButtons(QString)));
    hbox->addWidget(mNoteEdit);

    hbox->addSpacing(5);

    mCollectionCombobox = new Akonadi::CollectionComboBox(_k_noteEditStubModel);
    mCollectionCombobox->setAccessRightsFilter(Akonadi::Collection::CanCreateItem);
    mCollectionCombobox->setMinimumWidth(250);
    mCollectionCombobox->setMimeTypeFilter( QStringList() << Akonadi::NoteUtils::noteMimeType() );
    mCollectionCombobox->setObjectName(QLatin1String("akonadicombobox"));
#ifndef QT_NO_ACCESSIBILITY
    mCollectionCombobox->setAccessibleDescription(i18n("Calendar where the new event will be stored."));
#endif
    mCollectionCombobox->setToolTip(i18n("Calendar where the new event will be stored."));

    connect(mCollectionCombobox, SIGNAL(currentIndexChanged(int)), SLOT(slotCollectionChanged(int)));
    connect(mCollectionCombobox, SIGNAL(activated(int)), SLOT(slotCollectionChanged(int)));
    hbox->addWidget(mCollectionCombobox);

    hbox = new QHBoxLayout;
    hbox->setMargin(0);
    hbox->setSpacing(2);
    vbox->addLayout(hbox);

    hbox->addStretch(1);

    hbox = new QHBoxLayout;
    hbox->setSpacing(2);
    hbox->setMargin(0);
    vbox->addLayout(hbox);

    hbox->addStretch(1);

    mSaveButton = new KPushButton(KIcon(QLatin1String("view-pim-notes")), i18n("&Save"));
    mSaveButton->setObjectName(QLatin1String("save-button"));
    mSaveButton->setEnabled(false);
#ifndef QT_NO_ACCESSIBILITY
    mSaveButton->setAccessibleDescription(i18n("Create new note and close this widget."));
#endif
    connect(mSaveButton, SIGNAL(clicked(bool)), this, SLOT(slotReturnPressed()));
    hbox->addWidget(mSaveButton);

    KPushButton *btn = new KPushButton(KStandardGuiItem::cancel());
    btn->setObjectName(QLatin1String("close-button"));
#ifndef QT_NO_ACCESSIBILITY
    btn->setAccessibleDescription(i18n("Close the widget for creating new notes."));
#endif
    connect(btn, SIGNAL(clicked(bool)), this, SLOT(slotCloseWidget()));
    hbox->addWidget(btn);

    readConfig();
    setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
    installEventFilter(this);
    mCollectionCombobox->installEventFilter(this);
}

NoteEdit::~NoteEdit()
{
    writeConfig();
}

void NoteEdit::writeConfig()
{
    if (mCollectionCombobox->currentCollection().id() != MessageViewer::GlobalSettingsBase::self()->lastNoteSelectedFolder()) {
        MessageViewer::GlobalSettingsBase::self()->setLastNoteSelectedFolder(mCollectionCombobox->currentCollection().id());
        MessageViewer::GlobalSettingsBase::self()->writeConfig();
    }
}

void NoteEdit::slotUpdateButtons(const QString &subject)
{
    const bool subjectIsNotEmpty = !subject.isEmpty();
    mSaveButton->setEnabled(subjectIsNotEmpty);
}

void NoteEdit::showNoteEdit()
{
    mNoteEdit->setFocus();
    show();
}

void NoteEdit::readConfig()
{
    const qint64 id = MessageViewer::GlobalSettingsBase::self()->lastNoteSelectedFolder();
    if (id!=-1) {
        mCollectionCombobox->setDefaultCollection(Akonadi::Collection(id));
    }
}

Akonadi::Collection NoteEdit::collection() const
{
    return mCollection;
}

void NoteEdit::slotCollectionChanged(int /*index*/)
{
    setCollection(mCollectionCombobox->currentCollection());
}

void NoteEdit::setCollection(const Akonadi::Collection &value)
{
    if (mCollection != value) {
        mCollection = value;
        Q_EMIT collectionChanged(mCollection);
    }
}

KMime::Message::Ptr NoteEdit::message() const
{
    return mMessage;
}

void NoteEdit::setMessage(const KMime::Message::Ptr &value)
{
    if (mMessage != value) {
        mMessage = value;
        const KMime::Headers::Subject * const subject = mMessage ? mMessage->subject(false) : 0;
        if (subject) {
            mNoteEdit->setText(subject->asUnicodeString());
            mNoteEdit->selectAll();
            mNoteEdit->setFocus();
        } else {
            mNoteEdit->clear();
        }
        Q_EMIT messageChanged(mMessage);
    }
}

void NoteEdit::slotCloseWidget()
{
    writeConfig();
    mNoteEdit->clear();
    mMessage = KMime::Message::Ptr();
    hide();
}

void NoteEdit::slotReturnPressed()
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
        Akonadi::NoteUtils::NoteMessageWrapper note;
        note.setTitle(mNoteEdit->text());
        Q_EMIT createNote(note.message(), collection);
        mNoteEdit->clear();
        hide();
    }
}

bool NoteEdit::eventFilter(QObject *object, QEvent *e)
{
    // Close the bar when pressing Escape.
    // Not using a QShortcut for this because it could conflict with
    // window-global actions (e.g. Emil Sedgh binds Esc to "close tab").
    // With a shortcut override we can catch this before it gets to kactions.
    const bool shortCutOverride = (e->type() == QEvent::ShortcutOverride);
    if (shortCutOverride ) {
        QKeyEvent* kev = static_cast<QKeyEvent* >(e);
        if (kev->key() == Qt::Key_Escape) {
            e->accept();
            slotCloseWidget();
            return true;
        } else if ( kev->key() == Qt::Key_Enter ||
                    kev->key() == Qt::Key_Return ||
                    kev->key() == Qt::Key_Space) {
            e->accept();
            if (object == mCollectionCombobox) {
                mCollectionCombobox->showPopup();
            }
            return true;
        }
    }
    return QWidget::eventFilter(object,e);
}