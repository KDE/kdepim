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

#include "serversievelistwidget.h"
#include "sieveeditorutil.h"
#include "serversievesettingsdialog.h"

#include <KLocalizedString>

#include <QListWidgetItem>
#include <QPointer>

ServerSieveListWidget::ServerSieveListWidget(QWidget *parent)
    : QListWidget(parent)
{
    connect(this, &ServerSieveListWidget::itemDoubleClicked, this, &ServerSieveListWidget::modifyServerConfig);
}

ServerSieveListWidget::~ServerSieveListWidget()
{

}

void ServerSieveListWidget::readConfig()
{
    const QVector<SieveEditorUtil::SieveServerConfig> lstServer = SieveEditorUtil::readServerSieveConfig();
    Q_FOREACH (const SieveEditorUtil::SieveServerConfig &conf, lstServer) {
        ServerSieveListWidgetItem *item = new ServerSieveListWidgetItem(this);
        item->setServerConfig(conf);
    }
}

void ServerSieveListWidget::writeConfig()
{
    QVector<SieveEditorUtil::SieveServerConfig> lstServerConfig;
    for (int i = 0; i < count(); ++i) {
        ServerSieveListWidgetItem *serverSieveItem = static_cast<ServerSieveListWidgetItem *>(item(i));
        if (serverSieveItem) {
            SieveEditorUtil::SieveServerConfig config = serverSieveItem->serverConfig();
            config.enabled = (serverSieveItem->checkState() == Qt::Checked);
            lstServerConfig.append(config);
        }
    }
    SieveEditorUtil::writeServerSieveConfig(lstServerConfig);
}

void ServerSieveListWidget::modifyServerConfig()
{
    QListWidgetItem *item = currentItem();
    if (!item) {
        return;
    }

    ServerSieveListWidgetItem *serverSieveListItem = static_cast<ServerSieveListWidgetItem *>(item);

    QPointer<ServerSieveSettingsDialog> dlg = new ServerSieveSettingsDialog(this);
    dlg->setWindowTitle(i18n("Modify Settings"));
    dlg->setServerSieveConfig(serverSieveListItem->serverConfig());
    if (dlg->exec()) {
        serverSieveListItem->setServerConfig(dlg->serverSieveConfig());
    }
    delete dlg;
}

void ServerSieveListWidget::addServerConfig()
{
    QPointer<ServerSieveSettingsDialog> dlg = new ServerSieveSettingsDialog(this);
    if (dlg->exec()) {
        ServerSieveListWidgetItem *item = new ServerSieveListWidgetItem(this);
        item->setServerConfig(dlg->serverSieveConfig());
    }
    delete dlg;
}

ServerSieveListWidgetItem::ServerSieveListWidgetItem(QListWidget *parent)
    : QListWidgetItem(parent)
{

}

ServerSieveListWidgetItem::~ServerSieveListWidgetItem()
{

}

SieveEditorUtil::SieveServerConfig ServerSieveListWidgetItem::serverConfig() const
{
    return mServerConfig;
}

void ServerSieveListWidgetItem::setServerConfig(const SieveEditorUtil::SieveServerConfig &conf)
{
    setCheckState(conf.enabled ? Qt::Checked : Qt::Unchecked);
    setText(conf.serverName);
    mServerConfig = conf;
}
