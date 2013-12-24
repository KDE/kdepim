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

#include "serversievelistwidget.h"
#include "sieveeditorutil.h"
#include "serversievesettingsdialog.h"

#include <QListWidgetItem>
#include <QPointer>

ServerSieveListWidget::ServerSieveListWidget(QWidget *parent)
    : QListWidget(parent)
{
    connect(this, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(modifyServerConfig()));
}

ServerSieveListWidget::~ServerSieveListWidget()
{

}

void ServerSieveListWidget::readConfig()
{
    const QList<SieveEditorUtil::SieveServerConfig> lstServer = SieveEditorUtil::readServerSieveConfig();
    Q_FOREACH (const SieveEditorUtil::SieveServerConfig &conf, lstServer) {
        ServerSieveListWidgetItem *item = new ServerSieveListWidgetItem(this);
        item->setServerConfig(conf);
    }
}

void ServerSieveListWidget::writeConfig()
{
    QList<SieveEditorUtil::SieveServerConfig> lstServerConfig;
    for (int i=0; i <count(); ++i) {
        ServerSieveListWidgetItem *serverSieveItem = static_cast<ServerSieveListWidgetItem*>(item(i));
        if (serverSieveItem) {
            lstServerConfig.append(serverSieveItem->serverConfig());
        }
    }
    SieveEditorUtil::writeServerSieveConfig(lstServerConfig);
}


void ServerSieveListWidget::modifyServerConfig()
{
    QListWidgetItem *item = currentItem();
    if (!item)
        return;

    ServerSieveListWidgetItem *serverSieveListItem = static_cast<ServerSieveListWidgetItem *>(item);

    QPointer<ServerSieveSettingsDialog> dlg = new ServerSieveSettingsDialog(this);
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
    setText(conf.serverName);
    mServerConfig = conf;
}
