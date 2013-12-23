/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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


#include "notesmanager.h"
#include "notesharedglobalconfig.h"
#include "noteshared/network/notesnetworkreceiver.h"
#include "noteshared/job/createnewnotejob.h"
#include "noteshared/akonadi/noteschangerecorder.h"
#include "noteshared/akonadi/notesakonaditreemodel.h"
#include "noteshared/attributes/notealarmattribute.h"
#include "notesagentalarmdialog.h"

#include <Akonadi/Session>
#include <Akonadi/Collection>
#include <Akonadi/Item>
#include <Akonadi/ChangeRecorder>

#include <KMime/KMimeMessage>

#include <ksocketfactory.h>
#include <KNotification>
#include <KIconLoader>
#include <KLocalizedString>
#include <KIcon>

#include <QTcpServer>
#include <QTimer>

NotesManager::NotesManager(QObject *parent)
    : QObject(parent),
      mListener(0),
      mCheckAlarm(0)
{
    Akonadi::Session *session = new Akonadi::Session( "KNotes Session", this );
    mNoteRecorder = new NoteShared::NotesChangeRecorder(this);
    mNoteRecorder->changeRecorder()->setSession(session);
    mNoteTreeModel = new NoteShared::NotesAkonadiTreeModel(mNoteRecorder->changeRecorder(), this);

    connect( mNoteTreeModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
             SLOT(slotRowInserted(QModelIndex,int,int)));

    connect( mNoteRecorder->changeRecorder(), SIGNAL(itemChanged(Akonadi::Item,QSet<QByteArray>)), SLOT(slotItemChanged(Akonadi::Item,QSet<QByteArray>)));
    connect( mNoteRecorder->changeRecorder(), SIGNAL(itemRemoved(Akonadi::Item)), SLOT(slotItemRemoved(Akonadi::Item)) );
}

NotesManager::~NotesManager()
{
    clear();
}

void NotesManager::clear()
{
    delete mListener;
    mListener=0;
    if (mCheckAlarm->isActive())
        mCheckAlarm->stop();
}

void NotesManager::slotItemRemoved(const Akonadi::Item &item)
{
    if (mListItem.contains(item)) {
        mListItem.removeAll(item);
    }
}

void NotesManager::slotItemChanged(const Akonadi::Item &item, const QSet<QByteArray> &set)
{
    if (set.contains("ATR:NoteAlarmAttribute")) {
        mListItem.removeAll(item);
        mListItem.append(item);
    }
}

void NotesManager::slotRowInserted(const QModelIndex &parent, int start, int end)
{
    for ( int i = start; i <= end; ++i) {
        if ( mNoteTreeModel->hasIndex( i, 0, parent ) ) {
            const QModelIndex child = mNoteTreeModel->index( i, 0, parent );
            Akonadi::Item item =
                    mNoteTreeModel->data( child, Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
            if ( !item.hasPayload<KMime::Message::Ptr>() )
                continue;
            if ( item.hasAttribute<NoteShared::NoteAlarmAttribute>()) {
                mListItem.append(item);
            }
        }
    }
}

void NotesManager::slotCheckAlarm()
{
    QDateTime from = NoteShared::NoteSharedGlobalConfig::self()->alarmsLastChecked().addSecs( 1 );
    if ( !from.isValid() ) {
        from.setTime_t( 0 );
    }

    const KDateTime now = KDateTime::currentLocalDateTime();
    NoteShared::NoteSharedGlobalConfig::self()->setAlarmsLastChecked( now.dateTime() );


    Akonadi::Item::List lst;
    Q_FOREACH (const Akonadi::Item &item, mListItem) {
        NoteShared::NoteAlarmAttribute *attrAlarm = item.attribute<NoteShared::NoteAlarmAttribute>();
        if (attrAlarm) {
            if (attrAlarm->dateTime()< KDateTime::currentDateTime(KDateTime::LocalZone)) {
                lst.append(item);
            }
        }
    }
    if (!lst.isEmpty()) {
        if (!mAlarmDialog) {
            mAlarmDialog = new NotesAgentAlarmDialog;
        }
        mAlarmDialog->addListAlarm(lst);
        mAlarmDialog->show();
    }
    mCheckAlarm->start();
}

void NotesManager::load()
{
    updateNetworkListener();
    if (!mCheckAlarm)
        mCheckAlarm = new QTimer(this);
    if (mCheckAlarm->isActive())
        mCheckAlarm->stop();

    mCheckAlarm->setInterval(1000*60*NoteShared::NoteSharedGlobalConfig::checkInterval());
    connect(mCheckAlarm, SIGNAL(timeout()), this, SLOT(slotCheckAlarm()));
    //mCheckAlarm->start();
    slotCheckAlarm();
}

void NotesManager::stopAll()
{
    clear();
}

void NotesManager::slotAcceptConnection()
{
    // Accept the connection and make KNotesNetworkReceiver do the job
    QTcpSocket *s = mListener->nextPendingConnection();

    if ( s ) {
        NoteShared::NotesNetworkReceiver *recv = new NoteShared::NotesNetworkReceiver( s );
        connect( recv, SIGNAL(noteReceived(QString,QString)), SLOT(slotNewNote(QString,QString)) );
    }
}

void NotesManager::slotNewNote(const QString &name, const QString &text)
{
    const QPixmap pixmap = KIcon( QLatin1String("knotes") ).pixmap( KIconLoader::SizeSmall, KIconLoader::SizeSmall );

    KNotification::event( QLatin1String("receivednotes"),
                          i18n("Note Received"),
                          pixmap,
                          0,
                          KNotification::CloseOnTimeout,
                          KGlobal::mainComponent());

    NoteShared::CreateNewNoteJob *job = new NoteShared::CreateNewNoteJob(this, 0);
    //For the moment it doesn't support richtext.
    job->setRichText(false);
    job->setNote(name, text);
    job->start();
}

void NotesManager::updateNetworkListener()
{
    delete mListener;
    mListener=0;

    if ( NoteShared::NoteSharedGlobalConfig::receiveNotes() ) {
        // create the socket and start listening for connections
        mListener= KSocketFactory::listen( QLatin1String("knotes") , QHostAddress::Any,
                                           NoteShared::NoteSharedGlobalConfig::port() );
        connect( mListener, SIGNAL(newConnection()),
                 SLOT(slotAcceptConnection()) );
    }
}
