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
#include <QLineEdit>
#include <QIcon>
#include <KDateTimeEdit>
#include <QDateTime>
#include <QDebug>

#include <QHBoxLayout>
#include <QEvent>
#include <QKeyEvent>
#include <QLabel>

#include <AkonadiWidgets/CollectionComboBox>
#include <QPushButton>

#include <incidenceeditor-ng/incidencedialogfactory.h>
#include <incidenceeditor-ng/incidencedialog.h>
#include <KGuiItem>
#include <KStandardGuiItem>


namespace MessageViewer {
MESSAGEVIEWER_EXPORT QAbstractItemModel *_k_eventEditStubModel = 0;
}

using namespace MessageViewer;

EventEdit::EventEdit(QWidget *parent)
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

    QLabel *lab = new QLabel(i18n("Event:"));
    hbox->addWidget(lab);

    mEventEdit = new QLineEdit;
    mEventEdit->setClearButtonEnabled(true);
    mEventEdit->setObjectName(QLatin1String("noteedit"));
    mEventEdit->setFocus();
    connect(mEventEdit, SIGNAL(returnPressed()), SLOT(slotReturnPressed()));
    connect(mEventEdit, SIGNAL(textChanged(QString)), SLOT(slotUpdateButtons(QString)));
    hbox->addWidget(mEventEdit);

    hbox->addSpacing(5);

    mCollectionCombobox = new Akonadi::CollectionComboBox(_k_eventEditStubModel);
    mCollectionCombobox->setAccessRightsFilter(Akonadi::Collection::CanCreateItem);
    mCollectionCombobox->setMinimumWidth(250);
    mCollectionCombobox->setMimeTypeFilter( QStringList() << KCalCore::Event::eventMimeType() );
    mCollectionCombobox->setObjectName(QLatin1String("akonadicombobox"));
#ifndef QT_NO_ACCESSIBILITY
    mCollectionCombobox->setAccessibleDescription( i18n("Calendar where the new event will be stored.") );
#endif
    mCollectionCombobox->setToolTip( i18n("Calendar where the new event will be stored.") );

    connect(mCollectionCombobox, SIGNAL(currentIndexChanged(int)), SLOT(slotCollectionChanged(int)));
    connect(mCollectionCombobox, SIGNAL(activated(int)), SLOT(slotCollectionChanged(int)));
    hbox->addWidget(mCollectionCombobox);

    hbox = new QHBoxLayout;
    hbox->setMargin(0);
    hbox->setSpacing(2);
    vbox->addLayout(hbox);

    lab = new QLabel(i18n("Start:"));
    hbox->addWidget(lab);
    KDateTime currentDateTime = KDateTime::currentDateTime(KDateTime::LocalZone);
    mStartDateTimeEdit = new KDateTimeEdit;
    mStartDateTimeEdit->setObjectName(QLatin1String("startdatetimeedit"));
    //QT5 mStartDateTimeEdit->setDateTime(currentDateTime);
#ifndef QT_NO_ACCESSIBILITY
    mStartDateTimeEdit->setAccessibleDescription( i18n("Select start time for event.") );
#endif
    connect(mStartDateTimeEdit, SIGNAL(dateTimeChanged(KDateTime)),
            this, SLOT(slotStartDateTimeChanged(KDateTime)));
    hbox->addWidget(mStartDateTimeEdit);

    hbox->addSpacing(5);

    lab = new QLabel(i18n("End:"));
    hbox->addWidget(lab);
    mEndDateTimeEdit = new KDateTimeEdit;
    mEndDateTimeEdit->setObjectName(QLatin1String("enddatetimeedit"));
    //QT5 mEndDateTimeEdit->setDateTime(currentDateTime.addSecs(3600));
#ifndef QT_NO_ACCESSIBILITY
    mEndDateTimeEdit->setAccessibleDescription( i18n("Select end time for event.") );
#endif
    hbox->addWidget(mEndDateTimeEdit);

    hbox->addStretch(1);

    hbox = new QHBoxLayout;
    hbox->setSpacing(2);
    hbox->setMargin(0);
    vbox->addLayout(hbox);

    hbox->addStretch(1);

    mSaveButton = new QPushButton(QIcon::fromTheme(QLatin1String("appointment-new")), i18n("&Save"));
    mSaveButton->setObjectName(QLatin1String("save-button"));
    mSaveButton->setEnabled(false);
#ifndef QT_NO_ACCESSIBILITY
    mSaveButton->setAccessibleDescription(i18n("Create new event and close this widget."));
#endif
    connect(mSaveButton, SIGNAL(clicked(bool)), this, SLOT(slotReturnPressed()));
    hbox->addWidget(mSaveButton);

    mOpenEditorButton = new QPushButton(i18n("Open &editor..."));
#ifndef QT_NO_ACCESSIBILITY
    mOpenEditorButton->setAccessibleDescription(i18n("Open event editor, where more details can be changed."));
