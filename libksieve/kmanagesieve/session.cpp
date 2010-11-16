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
  m_socket( new KTcpSocket( this ) ),
  m_parsingCapabilities( false ),
  m_supportsStartTls( false )
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
  m_parsingCapabilities = true;
}

void Session::disconnectFromHost()
{
  m_socket->disconnectFromHost();
}

void Session::dataReceived()
{
  while ( m_socket->canReadLine() ) {
    const QByteArray line = m_socket->readLine();
    kDebug() << "S: " << m_socket->readLine();
    Response r;
    if ( !r.parseResponse( line ) ) {
      // protocol violation
      disconnectFromHost();
    }
    kDebug() << r.type() << r.key() << r.value() << r.extra() << r.quantity();

    // should probably be refactored into a capability job
    if ( m_parsingCapabilities ) {
      if ( r.type() == Response::Action ) {
        if ( r.action().toLower().contains("ok") ) {
          kDebug() << "Sieve server ready & awaiting authentication." << endl;
          m_parsingCapabilities = false;
        } else {
          kDebug() << "Unknown action " << r.action() << "." << endl;
        }
      } else if ( r.key() == "IMPLEMENTATION" ) {
        m_implementation = QString::fromLatin1( r.value() );
        kDebug() << "Connected to Sieve server: " << r.value();
      } else if ( r.key() == "SASL") {
        m_saslMethods = QString::fromLatin1( r.value() ).split( ' ', QString::SkipEmptyParts );
        kDebug() << "Server SASL authentication methods: " << m_saslMethods;
      } else if ( r.key() == "SIEVE" ) {
        // Save script capabilities
        m_sieveExtensions = QString::fromLatin1( r.value() ).split( ' ', QString::SkipEmptyParts );
        kDebug() << "Server script capabilities: " << m_sieveExtensions;
      } else if (r.key() == "STARTTLS") {
        kDebug() << "Server supports TLS";
        m_supportsStartTls = true;
      } else {
        kDebug() << "Unrecognised key " << r.key() << endl;
      }
    }
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

QStringList Session::sieveExtensions() const
{
  return m_sieveExtensions;
}

bool Session::requestCapabilitiesAfterStartTls() const
{
  // Cyrus didn't send CAPABILITIES after STARTTLS until 2.3.11, which is
  // not standard conform, but we need to support that anyway.
  // m_implementation looks like this 'Cyrus timsieved v2.2.12' for Cyrus btw.
  QRegExp regExp( "Cyrus\\stimsieved\\sv(\\d+)\\.(\\d+)\\.(\\d+)([-\\w]*)", Qt::CaseInsensitive );
  if ( regExp.indexIn( m_implementation ) >= 0 ) {
    const int major = regExp.cap( 1 ).toInt();
    const int minor = regExp.cap( 2 ).toInt();
    const int patch = regExp.cap( 3 ).toInt();
    const QString vendor = regExp.cap( 4 );
    if ( major < 2 || (major == 2 && (minor < 3 || (minor == 3 && patch < 11))) || (vendor == "-kolab-nocaps") ) {
      kDebug() << "Enabling compat mode for Cyrus < 2.3.11 or Cyrus marked as \"kolab-nocaps\"";
      return true;
    }
  }
  return false;
}

#include "session.moc"
