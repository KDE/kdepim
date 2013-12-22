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

#include "addsieveserverdialog.h"
#include "serversievesettings.h"

#include <KLocalizedString>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QSpinBox>

AddSieveServerDialog::AddSieveServerDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Add Server Sieve" ) );
    setButtons( Cancel | Ok  );

    QWidget *w = new QWidget;
    QVBoxLayout *lay = new QVBoxLayout;
    mServerSieveSettings = new ServerSieveSettings;
    lay->addWidget(mServerSieveSettings);
    lay->setMargin(0);
    w->setLayout(lay);
    setMainWidget(w);
    resize(300,200);
}

AddSieveServerDialog::~AddSieveServerDialog()
{

}

QString AddSieveServerDialog::serverName() const
{
    return mServerSieveSettings->serverName();
}

void AddSieveServerDialog::setServerName(const QString &name)
{
    mServerSieveSettings->setServerName(name);
}

int AddSieveServerDialog::port() const
{
    return mServerSieveSettings->port();
}

void AddSieveServerDialog::setPort(int value)
{
    mServerSieveSettings->setPort(value);
}

QString AddSieveServerDialog::userName() const
{
    return mServerSieveSettings->userName();
}

void AddSieveServerDialog::setUserName(const QString &name)
{
    mServerSieveSettings->setUserName(name);
}

QString AddSieveServerDialog::password() const
{
    return mServerSieveSettings->password();
}

void AddSieveServerDialog::setPassword(const QString &pass)
{
    mServerSieveSettings->setPassword(pass);
}
