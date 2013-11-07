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

//krazy:excludeall=null since used by SASL (C library)

#include "session.h"
#include "response.h"
#include "sievejob_p.h"

#include <kdebug.h>
#include <ktcpsocket.h>
#include <kio/sslui.h>
#include <kio/authinfo.h>
#include <KLocalizedString>
#include <KPasswordDialog>
#include <KMessageBox>

#include <QTimer>

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
    m_sasl_conn( 0 ),
    m_sasl_client_interact( 0 ),
    m_currentJob( 0 ),
    m_sslCheck(0),
    m_state( None ),
    m_pendingQuantity( -1 ),
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
    disconnectFromHost( false );
    delete m_socket;
    delete m_sslCheck;
}

void Session::connectToHost( const KUrl &url )
{
    kDebug() << url;
    if ( m_socket->state() == KTcpSocket::ConnectedState || m_socket->state() == KTcpSocket::ConnectingState )
        return;

    m_url = url;
    m_socket->connectToHost( url.host(), url.port() ? url.port() : 4190 );
    m_state = PreTlsCapabilities;
}

void Session::disconnectFromHost( bool sendLogout )
{
    if ( sendLogout )
        sendData( "LOGOUT" );
    m_socket->disconnectFromHost();
    if ( m_currentJob )
        killJob( m_currentJob );
    foreach ( SieveJob* job, m_jobs )
        killJob( job );
    deleteLater();
}

void Session::dataReceived()
{
    if ( m_pendingQuantity > 0 ) {
        const QByteArray buffer = m_socket->read( qMin( m_pendingQuantity, m_socket->bytesAvailable() ) );
        m_data += buffer;
        m_pendingQuantity -= buffer.size();
        if ( m_pendingQuantity <= 0 ) {
            kDebug() << "S: " << m_data.trimmed();
            processResponse( m_lastResponse, m_data );
        }
    }

    while ( m_socket->canReadLine() ) {
        QByteArray line = m_socket->readLine();
        if ( line.endsWith( "\r\n" ) ) { //krazy:exclude=strings
            line.chop( 2 );
        }
        if ( line.isEmpty() ) {
            continue; // ignore CRLF after data blocks
        }
        kDebug() << "S: " << line;
        Response r;
        if ( !r.parseResponse( line ) ) {
            kDebug() << "protocol violation!";
            disconnectFromHost( false );
        }
        kDebug() << r.type() << r.key() << r.value() << r.extra() << r.quantity();

        m_lastResponse = r;
        if ( r.type() == Response::Quantity ) {
            m_data.clear();
            m_pendingQuantity = r.quantity();
            dataReceived(); // in case the data block is already completely in the buffer
            return;
        } else if ( r.operationResult() == Response::Bye ) {
            disconnectFromHost( false );
            return;
        }
        processResponse( r, QByteArray() );
    }
}

void Session::feedBack(const QByteArray &data)
{
    Response response;
    if ( !response.parseResponse( data ) ) {
        m_errorMsg = KIO::buildErrorString( KIO::ERR_UNKNOWN, i18n( "Syntax error." ) );
        disconnectFromHost();
        return;
    }
    m_lastResponse = response;

    if ( response.type() == Response::Quantity ) {
        m_data.clear();
        m_pendingQuantity = response.quantity();
        dataReceived();
        return;
    } else {
        processResponse( response, QByteArray() );
    }
}

