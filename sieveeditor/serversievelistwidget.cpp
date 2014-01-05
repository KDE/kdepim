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

#include "serversievelistwidget.h"
#include "sieveeditorutil.h"
#include "serversievesettingsdialog.h"

#include <KLocalizedString>

#include <QTreeWidgetItem>
#include <QPointer>
#include <QMenu>

ServerSieveListWidget::ServerSieveListWidget(QWidget *parent)
    : QTreeWidget(parent)
{
    connect(this, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(modifyServerConfig()));
    connect( this, SIGNAL(customContextMenuRequested(QPoint)),
             this, SLOT(slotContextMenuRequested(QPoint)) );
}

ServerSieveListWidget::~ServerSieveListWidget()
{

}

void ServerSieveListWidget::slotContextMenuRequested(const QPoint &pos)
{
    QTreeWidgetItem *item = itemAt( pos );
    if ( !item )
        return;
    //if ( !item->parent() && !mUrls.count( item ))
        //return;
    QMenu menu;
    if ( /*isFileNameItem( item )*/false ) {
        // script items:
        menu.addAction( i18n( "Edit Script..." ), this, SLOT(slotEditScript()) );
        menu.addAction( i18n( "Delete Script" ), this, SLOT(slotDeleteScript()) );
#if 0
        if ( itemIsActived( item ) ) {
            menu.addSeparator();
            menu.addAction( i18n( "Deactivate Script" ), this, SLOT(slotDeactivateScript()) );
        }
#endif
    } else if ( !item->parent() ) {
#if 0
        // top-levels:
        if ( !serverHasError(item) && mJobs.keys(item).isEmpty())
            menu.addAction( i18n( "New Script..." ), this, SLOT(slotNewScript()) );
#endif
    }
    if ( !menu.actions().isEmpty() )
        menu.exec( viewport()->mapToGlobal(pos) );
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
    for (int i=0; i <topLevelItemCount(); ++i) {
        ServerSieveListWidgetItem *serverSieveItem = static_cast<ServerSieveListWidgetItem*>(topLevelItem(i));
        if (serverSieveItem) {
            lstServerConfig.append(serverSieveItem->serverConfig());
        }
    }
    SieveEditorUtil::writeServerSieveConfig(lstServerConfig);
}


void ServerSieveListWidget::modifyServerConfig()
{
    QTreeWidgetItem *item = currentItem();
    if (!item)
        return;

    ServerSieveListWidgetItem *serverSieveListItem = static_cast<ServerSieveListWidgetItem *>(item);

    QPointer<ServerSieveSettingsDialog> dlg = new ServerSieveSettingsDialog(this);
    dlg->setCaption(i18n("Modify Settings"));
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

ServerSieveListWidgetItem::ServerSieveListWidgetItem(QTreeWidget *parent)
    : QTreeWidgetItem(parent)
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
    setText(0, conf.serverName);
    mServerConfig = conf;
}
