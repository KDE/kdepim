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
#include "sasl-common.h"

#include <kdebug.h>
#include <ktcpsocket.h>
#include <kio/sslui.h>
#include <kio/authinfo.h>
#include <KLocalizedString>
#include <KIO/PasswordDialog>

static sasl_callback_t callbacks[] = {
    { SASL_CB_ECHOPROMPT, NULL, NULL },
    { SASL_CB_NOECHOPROMPT, NULL, NULL },
    { SASL_CB_GETREALM, NULL, NULL },
    { SASL_CB_USER, NULL, NULL },
    { SASL_CB_AUTHNAME, NULL, NULL },
    { SASL_CB_PASS, NULL, NULL },
    { SASL_CB_CANON_USER, NULL, NULL },
    { SASL_CB_LIST_END, NULL, NULL }
};

using namespace KManageSieve;

Session::Session( QObject *parent ) :
  QObject( parent ),
  m_socket( new KTcpSocket( this ) ),
  m_state( None ),
  m_supportsStartTls( false )
{
  kDebug();
  connect( m_socket, SIGNAL(readyRead()), SLOT(dataReceived()) );
  connect( m_socket, SIGNAL(error(KTcpSocket::Error)), SLOT(socketError()) );
  connect( m_socket, SIGNAL(disconnected()), SLOT(socketError()) );

  static bool saslInitialized = false;
  if ( !saslInitialized ) {
    initSASL();
    saslInitialized = true;
  }
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

  m_url = url;
  m_socket->connectToHost( url.host(), url.port() ? url.port() : 2000 );
  m_state = PreTlsCapabilities;
}

void Session::disconnectFromHost()
{
  m_socket->disconnectFromHost();
}