void Session::processResponse(const KManageSieve::Response &response, const QByteArray &data)
{
    switch ( m_state ) {
    // should probably be refactored into a capability job
    case PreTlsCapabilities:
    case PostTlsCapabilities:
        if ( response.type() == Response::Action ) {
            if ( response.operationSuccessful() ) {
                kDebug() << "Sieve server ready & awaiting authentication.";
                if ( m_state == PreTlsCapabilities ) {
                    if ( !allowUnencrypted() && !QSslSocket::supportsSsl() ) {
                        m_errorMsg = KIO::buildErrorString( KIO::ERR_SLAVE_DEFINED, i18n("Cannot use TLS since the underlying Qt library does not support it.") );
                        disconnectFromHost();
                        return;
                    }
                    if ( !allowUnencrypted() && QSslSocket::supportsSsl() && !m_supportsStartTls &&
                         KMessageBox::warningContinueCancel( 0,
                                                             i18n("TLS encryption was requested, but your Sieve server does not advertise TLS in its capabilities.\n"
                                                                  "You can choose to try to initiate TLS negotiations nonetheless, or cancel the operation."),
                                                             i18n("Server Does Not Advertise TLS"), KGuiItem(i18n("&Start TLS nonetheless")), KStandardGuiItem::cancel(),
                                                             QString::fromLatin1( "ask_starttls_%1" ).arg(m_url.host() )  ) != KMessageBox::Continue )
                    {
                        m_errorMsg = KIO::buildErrorString( KIO::ERR_USER_CANCELED, i18n("TLS encryption requested, but not supported by server.") );
                        disconnectFromHost();
                        return;
                    }

                    if ( m_supportsStartTls && QSslSocket::supportsSsl() ) {
                        m_state = StartTls;
                        sendData( "STARTTLS" );
                    } else {
                        m_state = Authenticating;
                        startAuthentication();
                    }
                } else {
                    m_state = Authenticating;
                    startAuthentication();
                }
            } else {
                kDebug() << "Unknown action " << response.action() << ".";
            }
        } else if ( response.key() == "IMPLEMENTATION" ) {
            m_implementation = QString::fromLatin1( response.value() );
            kDebug() << "Connected to Sieve server: " << response.value();
        } else if ( response.key() == "SASL") {
            m_saslMethods = QString::fromLatin1( response.value() ).split( ' ', QString::SkipEmptyParts );
            kDebug() << "Server SASL authentication methods: " << m_saslMethods;
        } else if ( response.key() == "SIEVE" ) {
            // Save script capabilities
            m_sieveExtensions = QString::fromLatin1( response.value() ).split( ' ', QString::SkipEmptyParts );
            kDebug() << "Server script capabilities: " << m_sieveExtensions;
        } else if (response.key() == "STARTTLS") {
            kDebug() << "Server supports TLS";
            m_supportsStartTls = true;
        } else {
            kDebug() << "Unrecognised key " << response.key();
        }
        break;
    case StartTls:
        if ( response.operationSuccessful() ) {
            QMetaObject::invokeMethod( this, "startSsl", Qt::QueuedConnection ); // queued to avoid deadlock with waitForEncrypted
            m_state = None;
        } else {
            m_errorMsg = KIO::buildErrorString( KIO::ERR_SLAVE_DEFINED, i18n("The server does not seem to support TLS. Disable TLS if you want to connect without encryption.") );
            disconnectFromHost();
        }
        break;
    case Authenticating:
        if ( response.operationResult() == Response::Other ) {
            if ( !saslClientStep( data ) ) {
                m_errorMsg = KIO::buildErrorString( KIO::ERR_COULD_NOT_AUTHENTICATE, QString::fromUtf8( sasl_errdetail( m_sasl_conn ) ) );
                disconnectFromHost();
                return;
            }
        } else {
            m_state = None;
            sasl_dispose( &m_sasl_conn );
            if ( response.operationSuccessful() ) {
                kDebug() << "Authentication complete.";
                QMetaObject::invokeMethod( this, "executeNextJob", Qt::QueuedConnection );
            } else {
                KIO::buildErrorString( KIO::ERR_COULD_NOT_AUTHENTICATE, i18n("Authentication failed.\nMost likely the password is wrong.\nThe server responded:\n%1", QString::fromLatin1( response.action() ) ) );
                disconnectFromHost();
                return;
            }
        }
        break;
    default:
        if ( m_currentJob ) {
            if ( m_currentJob->d->handleResponse( response, data ) ) {
                m_currentJob = 0;
                QMetaObject::invokeMethod( this, "executeNextJob", Qt::QueuedConnection );
            }
            break;
        } else {
            // we can get here in the kill current job case
            if ( response.operationResult() != Response::Other ) {
                QMetaObject::invokeMethod( this, "executeNextJob", Qt::QueuedConnection );
                return;
            }
        }
        kDebug() << "Unhandled response!";
    }
}

