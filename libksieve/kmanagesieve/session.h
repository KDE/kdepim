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

#ifndef KSIEVE_KMANAGESIEVE_SESSION_H
#define KSIEVE_KMANAGESIEVE_SESSION_H

#include "response.h"
#include "sasl-common.h"

#include <KUrl>
#include <QtCore/QObject>
#include <QtCore/QQueue>
#include <QStringList>

class QTimer;
class KTcpSocket;

namespace KManageSieve {

class SieveJob;

/** A network session with a manage sieve server.
 * @internal
 */
class Session : public QObject
{
    Q_OBJECT
public:
    explicit Session( QObject *parent = 0 );
    ~Session();

    void connectToHost( const KUrl &url );
    void disconnectFromHost( bool sendLogout = true );

    void scheduleJob( SieveJob *job );
    void killJob( SieveJob *job );
    void sendData( const QByteArray &data );
    void feedBack( const QByteArray &data );

    QString errorMessage() const;
    void setErrorMessage( const QString &msg );

    QStringList sieveExtensions() const;

private:
    bool requestCapabilitiesAfterStartTls() const;
    void startAuthentication();
    QStringList requestedSaslMethod() const;
    bool allowUnencrypted() const;
    bool saslInteract( void *in );
    bool saslClientStep( const QByteArray &challenge );
    void processResponse( const Response &response, const QByteArray &data );

private slots:
    void dataReceived();
    void socketError();
    void startSsl();
    void executeNextJob();
    void slotSslTimeout();
    void slotEncryptedDone();

private:
    void sslResult(bool encrypted);

    KUrl m_url;
    KTcpSocket *m_socket;
    sasl_conn_t *m_sasl_conn;
    sasl_interact_t *m_sasl_client_interact;
    QQueue<SieveJob*> m_jobs;
    SieveJob* m_currentJob;
    QStringList m_sieveExtensions;
    QStringList m_saslMethods;
    QString m_implementation;
    enum State {
        None,
        PreTlsCapabilities,
        PostTlsCapabilities,
        StartTls,
        Authenticating
    };
    QTimer *m_sslCheck;
    State m_state;
    Response m_lastResponse;
    QByteArray m_data;
    QString m_errorMsg;
    qint64 m_pendingQuantity;
    bool m_supportsStartTls;
};

}

#endif