#endif
    mOpenEditorButton->setObjectName(QLatin1String("open-editor-button"));
    mOpenEditorButton->setEnabled(false);
    connect(mOpenEditorButton, SIGNAL(clicked(bool)), this, SLOT(slotOpenEditor()));
    hbox->addWidget(mOpenEditorButton);

    QPushButton *btn = new QPushButton;
    KGuiItem::assign(btn,KStandardGuiItem::cancel());
    btn->setObjectName(QLatin1String("close-button"));
#ifndef QT_NO_ACCESSIBILITY
    btn->setAccessibleDescription(i18n("Close the widget for creating new events."));
#endif
    connect(btn, SIGNAL(clicked(bool)), this, SLOT(slotCloseWidget()));
    hbox->addWidget(btn);

    readConfig();
    setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
    installEventFilter(this);
    mCollectionCombobox->installEventFilter(this);
}

EventEdit::~EventEdit()
{
    writeConfig();
}

void EventEdit::writeConfig()
{
    if (mCollectionCombobox->currentCollection().id() != MessageViewer::GlobalSettingsBase::self()->lastEventSelectedFolder()) {
        MessageViewer::GlobalSettingsBase::self()->setLastEventSelectedFolder(mCollectionCombobox->currentCollection().id());
        MessageViewer::GlobalSettingsBase::self()->save();
    }
}

void EventEdit::slotUpdateButtons(const QString &subject)
{
    const bool subjectIsNotEmpty = !subject.isEmpty();
    mSaveButton->setEnabled(subjectIsNotEmpty);
    mOpenEditorButton->setEnabled(subjectIsNotEmpty);
}

void EventEdit::showEventEdit()
{
    mEventEdit->setFocus();
    show();
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
            mEventEdit->setText(i18n("Reply to \"%1\"", subject->asUnicodeString()));
            mEventEdit->selectAll();
            mEventEdit->setFocus();
        } else {
            mEventEdit->clear();
        }
        Q_EMIT messageChanged(mMessage);
    }
}

void EventEdit::slotCloseWidget()
{
    writeConfig();
    mEventEdit->clear();
    mMessage = KMime::Message::Ptr();
    hide();
}

void EventEdit::slotReturnPressed()
{
    if (!mMessage) {
        qDebug()<<" Message is null";
        return;
    }
    const Akonadi::Collection collection = mCollectionCombobox->currentCollection();
    if (!collection.isValid()) {
        qDebug()<<" Collection is not valid";
        return;
    }

    const QDateTime dtstart = mStartDateTimeEdit->dateTime();
    const QDateTime dtend = mEndDateTimeEdit->dateTime();
    if (!dtstart.isValid() || !dtend.isValid()) {
        qDebug()<<" date is not valid !";
        return;
    }

    if (!mEventEdit->text().trimmed().isEmpty()) {
        KCalCore::Event::Ptr event( new KCalCore::Event );
#if 0 //QT5
        event->setDtStart(dtstart);
        event->setDtEnd(dtend);
#endif
        event->setSummary(mEventEdit->text());
        Q_EMIT createEvent(event, collection);
        mEventEdit->clear();
        hide();
    }
}

bool EventEdit::eventFilter(QObject *object, QEvent *e)
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

void EventEdit::slotStartDateTimeChanged(const KDateTime &newDateTime)
{
    if (!newDateTime.isValid()) {
      return;
    }

    if (mEndDateTimeEdit->date() == newDateTime.date() && mEndDateTimeEdit->time() < newDateTime.time()) {
        mEndDateTimeEdit->setTime(newDateTime.time());
    }
    if (mEndDateTimeEdit->date() < newDateTime.date()) {
        mEndDateTimeEdit->setDate(newDateTime.date());
    }

    //QT5 mEndDateTimeEdit->setMinimumDateTime(newDateTime);
}


void EventEdit::slotOpenEditor()
{
    KCalCore::Attachment::Ptr attachment(new KCalCore::Attachment(mMessage->encodedContent().toBase64(), KMime::Message::mimeType()));
    const KMime::Headers::Subject * const subject = mMessage->subject(false);
    if (subject) {
        attachment->setLabel(subject->asUnicodeString());
    }

    KCalCore::Event::Ptr event(new KCalCore::Event);
    event->setSummary(mEventEdit->text());
    //QT5 event->setDtStart(mStartDateTimeEdit->dateTime());
    //QT5 event->setDtEnd(mEndDateTimeEdit->dateTime());
    event->addAttachment(attachment);

    Akonadi::Item item;
    item.setPayload<KCalCore::Event::Ptr>(event);
    item.setMimeType(KCalCore::Event::eventMimeType());

    IncidenceEditorNG::IncidenceDialog *dlg = IncidenceEditorNG::IncidenceDialogFactory::create(true, KCalCore::IncidenceBase::TypeEvent, 0, this);
    connect(dlg, SIGNAL(finished()), this, SLOT(slotCloseWidget()));
    dlg->load(item);
    dlg->open();
}
