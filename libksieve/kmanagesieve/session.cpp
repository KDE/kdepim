/*
    Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    Author: Volker Krause <volker.krause@kdab.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "session.h"
#include "response.h"

#include <kdebug.h>
#include <ktcpsocket.h>

using namespace KManageSieve;

Session::Session( QObject *parent ) :
  QObject( parent ),
  m_socket( new KTcpSocket( this ) )
{
  kDebug();
  connect( m_socket, SIGNAL(readyRead()), SLOT(dataReceived()) );
  connect( m_socket, SIGNAL(error(KTcpSocket::Error)), SLOT(socketError()) );
  connect( m_socket, SIGNAL(disconnected()), SLOT(socketError()) );
}

Session::~Session()
{
  kDebug();
  delete m_socket;
}

void Session::connectToHost( const KUrl &url )
{
  kDebug() << url;
  if ( m_socket->state() == KTcpSocket::ConnectedState || m_socket->state() == KTcpSocket::ConnectingState )
    return;

  m_socket->connectToHost( url.host(), url.port() ? url.port() : 2000 );
}

void Session::disconnectFromHost()
{
  m_socket->disconnectFromHost();
}

void Session::dataReceived()
{
  kDebug();
  while ( m_socket->canReadLine() ) {
    const QByteArray line = m_socket->readLine();
    kDebug() << "S: " << m_socket->readLine();
    Response r;
    if ( !r.parseResponse( line ) ) {
      // protocol violation
      disconnectFromHost();
    }
    kDebug() << r.type() << r.key() << r.value() << r.extra() << r.quantity();
  }
}

void Session::socketError()
{
  kDebug() << m_socket->errorString();
}

void Session::scheduleJob(SieveJob* job)
{
  kDebug() << job;
}

#include "session.moc"
