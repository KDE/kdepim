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

#include <QHBoxLayout>
#include <QEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QToolButton>

#include <Akonadi/CollectionComboBox>


namespace MessageViewer {
MESSAGEVIEWER_EXPORT QAbstractItemModel *_k_todoEditStubModel = 0;
}

using namespace MessageViewer;

TodoEdit::TodoEdit(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->setSpacing(0);
    setLayout(hbox);

    QToolButton *closeBtn = new QToolButton( this );
    closeBtn->setIcon( KIcon( QLatin1String("dialog-close") ) );
    closeBtn->setObjectName(QLatin1String("close-button"));
    closeBtn->setIconSize( QSize( 16, 16 ) );
    closeBtn->setToolTip( i18n( "Close" ) );

#ifndef QT_NO_ACCESSIBILITY
    closeBtn->setAccessibleName( i18n( "Close" ) );
    closeBtn->setAccessibleDescription( i18n("Close widget to create new Todo") );
#endif

    closeBtn->setAutoRaise( true );
    hbox->addWidget( closeBtn );
    connect( closeBtn, SIGNAL(clicked()), this, SLOT(slotCloseWidget()) );

    QLabel *lab = new QLabel(i18n("Todo:"));
    hbox->addWidget(lab);

    mNoteEdit = new KLineEdit;
    mNoteEdit->setClearButtonShown(true);
    mNoteEdit->setObjectName(QLatin1String("noteedit"));
    mNoteEdit->setFocus();
    connect(mNoteEdit, SIGNAL(returnPressed()), SLOT(slotReturnPressed()));
    hbox->addWidget(mNoteEdit);
    mCollectionCombobox = new Akonadi::CollectionComboBox(_k_todoEditStubModel);
    mCollectionCombobox->setMinimumWidth(250);
    mCollectionCombobox->setMimeTypeFilter( QStringList() << KCalCore::Todo::todoMimeType() );
    mCollectionCombobox->setObjectName(QLatin1String("akonadicombobox"));
#ifndef QT_NO_ACCESSIBILITY
    mCollectionCombobox->setAccessibleDescription( i18n("Select collection where Todo will stored.") );
#endif

    connect(mCollectionCombobox, SIGNAL(currentIndexChanged(int)), SLOT(slotCollectionChanged(int)));
    connect(mCollectionCombobox, SIGNAL(activated(int)), SLOT(slotCollectionChanged(int)));
    hbox->addWidget(mCollectionCombobox);
    readConfig();
    setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
}

TodoEdit::~TodoEdit()
{

}

void TodoEdit::setMessageUrlAkonadi(const QString &url)
{
    mMessageUrlAkonadi = url;
}

QString TodoEdit::messageUrlAkonadi() const
{
    return mMessageUrlAkonadi;
}

void TodoEdit::writeConfig()
{
    MessageViewer::GlobalSettingsBase::self()->setLastSelectedFolder(mCollectionCombobox->currentCollection().id());
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
        mMessageUrlAkonadi.clear();
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
    hide();
}

void TodoEdit::slotReturnPressed()
{
    if (!mMessage) {
        kDebug()<<" Message is null";
        return;
    }
    const Akonadi::Collection collection = mCollectionCombobox->currentCollection();
    if (!mNoteEdit->text().isEmpty() && collection.isValid()) {
        KCalCore::Todo::Ptr todo( new KCalCore::Todo );
        todo->setSummary(mNoteEdit->text());
        Q_EMIT createTodo(todo, collection, mMessageUrlAkonadi);
        mNoteEdit->clear();
    }
}

bool TodoEdit::event(QEvent* e)
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
                   kev->key() == Qt::Key_Return ) {
            e->accept();
            if ( shortCutOverride ) {
                return true;
            }
        }
    }
    return QWidget::event(e);
}


