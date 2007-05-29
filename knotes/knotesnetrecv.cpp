/*******************************************************************
 KNotes -- Notes for the KDE project

 Copyright (c) 2003, Daniel Martin <daniel.martin@pirack.com>
               2004, 2006, Michael Brade <brade@kde.org>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

 In addition, as a special exception, the copyright holders give
 permission to link the code of this program with any edition of
 the Qt library by Trolltech AS, Norway (or with modified versions
 of Qt that use the same license as Qt), and distribute linked
 combinations including the two.  You must obey the GNU General
 Public License in all respects for all of the code used other than
 Qt.  If you modify this file, you may extend this exception to
 your version of the file, but you are not obligated to do so.  If
 you do not wish to do so, delete this exception statement from
 your version.
*******************************************************************/

#include <qtimer.h>
#include <qdatetime.h>
#include <qregexp.h>
#include <qcstring.h>

#include <kdebug.h>
#include <kbufferedsocket.h>
#include <ksocketaddress.h>
#include <klocale.h>
#include <kglobal.h>

#include "knotesnetrecv.h"

// Maximum note size in chars we are going to accept,
// to prevent "note floods".
#define MAXBUFFER 4096

// Maximum time we are going to wait between data receptions,
// to prevent memory and connection floods. In milliseconds.
#define MAXTIME 10000

// Small buffer's size
#define SBSIZE 512

using namespace KNetwork;


KNotesNetworkReceiver::KNotesNetworkReceiver( KBufferedSocket *s )
  : QObject(),
    m_buffer( new QByteArray() ), m_sock( s )
{
    QString date = KGlobal::locale()->formatDateTime( QDateTime::currentDateTime(), true, false );

    // Add the remote IP or hostname and the date to the title, to help the
    // user guess who wrote it.
    m_titleAddon = QString(" [%1, %2]")
                   .arg( m_sock->peerAddress().nodeName() )
                   .arg( date );

    // Setup the communications
    connect( m_sock, SIGNAL(readyRead()), SLOT(slotDataAvailable()) );
    connect( m_sock, SIGNAL(closed()), SLOT(slotConnectionClosed()) );
    connect( m_sock, SIGNAL(gotError( int )), SLOT(slotError( int )) );

    m_sock->enableRead( true );

    // Setup the timer
    m_timer = new QTimer( this, "m_timer" );
    connect( m_timer, SIGNAL(timeout()), SLOT(slotReceptionTimeout()) );
    m_timer->start( MAXTIME, true );
}

KNotesNetworkReceiver::~KNotesNetworkReceiver()
{
    delete m_buffer;
    delete m_sock;
}

void KNotesNetworkReceiver::slotDataAvailable()
{
    char smallBuffer[SBSIZE];
    int smallBufferLen;

    do
    {
        // Append to "big buffer" only if we have some space left.
        int curLen = m_buffer->count();

        smallBufferLen = m_sock->readBlock( smallBuffer, SBSIZE );

        // Limit max transfer over buffer, to avoid overflow.
        smallBufferLen = kMin( smallBufferLen, MAXBUFFER - curLen );
        
        if ( smallBufferLen > 0 )
        {
            m_buffer->resize( curLen + smallBufferLen );
            memcpy( m_buffer->data() + curLen, smallBuffer, smallBufferLen );
        }
    }
    while ( smallBufferLen == SBSIZE );

    // If we are overflowing, close connection.
    if ( m_buffer->count() == MAXBUFFER )
        m_sock->close();
    else
        m_timer->changeInterval( MAXTIME );
}

void KNotesNetworkReceiver::slotReceptionTimeout()
{
    m_sock->close();
}

void KNotesNetworkReceiver::slotConnectionClosed()
{
    if ( m_timer->isActive() )
    {
        QString noteText = QString( *m_buffer ).stripWhiteSpace();

        // First line is the note title or, in case of ATnotes, the id
        int pos = noteText.find( QRegExp("[\r\n]") );
        QString noteTitle = noteText.left( pos ).stripWhiteSpace() + m_titleAddon;

        noteText = noteText.mid( pos ).stripWhiteSpace();

        if ( !noteText.isEmpty() )
            emit sigNoteReceived( noteTitle, noteText );
    }

    deleteLater();
}

void KNotesNetworkReceiver::slotError( int err )
{
    kdWarning() << k_funcinfo
                << m_sock->errorString( static_cast<KSocketBase::SocketError>(err) ) 
                << endl;
}

#include "knotesnetrecv.moc"
