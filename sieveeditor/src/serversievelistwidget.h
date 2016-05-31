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

#ifndef SERVERSIEVELISTWIDGET_H
#define SERVERSIEVELISTWIDGET_H

#include <QListWidget>
#include "sieveeditorutil.h"

class ServerSieveListWidgetItem : public QListWidgetItem
{
public:
    explicit ServerSieveListWidgetItem(QListWidget *parent = Q_NULLPTR);
    ~ServerSieveListWidgetItem();

    SieveEditorUtil::SieveServerConfig serverConfig() const;
    void setServerConfig(const SieveEditorUtil::SieveServerConfig &conf);

private:
    SieveEditorUtil::SieveServerConfig mServerConfig;
};

class ServerSieveListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit ServerSieveListWidget(QWidget *parent = Q_NULLPTR);
    ~ServerSieveListWidget();

    void readConfig();
    void writeConfig();
    void addServerConfig();

public Q_SLOTS:
    void modifyServerConfig();
};

#endif // SERVERSIEVELISTWIDGET_H