void Session::socketError()
{
    kDebug() << m_socket->errorString();
    disconnectFromHost( false );
}

void Session::scheduleJob(SieveJob *job)
{
    kDebug() << job;
    m_jobs.enqueue( job );
    QMetaObject::invokeMethod( this, "executeNextJob", Qt::QueuedConnection );
}

void Session::killJob(SieveJob* job)
{
    kDebug() << job;
    if ( m_currentJob == job ) {
        m_currentJob->d->killed();
        m_currentJob = 0;
    } else {
        m_jobs.removeAll( job );
        job->d->killed();
    }
}

void Session::executeNextJob()
{
    if ( m_socket->state() != KTcpSocket::ConnectedState || m_state != None || m_currentJob || m_jobs.isEmpty() )
        return;
    m_currentJob = m_jobs.dequeue();
    m_currentJob->d->run( this );
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

void Session::slotEncryptedDone()
{
    m_sslCheck->stop();
    sslResult(true);
}

void Session::sslResult(bool encrypted)
{
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
            disconnectFromHost();
            return;
        }
    }
    kDebug() << "TLS negotiation done.";
    if ( requestCapabilitiesAfterStartTls() )
        sendData( "CAPABILITY" );
    m_state = PostTlsCapabilities;
}

void Session::slotSslTimeout()
{
    disconnect(m_socket, SIGNAL(encrypted()), this, SLOT(slotEncryptedDone()));
    sslResult(false);
}

