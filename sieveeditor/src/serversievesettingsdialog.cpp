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

#include "serversievesettingsdialog.h"
#include "serversievesettings.h"

#include <KLocalizedString>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>

ServerSieveSettingsDialog::ServerSieveSettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18nc("@title:window", "Add Sieve Server"));

    QWidget *w = new QWidget;
    QVBoxLayout *lay = new QVBoxLayout;
    mServerSieveSettings = new ServerSieveSettings;
    connect(mServerSieveSettings, &ServerSieveSettings::enableOkButton, this, &ServerSieveSettingsDialog::slotEnableButtonOk);
    lay->addWidget(mServerSieveSettings);
    lay->setMargin(0);
    w->setLayout(lay);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(w);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    mOkButton = buttonBox->button(QDialogButtonBox::Ok);
    mOkButton->setDefault(true);
    mOkButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ServerSieveSettingsDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ServerSieveSettingsDialog::reject);
    mainLayout->addWidget(buttonBox);

    readConfig();
    mOkButton->setEnabled(false);
}

ServerSieveSettingsDialog::~ServerSieveSettingsDialog()
{
    writeConfig();
}

void ServerSieveSettingsDialog::readConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "ServerSieveSettingsDialog");
    const QSize size = group.readEntry("Size", QSize(450, 350));
    if (size.isValid()) {
        resize(size);
    }
}

void ServerSieveSettingsDialog::writeConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "ServerSieveSettingsDialog");
    group.writeEntry("Size", size());
    group.sync();
}

void ServerSieveSettingsDialog::slotEnableButtonOk(bool b)
{
    mOkButton->setEnabled(b);
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
