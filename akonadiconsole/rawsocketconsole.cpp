/*
    Copyright (c) 2009 Volker Krause <vkrause@kde.org>

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

#include "rawsocketconsole.h"

#include <akonadi/private/xdgbasedirs_p.h>
#include <AkonadiCore/servermanager.h>

#include "akonadiconsole_debug.h"

#include <QIcon>
#include <KLocalizedString>

#include <QFile>
#include <QLocalSocket>
#include <QSettings>
#include <QFontDatabase>

using namespace Akonadi;

RawSocketConsole::RawSocketConsole(QWidget *parent) :
    QWidget(parent),
    mSocket(new QLocalSocket(this))
{
    ui.setupUi(this);
    ui.execButton->setIcon(QIcon::fromTheme(QStringLiteral("application-x-executable")));
    connect(ui.execButton, &QPushButton::clicked, this, &RawSocketConsole::execClicked);
    connect(ui.commandEdit, &QLineEdit::returnPressed, this, &RawSocketConsole::execClicked);
    connect(ui.connectButton, &QPushButton::clicked, this, &RawSocketConsole::connectClicked);
    connect(ui.clearButton, &QPushButton::clicked, ui.protocolView, &KTextEdit::clear);
    ui.protocolView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    connect(mSocket, &QLocalSocket::readyRead, this, &RawSocketConsole::dataReceived);
    connect(mSocket, &QLocalSocket::connected, this, &RawSocketConsole::connected);
    connect(mSocket, &QLocalSocket::disconnected, this, &RawSocketConsole::disconnected);

    disconnected();
    connectClicked();
}

void RawSocketConsole::execClicked()
{
    const QString command = ui.commandEdit->text().trimmed() + QLatin1Char('\n');
    if (command.isEmpty()) {
        return;
    }
    mSocket->write(command.toUtf8());
    ui.commandEdit->clear();
    ui.protocolView->append(QLatin1String("<font color=\"red\">") + command + QLatin1String("</font>"));
}

void RawSocketConsole::dataReceived()
{
    while (mSocket->canReadLine()) {
        const QString line = QString::fromUtf8(mSocket->readLine());
        ui.protocolView->append(QLatin1String("<font color=\"blue\">") + line + QLatin1String("</font>"));
    }
}

void RawSocketConsole::connected()
{
    ui.connectButton->setChecked(true);
    ui.connectButton->setText(i18n("Disconnect"));
    ui.execButton->setEnabled(true);
    ui.commandEdit->setEnabled(true);
}

void RawSocketConsole::disconnected()
{
    ui.connectButton->setChecked(false);
    ui.connectButton->setText(i18n("Connect"));
    ui.execButton->setEnabled(false);
    ui.commandEdit->setEnabled(false);
}

void RawSocketConsole::connectClicked()
{
    if (mSocket->state() == QLocalSocket::ConnectedState) {
        mSocket->close();
    } else {
        QString connectionConfigFile;
        if (Akonadi::ServerManager::self()->hasInstanceIdentifier()) {
            const QString akonadiPath = XdgBaseDirs::findResourceDir("config", QStringLiteral("akonadi"));
            connectionConfigFile = akonadiPath + QLatin1String("/instance/")
                                   + Akonadi::ServerManager::self()->instanceIdentifier()
                                   + QLatin1String("/akonadiconnectionrc");
        } else {
            connectionConfigFile = XdgBaseDirs::akonadiConnectionConfigFile();
        }

        if (!QFile::exists(connectionConfigFile)) {
            qCWarning(AKONADICONSOLE_LOG) << "Akonadi Client Session: connection config file '"
                                          << "akonadi/akonadiconnectionrc cannot be found in '"
                                          << XdgBaseDirs::homePath("config") << "' nor in any of "
                                          << XdgBaseDirs::systemPathList("config");
        }
        QSettings conSettings(connectionConfigFile, QSettings::IniFormat);
#ifdef Q_OS_WIN  //krazy:exclude=cpp
        const QString namedPipe = conSettings.value(QLatin1String("Data/NamedPipe"), QStringLiteral("Akonadi")).toString();
        mSocket->connectToServer(namedPipe);
#else
        const QString defaultSocketDir = XdgBaseDirs::saveDir("data", QStringLiteral("akonadi"));
        const QString path = conSettings.value(QLatin1String("Data/UnixPath"), QString(defaultSocketDir + QLatin1String("/akonadiserver.socket"))).toString();
        mSocket->connectToServer(path);
#endif
    }
}

