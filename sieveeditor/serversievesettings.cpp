/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "serversievesettings.h"
#include "ui_serversievesettings.h"

ServerSieveSettings::ServerSieveSettings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ServerSieveSettings)
{
    ui->setupUi(this);
}

ServerSieveSettings::~ServerSieveSettings()
{
    delete ui;
}

QString ServerSieveSettings::serverName() const
{
    return ui->serverName->text();
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
    return ui->userName->text();
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
