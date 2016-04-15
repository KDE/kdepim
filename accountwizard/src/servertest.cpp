/*
    Copyright (c) 2010 Tom Albers <toma@kde.org>

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

#include "servertest.h"
#include <mailtransport/transport.h>
#include <mailtransport/servertest.h>

#include "accountwizard_debug.h"
#include <kmessagebox.h>
#include <KLocalizedString>

ServerTest::ServerTest(QObject *parent) :
    QObject(parent), m_serverTest(new MailTransport::ServerTest(0))
{
    qCDebug(ACCOUNTWIZARD_LOG) << "Welcome!";
    connect(m_serverTest, &MailTransport::ServerTest::finished, this, &ServerTest::testFinished);
}

ServerTest::~ServerTest()
{
    delete m_serverTest;
}

void ServerTest::test(const QString &server, const QString &protocol)
{
    qCDebug(ACCOUNTWIZARD_LOG) << server << protocol;
    m_serverTest->setServer(server);
    m_serverTest->setProtocol(protocol);
    if (protocol == QLatin1String("submission")) {
        m_serverTest->setProtocol(QStringLiteral("smtp"));
        m_serverTest->setPort(MailTransport::Transport::EnumEncryption::None, 587);
        m_serverTest->setPort(MailTransport::Transport::EnumEncryption::SSL, 0);        //No ssl port for submission
    }
    m_serverTest->start();
}

void ServerTest::testFinished(const QList< int > &list)
{
    qCDebug(ACCOUNTWIZARD_LOG) << "types: " << list;
    if (list.contains(MailTransport::Transport::EnumEncryption::TLS)) {
        Q_EMIT testResult(QStringLiteral("tls"));
    } else if (list.contains(MailTransport::Transport::EnumEncryption::SSL)) {
        Q_EMIT testResult(QStringLiteral("ssl"));
    } else {
        KMessageBox::information(0, i18n("There seems to be a problem in reaching this server "
                                         "or choosing a safe way to sent the credentials to server. We advise you to "
                                         "check the settings of the account and adjust it manually if needed."),
                                 i18n("Autodetecting settings failed"));
        Q_EMIT testFail();
    }
}

