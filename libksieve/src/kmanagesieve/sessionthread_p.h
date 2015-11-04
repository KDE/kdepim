/*
    Copyright (c) 2015 Daniel Vrátil <dvratil@kde.org>

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

#ifndef KMANAGESIEVE_SESSIONTHREAD_H
#define KMANAGESIEVE_SESSIONTHREAD_H

#include <QtCore/QObject>
#include <QtCore/QUrl>

#include <KIO/Global>
#include <KTcpSocket>

#include "sasl-common.h"
#include "response.h"

class QTimer;

namespace KManageSieve
{

class Session;

class SessionThread : public QObject
{
    Q_OBJECT

public:
    explicit SessionThread(Session *session, QObject *parent = Q_NULLPTR);
    ~SessionThread();

    void connectToHost(const QUrl &url);
    void disconnectFromHost(bool sendLogout);

    void sendData(const QByteArray &data);
    void feedBack(const QByteArray &data);

    void startAuthentication();
    void continueAuthentication(const Response &response, const QByteArray &data);

    void startSsl();
    void handleSslErrorResponse(bool response);

Q_SIGNALS:
    void responseReceived(const KManageSieve::Response &response, const QByteArray &data);
    void error(const QString &error);
    void authenticationDone();
    void sslDone();
    void sslError(const KSslErrorUiData &data);

    void socketConnected();
    void socketDisconnected();

private Q_SLOTS:
    void doInit();
    void doDestroy();
    void doConnectToHost(const QUrl &url);
    void doDisconnectFromHost(bool sendLogout);
    void doSendData(const QByteArray &data);
    void doFeedBack(const QByteArray &data);
    void doStartAuthentication();
    void doContinueAuthentication(const KManageSieve::Response &response, const QByteArray &data);
    void doStartSsl();
    void doHandleSslErrorResponse(bool response);

    void slotDataReceived();
    void slotSocketError();
    void slotSslTimeout();
    void slotEncryptedDone();

private:
    bool saslInteract(void *in);
    bool saslClientStep(const QByteArray &challenge);
    void sslResult(bool encrypted);

private:
    Session *m_session;
    KTcpSocket *m_socket;

    QUrl m_url;

    sasl_conn_t *m_sasl_conn;
    sasl_interact_t *m_sasl_client_interact;

    QByteArray m_data;
    Response m_lastResponse;
    qint64 m_pendingQuantity;

    QTimer *m_sslCheck;
};

} // namespace KManageSieve

#endif // KMANAGESIEVE_SESSIONTHREAD_H