void Session::startSsl()
{
    kDebug();
    if (!m_sslCheck) {
        m_sslCheck = new QTimer;
        m_sslCheck->setInterval(60*1000);
        connect(m_sslCheck, SIGNAL(timeout()), this, SLOT(slotSslTimeout()));
    }
    m_socket->setAdvertisedSslVersion( KTcpSocket::TlsV1 );
    m_socket->ignoreSslErrors();
    connect(m_socket, SIGNAL(encrypted()), SLOT(slotEncryptedDone()));
    m_sslCheck->start();
    m_socket->startClientEncryption();
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
    m_sasl_conn = NULL;
    m_sasl_client_interact = NULL;
    const char *out = NULL;
    uint outlen;
    const char *mechusing = NULL;

    result = sasl_client_new( "sieve", m_url.host().toLatin1(), 0, 0, callbacks, 0, &m_sasl_conn );
    if ( result != SASL_OK ) {
        m_errorMsg = KIO::buildErrorString( KIO::ERR_COULD_NOT_AUTHENTICATE, QString::fromUtf8( sasl_errdetail( m_sasl_conn ) ) );
        disconnectFromHost();
        return;
    }

    do {
        result = sasl_client_start( m_sasl_conn, requestedSaslMethod().join(" ").toLatin1(), &m_sasl_client_interact, &out, &outlen, &mechusing);
        if ( result == SASL_INTERACT ) {
            if ( !saslInteract( m_sasl_client_interact ) ) {
                m_errorMsg = KIO::buildErrorString( KIO::ERR_COULD_NOT_AUTHENTICATE, QString::fromUtf8( sasl_errdetail( m_sasl_conn ) ) );
                sasl_dispose( &m_sasl_conn );
                disconnectFromHost();
                return;
            }
        }
    } while ( result == SASL_INTERACT );

    if ( result != SASL_CONTINUE && result != SASL_OK ) {
        m_errorMsg = KIO::buildErrorString( KIO::ERR_COULD_NOT_AUTHENTICATE, QString::fromUtf8( sasl_errdetail( m_sasl_conn ) ) );
        sasl_dispose( &m_sasl_conn );
        disconnectFromHost();
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
    const QString m = m_url.queryItem( QLatin1String("x-mech") );
    if ( !m.isEmpty() )
        return QStringList( m );
    return m_saslMethods;
}

bool Session::saslInteract(void* in)
{
    kDebug();
    sasl_interact_t *interact = ( sasl_interact_t * ) in;

    KIO::AuthInfo ai;
    ai.url = m_url;
    ai.username = m_url.userName();
    ai.password = m_url.password();
    ai.keepPassword = true;
    ai.caption = i18n("Sieve Authentication Details");
    ai.comment = i18n("Please enter your authentication details for your sieve account "
                      "(usually the same as your email password):");

    //some mechanisms do not require username && pass, so it doesn't need a popup
    //window for getting this info
    for ( ; interact->id != SASL_CB_LIST_END; interact++ ) {
        if ( interact->id == SASL_CB_AUTHNAME || interact->id == SASL_CB_PASS ) {
            if ( ai.username.isEmpty() || ai.password.isEmpty()) {

                QPointer<KPasswordDialog> dlg =
                        new KPasswordDialog(
                            0,
                            KPasswordDialog::ShowUsernameLine | KPasswordDialog::ShowKeepPassword
                            );
                dlg->setUsername( ai.username );
                dlg->setPassword( ai.password );
                dlg->setKeepPassword( ai.keepPassword );
                dlg->setPrompt( ai.prompt );
                dlg->setUsernameReadOnly( ai.readOnly );
                dlg->setCaption( ai.caption );
                dlg->addCommentLine( ai.commentLabel, ai.comment );

                bool gotIt = false;
                if ( dlg->exec() ) {
                    m_url.setUserName( dlg->username() );
                    m_url.setPassword( dlg->password() );
                    gotIt = true;
                }
                delete dlg;
                if ( !gotIt ) {
                    return false;
                }
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
            if (interact->result )
                interact->len = strlen( (const char *) interact->result );
            else
                interact->len = 0;
            break;
        case SASL_CB_PASS:
            kDebug() << "SASL_CB_PASS: [hidden] ";
            interact->result = strdup( m_url.password().toUtf8() );
            if (interact->result )
                interact->len = strlen( (const char *) interact->result );
            else
                interact->len = 0;
            break;
        default:
            interact->result = NULL;
            interact->len = 0;
            break;
        }
        interact++;
    }
    return true;
}

bool Session::saslClientStep(const QByteArray &challenge)
{
    int result;
    const char *out = NULL;
    uint outlen;

    const QByteArray challenge_decoded = QByteArray::fromBase64( challenge );
    do {
        result =
                sasl_client_step( m_sasl_conn,
                                  challenge_decoded.isEmpty() ? 0 : challenge_decoded.data(),
                                  challenge_decoded.size(),
                                  &m_sasl_client_interact,
                                  &out, &outlen );
        if ( result == SASL_INTERACT ) {
            if ( !saslInteract( m_sasl_client_interact ) ) {
                sasl_dispose( &m_sasl_conn );
                return false;
            }
        }
    } while ( result == SASL_INTERACT );

    kDebug() << "sasl_client_step: " << result;
    if ( result != SASL_CONTINUE && result != SASL_OK ) {
        kDebug() << "sasl_client_step failed with: " << result << QString::fromUtf8( sasl_errdetail( m_sasl_conn ) );
        sasl_dispose( &m_sasl_conn );
        return false;
    }

    sendData('\"' + QByteArray::fromRawData( out, outlen ).toBase64() + '\"');
    return true;
}

QString Session::errorMessage() const
{
    return m_errorMsg;
}

void Session::setErrorMessage(const QString& msg)
{
    m_errorMsg = msg;
}

bool Session::allowUnencrypted() const
{
    return m_url.queryItem(QLatin1String("x-allow-unencrypted")) == QLatin1String("true");
}

