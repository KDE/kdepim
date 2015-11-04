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

#include <QUrl>
#include <QObject>
#include <QQueue>
#include <QStringList>

class KSslErrorUiData;

namespace KManageSieve
{

class SieveJob;
class SessionThread;

struct AuthDetails {
    QString username;
    QString password;
    bool valid;
};

/** A network session with a manage sieve server.
 * @internal
 */
class Session : public QObject
{
    Q_OBJECT

public:
    explicit Session(QObject *parent = Q_NULLPTR);
    ~Session();

    void connectToHost(const QUrl &url);
    void disconnectFromHost(bool sendLogout = true);

    void scheduleJob(SieveJob *job);
    void killJob(SieveJob *job);
    void sendData(const QByteArray &data);
    void feedBack(const QByteArray &data);

    QString errorMessage() const;
    void setErrorMessage(const QString &msg);

    QStringList sieveExtensions() const;

private:
    bool requestCapabilitiesAfterStartTls() const;

    QStringList requestedSaslMethod() const;
    bool allowUnencrypted() const;

private Q_SLOTS:
    void processResponse(const KManageSieve::Response &response, const QByteArray &data);
    KManageSieve::AuthDetails requestAuthDetails(const QUrl &url);
    void authenticationDone();
    void sslError(const KSslErrorUiData &data);
    void sslDone();

    void executeNextJob();

private:
    SessionThread *m_thread;
    QUrl m_url;
    QQueue<SieveJob *> m_jobs;
    SieveJob *m_currentJob;
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
    QString m_errorMsg;
    State m_state;
    bool m_supportsStartTls;
    bool m_connected;

    friend class SessionThread;
};

}

#endif
