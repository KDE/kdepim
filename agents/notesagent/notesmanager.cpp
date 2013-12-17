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
}

NotesManager::~NotesManager()
{
    clear();
}

void NotesManager::clear()
{
    delete mListener;
    mListener=0;
}

void NotesManager::slotCheckAlarm()
{
    //TODO check alarm
    //TODO retart it.
    mCheckAlarm->start();
}

void NotesManager::load(bool forced)
{
    updateNetworkListener();
    if (!mCheckAlarm)
        mCheckAlarm = new QTimer(this);
    if (mCheckAlarm->isActive())
        mCheckAlarm->stop();

    mCheckAlarm->setInterval(1000*60*NoteShared::NoteSharedGlobalConfig::checkInterval());
    mCheckAlarm->start();
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