void Session::dataReceived()
{
  while ( m_socket->canReadLine() ) {
    const QByteArray line = m_socket->readLine();
    kDebug() << "S: " << line;
    Response r;
    if ( !r.parseResponse( line ) ) {
      kDebug() << "protocol violation!";
      disconnectFromHost();
    }
    kDebug() << r.type() << r.key() << r.value() << r.extra() << r.quantity();

    // should probably be refactored into a capability job
    switch ( m_state ) {
      case PreTlsCapabilities:
      case PostTlsCapabilities:
        if ( r.type() == Response::Action ) {
          if ( r.action().toLower().contains("ok") ) {
            kDebug() << "Sieve server ready & awaiting authentication." << endl;
            if ( m_state == PreTlsCapabilities ) {
              m_state = StartTls;
              // TODO: handle the non-tls case
              sendData( "STARTTLS" );
            } else {
              m_state = Authenticating;
              startAuthentication();
            }
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
        break;
      case StartTls:
        if ( r.type() == Response::Action && r.operationSuccessful() ) {
          QMetaObject::invokeMethod( this, "startSsl", Qt::QueuedConnection ); // queued to avoid deadlock with waitForEncrypted
          m_state = None;
        }
        break;
      default:
        kDebug() << "Unhandled response!";
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

void Session::startSsl()
{
  kDebug();
  m_socket->setAdvertisedSslVersion( KTcpSocket::TlsV1 );
  m_socket->ignoreSslErrors();
  m_socket->startClientEncryption();
  const bool encrypted = m_socket->waitForEncrypted( 60 * 1000 );

  const KSslCipher cipher = m_socket->sessionCipher();
  if ( !encrypted || m_socket->sslErrors().count() > 0 || m_socket->encryptionMode() != KTcpSocket::SslClientMode
        || cipher.isNull() || cipher.usedBits() == 0 )
  {
    kDebug() << "Initial SSL handshake failed. cipher.isNull() is" << cipher.isNull()
              << ", cipher.usedBits() is" << cipher.usedBits()
              << ", the socket says:" <<  m_socket->errorString()
              << "and the list of SSL errors contains"
              << m_socket->sslErrors().count() << "items.";

    if ( !KIO::SslUi::askIgnoreSslErrors( m_socket ) ) {
      disconnectFromHost(); // TODO error handling
      return;
    }
  }
  kDebug() << "TLS negotiation done.";
  if ( requestCapabilitiesAfterStartTls() )
    sendData( "CAPABILITY" );
  m_state = PostTlsCapabilities;
}

void Session::sendData(const QByteArray& data)
{
  kDebug() << "C: " << data;
  m_socket->write( data );
  m_socket->write( "\r\n" );
}

void Session::startAuthentication()
{
  int result;
  sasl_conn_t *conn = NULL;
  sasl_interact_t *client_interact = NULL;
  const char *out = NULL;
  uint outlen;
  const char *mechusing = NULL;

  KIO::AuthInfo ai;
  ai.url = m_url;
  ai.username = m_url.user();
  ai.password = m_url.password();
  ai.keepPassword = true;
  ai.caption = i18n("Sieve Authentication Details");
  ai.comment = i18n("Please enter your authentication details for your sieve account "
    "(usually the same as your email password):");

  result = sasl_client_new( "sieve", m_url.host().toLatin1(), 0, 0, callbacks, 0, &conn );
  if ( result != SASL_OK ) {
    kDebug() << "sasl_client_new failed with: " << result << QString::fromUtf8( sasl_errdetail( conn ) );
    disconnectFromHost(); // TODO handle error
    return;
  }

  do {
    result = sasl_client_start( conn, requestedSaslMethod().join(" ").toLatin1(), &client_interact, &out, &outlen, &mechusing);
    if ( result == SASL_INTERACT ) {
      if ( !saslInteract( client_interact, ai ) ) {
        sasl_dispose( &conn );
        disconnectFromHost(); // TODO handle error
        return;
      }
    }
  } while ( result == SASL_INTERACT );

  if ( result != SASL_CONTINUE && result != SASL_OK ) {
    kDebug() << "sasl_client_start failed with: " << result << QString::fromUtf8( sasl_errdetail( conn ) );
    sasl_dispose( &conn );
    disconnectFromHost(); // TODO handle error;
    return;
  }

  kDebug() << "Preferred authentication method is " << mechusing << ".";

  QByteArray authCommand = "AUTHENTICATE \"" + QByteArray(mechusing) + QByteArray("\"");
  const QByteArray challenge = QByteArray::fromRawData( out, outlen ).toBase64();
  if ( !challenge.isEmpty() ) {
    authCommand += " \"";
    authCommand += challenge;
    authCommand += '\"';
  }
  sendData( authCommand );
}

QStringList Session::requestedSaslMethod() const
{
  const QString m = m_url.queryItem( "x-mech" );
  if ( !m.isEmpty() )
    return QStringList( m );
  return m_saslMethods;
}

bool Session::saslInteract(void* in, KIO::AuthInfo& ai)
{
  kDebug();
  sasl_interact_t *interact = ( sasl_interact_t * ) in;

  //some mechanisms do not require username && pass, so it doesn't need a popup
  //window for getting this info
  for ( ; interact->id != SASL_CB_LIST_END; interact++ ) {
    if ( interact->id == SASL_CB_AUTHNAME || interact->id == SASL_CB_PASS ) {
      if ( m_url.user().isEmpty() || m_url.password().isEmpty()) {
        if ( !KIO::PasswordDialog::getNameAndPassword(
              ai.username,
              ai.password,
              &(ai.keepPassword),
              ai.prompt,
              ai.readOnly,
              ai.caption,
              ai.comment,
              ai.commentLabel
            ) == KIO::PasswordDialog::Accepted )
        {
          // calling error() below is wrong for two reasons:
          // - ERR_ABORTED is too harsh
          // - higher layers already call error() and that can't happen twice.
                //error(ERR_ABORTED, i18n("No authentication details supplied."));
                return false;
        }
        m_url.setUserName( ai.username );
        m_url.setPassword( ai.password );
      }
      break;
    }
  }

  interact = ( sasl_interact_t * ) in;
  while( interact->id != SASL_CB_LIST_END ) {
    kDebug() << "SASL_INTERACT id: " << interact->id;
    switch( interact->id ) {
      case SASL_CB_USER:
      case SASL_CB_AUTHNAME:
        kDebug() << "SASL_CB_[AUTHNAME|USER]: '" << m_url.userName() << "'";
        interact->result = strdup( m_url.userName().toUtf8() );
        interact->len = strlen( (const char *) interact->result );
        break;
      case SASL_CB_PASS:
        kDebug() << "SASL_CB_PASS: [hidden] ";
        interact->result = strdup( m_url.password().toUtf8() );
        interact->len = strlen( (const char *) interact->result );
        break;
      default:
        interact->result = NULL; interact->len = 0;
        break;
    }
    interact++;
  }
  return true;
}


#include "session.moc"
