/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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

#include "serversievesettingsdialog.h"
#include "serversievesettings.h"

#include <KLocalizedString>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QSpinBox>

ServerSieveSettingsDialog::ServerSieveSettingsDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Add Server Sieve" ) );
    setButtons( Cancel | Ok  );

    QWidget *w = new QWidget;
    QVBoxLayout *lay = new QVBoxLayout;
    mServerSieveSettings = new ServerSieveSettings;
    connect(mServerSieveSettings, SIGNAL(enableOkButton(bool)), this, SLOT(enableButtonOk(bool)));
    lay->addWidget(mServerSieveSettings);
    lay->setMargin(0);
    w->setLayout(lay);
    setMainWidget(w);
    resize(300,200);
    enableButtonOk(false);
}

ServerSieveSettingsDialog::~ServerSieveSettingsDialog()
{

}

QString ServerSieveSettingsDialog::serverName() const
{
    return mServerSieveSettings->serverName();
}

void ServerSieveSettingsDialog::setServerName(const QString &name)
{
    mServerSieveSettings->setServerName(name);
}

int ServerSieveSettingsDialog::port() const
{
    return mServerSieveSettings->port();
}

void ServerSieveSettingsDialog::setPort(int value)
{
    mServerSieveSettings->setPort(value);
}

QString ServerSieveSettingsDialog::userName() const
{
    return mServerSieveSettings->userName();
}

void ServerSieveSettingsDialog::setUserName(const QString &name)
{
    mServerSieveSettings->setUserName(name);
}

QString ServerSieveSettingsDialog::password() const
{
    return mServerSieveSettings->password();
}

void ServerSieveSettingsDialog::setPassword(const QString &pass)
{
    mServerSieveSettings->setPassword(pass);
}

void ServerSieveSettingsDialog::setServerSieveConfig(const SieveEditorUtil::SieveServerConfig &conf)
{
    mServerSieveSettings->setServerSieveConfig(conf);
}

SieveEditorUtil::SieveServerConfig ServerSieveSettingsDialog::serverSieveConfig() const
{
    return mServerSieveSettings->serverSieveConfig();
}
