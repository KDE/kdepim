/*
   Copyright (C) 2013-2016 Montel Laurent <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "serversievesettings.h"
#include "ui_serversievesettings.h"
#include <MailTransport/mailtransport/transport.h>

#include <KLocalizedString>
#include "sieveeditor_debug.h"

/** static helper functions **/
static QString authenticationModeString(MailTransport::Transport::EnumAuthenticationType::type mode)
{
    switch (mode) {
    case  MailTransport::Transport::EnumAuthenticationType::LOGIN:
        return QStringLiteral("LOGIN");
    case MailTransport::Transport::EnumAuthenticationType::PLAIN:
        return QStringLiteral("PLAIN");
    case MailTransport::Transport::EnumAuthenticationType::CRAM_MD5:
        return QStringLiteral("CRAM-MD5");
    case MailTransport::Transport::EnumAuthenticationType::DIGEST_MD5:
        return QStringLiteral("DIGEST-MD5");
    case MailTransport::Transport::EnumAuthenticationType::GSSAPI:
        return QStringLiteral("GSSAPI");
    case MailTransport::Transport::EnumAuthenticationType::NTLM:
        return QStringLiteral("NTLM");
    case MailTransport::Transport::EnumAuthenticationType::CLEAR:
        return i18nc("Authentication method", "Clear text");
    case MailTransport::Transport::EnumAuthenticationType::ANONYMOUS:
        return i18nc("Authentication method", "Anonymous");
    default:
        break;
    }
    return QString();
}

static void addAuthenticationItem(QComboBox *authCombo, MailTransport::Transport::EnumAuthenticationType::type authtype)
{
    //qCDebug(SIEVEEDITOR_LOG) << "adding auth item " << authenticationModeString( authtype );
    authCombo->addItem(authenticationModeString(authtype), QVariant(authtype));
}

static MailTransport::Transport::EnumAuthenticationType::type getCurrentAuthMode(QComboBox *authCombo)
{
    MailTransport::Transport::EnumAuthenticationType::type authtype = (MailTransport::Transport::EnumAuthenticationType::type) authCombo->itemData(authCombo->currentIndex()).toInt();
    //qCDebug(SIEVEEDITOR_LOG) << "current auth mode: " << authenticationModeString( authtype );
    return authtype;
}

static void setCurrentAuthMode(QComboBox *authCombo, MailTransport::Transport::EnumAuthenticationType::type authtype)
{
    //qCDebug(SIEVEEDITOR_LOG) << "setting authcombo: " << authenticationModeString( authtype );
    int index = authCombo->findData(authtype);
    if (index == -1) {
        qCWarning(SIEVEEDITOR_LOG) << "desired authmode not in the combo";
    }
    //qCDebug(SIEVEEDITOR_LOG) << "found corresponding index: " << index << "with data" << authenticationModeString( (MailTransport::Transport::EnumAuthenticationType::type) authCombo->itemData( index ).toInt() );
    authCombo->setCurrentIndex(index);
    MailTransport::Transport::EnumAuthenticationType::type t = (MailTransport::Transport::EnumAuthenticationType::type) authCombo->itemData(authCombo->currentIndex()).toInt();
    //qCDebug(SIEVEEDITOR_LOG) << "selected auth mode:" << authenticationModeString( t );
    Q_ASSERT(t == authtype);
}

ServerSieveSettings::ServerSieveSettings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ServerSieveSettings)
{
    ui->setupUi(this);
    populateDefaultAuthenticationOptions();
    connect(ui->serverName, &QLineEdit::textChanged, this, &ServerSieveSettings::slotUserServerNameChanged);
    connect(ui->userName, &QLineEdit::textChanged, this, &ServerSieveSettings::slotUserServerNameChanged);
}

ServerSieveSettings::~ServerSieveSettings()
{
    delete ui;
}

void ServerSieveSettings::populateDefaultAuthenticationOptions()
{
    ui->authenticationCombo->clear();
    addAuthenticationItem(ui->authenticationCombo, MailTransport::Transport::EnumAuthenticationType::CLEAR);
    addAuthenticationItem(ui->authenticationCombo, MailTransport::Transport::EnumAuthenticationType::LOGIN);
    addAuthenticationItem(ui->authenticationCombo, MailTransport::Transport::EnumAuthenticationType::PLAIN);
    addAuthenticationItem(ui->authenticationCombo, MailTransport::Transport::EnumAuthenticationType::CRAM_MD5);
    addAuthenticationItem(ui->authenticationCombo, MailTransport::Transport::EnumAuthenticationType::DIGEST_MD5);
    addAuthenticationItem(ui->authenticationCombo, MailTransport::Transport::EnumAuthenticationType::NTLM);
    addAuthenticationItem(ui->authenticationCombo, MailTransport::Transport::EnumAuthenticationType::GSSAPI);
    addAuthenticationItem(ui->authenticationCombo, MailTransport::Transport::EnumAuthenticationType::ANONYMOUS);
}

void ServerSieveSettings::slotUserServerNameChanged()
{
    Q_EMIT enableOkButton(!ui->userName->text().trimmed().isEmpty() && !ui->serverName->text().trimmed().isEmpty());
}

QString ServerSieveSettings::serverName() const
{
    return ui->serverName->text().trimmed();
}

void ServerSieveSettings::setServerName(const QString &name)
{
    ui->serverName->setText(name);
}

int ServerSieveSettings::port() const
{
    return ui->port->value();
}

void ServerSieveSettings::setPort(int value)
{
    ui->port->setValue(value);
}

QString ServerSieveSettings::userName() const
{
    return ui->userName->text().trimmed();
}

void ServerSieveSettings::setUserName(const QString &name)
{
    ui->userName->setText(name);
}

QString ServerSieveSettings::password() const
{
    return ui->password->text();
}

void ServerSieveSettings::setPassword(const QString &pass)
{
    ui->password->setText(pass);
}

void ServerSieveSettings::setServerSieveConfig(const SieveEditorUtil::SieveServerConfig &conf)
{
    setPassword(conf.password);
    setPort(conf.port);
    setServerName(conf.serverName);
    setUserName(conf.userName);
    setCurrentAuthMode(ui->authenticationCombo, conf.authenticationType);
}

SieveEditorUtil::SieveServerConfig ServerSieveSettings::serverSieveConfig() const
{
    SieveEditorUtil::SieveServerConfig conf;
    conf.password = password();
    conf.port = port();
    conf.serverName = serverName();
    conf.userName = userName();
    const MailTransport::Transport::EnumAuthenticationType::type authtype = getCurrentAuthMode(ui->authenticationCombo);
    conf.authenticationType = authtype;
    return conf;
}
