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
#include "sessionthread_p.h"

#include "kmanagersieve_debug.h"
#include <ktcpsocket.h>
#include <kio/sslui.h>
#include <kio/authinfo.h>
#include <KLocalizedString>
#include <KPasswordDialog>
#include <KMessageBox>
#include <kio/job.h>
#include <QRegularExpression>
#include <QTimer>

using namespace KManageSieve;

Q_DECLARE_METATYPE(KManageSieve::AuthDetails)
Q_DECLARE_METATYPE(KManageSieve::Response)
Q_DECLARE_METATYPE(KSslErrorUiData)

Session::Session(QObject *parent) :
    QObject(parent),
    m_thread(new SessionThread(this)),
    m_currentJob(Q_NULLPTR),
    m_state(None),
    m_supportsStartTls(false),
    m_connected(false)
{
    qRegisterMetaType<KManageSieve::AuthDetails>();
    qRegisterMetaType<KManageSieve::Response>();
    qRegisterMetaType<KSslErrorUiData>();

    connect(m_thread, &SessionThread::responseReceived,
            this, &Session::processResponse);
    connect(m_thread, &SessionThread::error,
            this, &Session::setErrorMessage);
    connect(m_thread, &SessionThread::authenticationDone,
            this, &Session::authenticationDone);
    connect(m_thread, &SessionThread::sslError,
            this, &Session::sslError);
    connect(m_thread, &SessionThread::sslDone,
            this, &Session::sslDone);
    connect(m_thread, &SessionThread::socketConnected,
    [ = ]() {
        m_connected = true;
    });
    connect(m_thread, &SessionThread::socketDisconnected,
    [ = ]() {
        m_connected = false;
    });
}

Session::~Session()
{
    qCDebug(KMANAGERSIEVE_LOG) << Q_FUNC_INFO;
    delete m_thread;
}

void Session::connectToHost(const QUrl &url)
{
    qCDebug(KMANAGERSIEVE_LOG) << "connect to host url: " << url;
    m_url = url;
    m_thread->connectToHost(url);
    m_state = PreTlsCapabilities;
}

void Session::disconnectFromHost(bool sendLogout)
{
    m_thread->disconnectFromHost(sendLogout);
    if (m_currentJob) {
        killJob(m_currentJob);
    }
    Q_FOREACH (SieveJob *job, m_jobs) {
        killJob(job);
    }
    deleteLater();
}

void Session::feedBack(const QByteArray &data)
{
    Response response;
    if (!response.parseResponse(data)) {
        m_errorMsg = KIO::buildErrorString(KIO::ERR_UNKNOWN, i18n("Syntax error."));
        disconnectFromHost();
        return;
    }

    m_thread->feedBack(data);
}

void Session::processResponse(const KManageSieve::Response &response, const QByteArray &data)
{
    switch (m_state) {
    // should probably be refactored into a capability job
    case PreTlsCapabilities:
    case PostTlsCapabilities:
        if (response.type() == Response::Action) {
            if (response.operationSuccessful()) {
                qCDebug(KMANAGERSIEVE_LOG) << "Sieve server ready & awaiting authentication.";
                if (m_state == PreTlsCapabilities) {
                    if (!allowUnencrypted() && !QSslSocket::supportsSsl()) {
                        m_errorMsg = KIO::buildErrorString(KIO::ERR_SLAVE_DEFINED, i18n("Cannot use TLS since the underlying Qt library does not support it."));
                        disconnectFromHost();
                        return;
                    }
                    if (!allowUnencrypted() && QSslSocket::supportsSsl() && !m_supportsStartTls &&
                            KMessageBox::warningContinueCancel(Q_NULLPTR,
                                    i18n("TLS encryption was requested, but your Sieve server does not advertise TLS in its capabilities.\n"
                                         "You can choose to try to initiate TLS negotiations nonetheless, or cancel the operation."),
                                    i18n("Server Does Not Advertise TLS"), KGuiItem(i18n("&Start TLS nonetheless")), KStandardGuiItem::cancel(),
                                    QStringLiteral("ask_starttls_%1").arg(m_url.host())) != KMessageBox::Continue) {
                        m_errorMsg = KIO::buildErrorString(KIO::ERR_USER_CANCELED, i18n("TLS encryption requested, but not supported by server."));
                        disconnectFromHost();
                        return;
                    }

                    if (m_supportsStartTls && QSslSocket::supportsSsl()) {
                        m_state = StartTls;
                        sendData("STARTTLS");
                    } else {
                        m_state = Authenticating;
                        m_thread->startAuthentication();
                    }
                } else {
                    m_state = Authenticating;
                    m_thread->startAuthentication();
                }
            } else {
                qCDebug(KMANAGERSIEVE_LOG) << "Unknown action " << response.action() << ".";
            }
        } else if (response.key() == "IMPLEMENTATION") {
            m_implementation = QString::fromLatin1(response.value());
            qCDebug(KMANAGERSIEVE_LOG) << "Connected to Sieve server: " << response.value();
        } else if (response.key() == "SASL") {
            m_saslMethods = QString::fromLatin1(response.value()).split(' ', QString::SkipEmptyParts);
            qCDebug(KMANAGERSIEVE_LOG) << "Server SASL authentication methods: " << m_saslMethods;
        } else if (response.key() == "SIEVE") {
            // Save script capabilities
            m_sieveExtensions = QString::fromLatin1(response.value()).split(' ', QString::SkipEmptyParts);
            qCDebug(KMANAGERSIEVE_LOG) << "Server script capabilities: " << m_sieveExtensions;
        } else if (response.key() == "STARTTLS") {
            qCDebug(KMANAGERSIEVE_LOG) << "Server supports TLS";
            m_supportsStartTls = true;
        } else {
            qCDebug(KMANAGERSIEVE_LOG) << "Unrecognised key " << response.key();
        }
        break;
    case StartTls:
        if (response.operationSuccessful()) {
            m_thread->startSsl();
            m_state = None;
        } else {
            m_errorMsg = KIO::buildErrorString(KIO::ERR_SLAVE_DEFINED, i18n("The server does not seem to support TLS. Disable TLS if you want to connect without encryption."));
            disconnectFromHost();
        }
        break;
    case Authenticating:
        m_thread->continueAuthentication(response, data);
        break;
    default:
        if (m_currentJob) {
            if (m_currentJob->d->handleResponse(response, data)) {
                m_currentJob = Q_NULLPTR;
                QMetaObject::invokeMethod(this, "executeNextJob", Qt::QueuedConnection);
            }
            break;
        } else {
            // we can get here in the kill current job case
            if (response.operationResult() != Response::Other) {
                QMetaObject::invokeMethod(this, "executeNextJob", Qt::QueuedConnection);
                return;
            }
        }
        qCDebug(KMANAGERSIEVE_LOG) << "Unhandled response!";
    }
}

