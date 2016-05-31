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

#ifndef SERVERSIEVESETTINGS_H
#define SERVERSIEVESETTINGS_H

#include <QWidget>
#include "sieveeditorutil.h"

namespace Ui
{
class ServerSieveSettings;
}

class ServerSieveSettings : public QWidget
{
    Q_OBJECT

public:
    explicit ServerSieveSettings(QWidget *parent = Q_NULLPTR);
    ~ServerSieveSettings();

    QString serverName() const;
    void setServerName(const QString &name);

    int port() const;
    void setPort(int value);

    QString userName() const;
    void setUserName(const QString &name);

    QString password() const;
    void setPassword(const QString &pass);

    void setServerSieveConfig(const SieveEditorUtil::SieveServerConfig &conf);
    SieveEditorUtil::SieveServerConfig serverSieveConfig() const;

Q_SIGNALS:
    void enableOkButton(bool enabled);

private Q_SLOTS:
    void slotUserServerNameChanged();

private:
    void populateDefaultAuthenticationOptions();
    Ui::ServerSieveSettings *ui;
};

#endif // SERVERSIEVESETTINGS_H
