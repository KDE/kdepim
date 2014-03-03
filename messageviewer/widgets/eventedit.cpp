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

#include "eventedit.h"

#include "messageviewer/globalsettings_base.h"

#include <KLocalizedString>
#include <KLineEdit>
#include <KIcon>
#include <KDateTimeEdit>

#include <QHBoxLayout>
#include <QEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QToolButton>

#include <Akonadi/CollectionComboBox>


namespace MessageViewer {
MESSAGEVIEWER_EXPORT QAbstractItemModel *_k_eventEditStubModel = 0;
}

using namespace MessageViewer;

EventEdit::EventEdit(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setSpacing(0);
    setLayout(vbox);
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->setMargin(2);
    vbox->addLayout(hbox);

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

    QLabel *lab = new QLabel(i18n("Event:"));
    hbox->addWidget(lab);

    mNoteEdit = new KLineEdit;
    mNoteEdit->setClearButtonShown(true);
    mNoteEdit->setObjectName(QLatin1String("noteedit"));
    mNoteEdit->setFocus();
    connect(mNoteEdit, SIGNAL(returnPressed()), SLOT(slotReturnPressed()));
    hbox->addWidget(mNoteEdit);
    mCollectionCombobox = new Akonadi::CollectionComboBox(_k_eventEditStubModel);
    mCollectionCombobox->setAccessRightsFilter(Akonadi::Collection::CanCreateItem);
    mCollectionCombobox->setMinimumWidth(250);
    mCollectionCombobox->setMimeTypeFilter( QStringList() << KCalCore::Event::eventMimeType() );
    mCollectionCombobox->setObjectName(QLatin1String("akonadicombobox"));
#ifndef QT_NO_ACCESSIBILITY
    mCollectionCombobox->setAccessibleDescription( i18n("Select collection where Todo will stored.") );
#endif

    connect(mCollectionCombobox, SIGNAL(currentIndexChanged(int)), SLOT(slotCollectionChanged(int)));
    connect(mCollectionCombobox, SIGNAL(activated(int)), SLOT(slotCollectionChanged(int)));
    hbox->addWidget(mCollectionCombobox);

    hbox = new QHBoxLayout;
    vbox->addLayout(hbox);

    hbox->addStretch();
    lab = new QLabel(i18n("Start:"));
    hbox->addWidget(lab);
    KDateTime currentDateTime = KDateTime::currentDateTime(KDateTime::LocalZone);
    mStartDateTimeEdit = new KDateTimeEdit;
    mStartDateTimeEdit->setObjectName(QLatin1String("startdatetimeedit"));
    mStartDateTimeEdit->setDateTime(currentDateTime);
#ifndef QT_NO_ACCESSIBILITY
    mStartDateTimeEdit->setAccessibleDescription( i18n("Select start time for event.") );
#endif


    hbox->addWidget(mStartDateTimeEdit);

    lab = new QLabel(i18n("End:"));
    hbox->addWidget(lab);
    mEndDateTimeEdit = new KDateTimeEdit;
    mEndDateTimeEdit->setObjectName(QLatin1String("enddatetimeedit"));
    mEndDateTimeEdit->setDateTime(currentDateTime.addSecs(3600));
#ifndef QT_NO_ACCESSIBILITY
    mEndDateTimeEdit->setAccessibleDescription( i18n("Select end time for event.") );
#endif
    hbox->addWidget(mEndDateTimeEdit);

    readConfig();
    setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
}

EventEdit::~EventEdit()
{

}

void EventEdit::writeConfig()
{
    MessageViewer::GlobalSettingsBase::self()->setLastEventSelectedFolder(mCollectionCombobox->currentCollection().id());
}

void EventEdit::readConfig()
{
    const qint64 id = MessageViewer::GlobalSettingsBase::self()->lastEventSelectedFolder();
    if (id!=-1) {
        mCollectionCombobox->setDefaultCollection(Akonadi::Collection(id));
    }
}

Akonadi::Collection EventEdit::collection() const
{
    return mCollection;
}

void EventEdit::slotCollectionChanged(int /*index*/)
{
    setCollection(mCollectionCombobox->currentCollection());
}

void EventEdit::setCollection(const Akonadi::Collection &value)
{
    if (mCollection != value) {
        mCollection = value;
        Q_EMIT collectionChanged(mCollection);
    }
}

KMime::Message::Ptr EventEdit::message() const
{
    return mMessage;
}

void EventEdit::setMessage(const KMime::Message::Ptr &value)
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

void EventEdit::slotCloseWidget()
{
    writeConfig();
    mNoteEdit->clear();
    mMessage = KMime::Message::Ptr();
    hide();
}

void EventEdit::slotReturnPressed()
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
        KCalCore::Event::Ptr event( new KCalCore::Event );
        event->setDtStart(mStartDateTimeEdit->dateTime());
        event->setDtEnd(mEndDateTimeEdit->dateTime());
        event->setSummary(mNoteEdit->text());
        Q_EMIT createEvent(event, collection);
        mNoteEdit->clear();
        hide();
    }
}

bool EventEdit::event(QEvent* e)
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