void Session::scheduleJob(SieveJob *job)
{
    qCDebug(KMANAGERSIEVE_LOG) << Q_FUNC_INFO << job;
    m_jobs.enqueue(job);
    QMetaObject::invokeMethod(this, "executeNextJob", Qt::QueuedConnection);
}

void Session::killJob(SieveJob *job)
{
    qCDebug(KMANAGERSIEVE_LOG) << Q_FUNC_INFO << job;
    if (m_currentJob == job) {
        m_currentJob->d->killed();
        m_currentJob = Q_NULLPTR;
    } else {
        m_jobs.removeAll(job);
        job->d->killed();
    }
}

void Session::executeNextJob()
{
    if (!m_connected || m_state != None || m_currentJob || m_jobs.isEmpty()) {
        return;
    }
    m_currentJob = m_jobs.dequeue();
    m_currentJob->d->run(this);
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
    QRegularExpression regExp(QStringLiteral("Cyrus\\stimsieved\\sv(\\d+)\\.(\\d+)\\.(\\d+)([-\\w]*)"), QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch matchExpression = regExp.match(m_implementation);
    if (matchExpression.hasMatch()) {
        const int major = matchExpression.captured(1).toInt();
        const int minor = matchExpression.captured(2).toInt();
        const int patch = matchExpression.captured(3).toInt();
        const QString vendor = matchExpression.captured(4);
        if (major < 2 || (major == 2 && (minor < 3 || (minor == 3 && patch < 11))) || (vendor == QLatin1String("-kolab-nocaps"))) {
            qCDebug(KMANAGERSIEVE_LOG) << "Enabling compat mode for Cyrus < 2.3.11 or Cyrus marked as \"kolab-nocaps\"";
            return true;
        }
    }
    return false;
}

void Session::sendData(const QByteArray &data)
{
    m_thread->sendData(data);
}

QStringList Session::requestedSaslMethod() const
{
    const QString m = QUrlQuery(m_url).queryItemValue(QStringLiteral("x-mech"));
    if (!m.isEmpty()) {
        return QStringList(m);
    }
    return m_saslMethods;
}

KManageSieve::AuthDetails Session::requestAuthDetails(const QUrl &url)
{
    KIO::AuthInfo ai;
    ai.url = url;
    ai.username = url.userName();
    ai.password = url.password();
    ai.keepPassword = true;
    ai.caption = i18n("Sieve Authentication Details");
    ai.comment = i18n("Please enter your authentication details for your sieve account "
                      "(usually the same as your email password):");

    QPointer<KPasswordDialog> dlg =
        new KPasswordDialog(
        Q_NULLPTR,
        KPasswordDialog::ShowUsernameLine | KPasswordDialog::ShowKeepPassword
    );
    dlg->setUsername(ai.username);
    dlg->setPassword(ai.password);
    dlg->setKeepPassword(ai.keepPassword);
    dlg->setPrompt(ai.prompt);
    dlg->setUsernameReadOnly(ai.readOnly);
    dlg->setWindowTitle(ai.caption);
    dlg->addCommentLine(ai.commentLabel, ai.comment);

    AuthDetails ad;
    ad.valid = false;
    if (dlg->exec()) {
        ad.username = dlg->password();
        ad.password = dlg->password();
        ad.valid = true;
    }
    delete dlg;
    return ad;
}

void Session::authenticationDone()
{
    m_state = None;
    QMetaObject::invokeMethod(this, "executeNextJob", Qt::QueuedConnection);
}

void Session::sslError(const KSslErrorUiData &data)
{
    const bool ignore = KIO::SslUi::askIgnoreSslErrors(data);
    m_thread->handleSslErrorResponse(ignore);
}

void Session::sslDone()
{
    qCDebug(KMANAGERSIEVE_LOG) << "TLS negotiation done.";
    if (requestCapabilitiesAfterStartTls()) {
        sendData("CAPABILITY");
    }
    m_state = PostTlsCapabilities;
}

QString Session::errorMessage() const
{
    return m_errorMsg;
}

void Session::setErrorMessage(const QString &msg)
{
    m_errorMsg = msg;
}

bool Session::allowUnencrypted() const
{
    return QUrlQuery(m_url).queryItemValue(QStringLiteral("x-allow-unencrypted")) == QLatin1String("true");
}
