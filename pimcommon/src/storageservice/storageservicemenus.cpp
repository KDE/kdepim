/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "storageservicemenus.h"
#include "storageservicepluginmanager.h"
#include "storageserviceplugin.h"

#include <KActionMenu>
#include <KLocalizedString>

using namespace PimCommon;

class PimCommon::StorageServiceMenusPrivate
{
public:
    StorageServiceMenusPrivate()
    {

    }
    KActionMenu *menuWithCapability(PimCommon::StorageServicePlugin::Capability mainCapability, const QList<PimCommon::StorageServicePlugin::Capability> &lstCapability, QWidget *parent) const
    {
        KActionMenu *menuService = new KActionMenu(i18n("Storage service"), parent);
        if (PimCommon::StorageServicePluginManager::self()->pluginsList().isEmpty()) {
            QAction *act = new QAction(i18n("No Storage service configured"), menuService);
            act->setEnabled(false);
            menuService->addAction(act);
        } else {
            //TODO create service !
        }
        return menuService;
    }

};

StorageServiceMenus::StorageServiceMenus(QObject *parent)
    : QObject(parent),
      d(new PimCommon::StorageServiceMenusPrivate)
{

}

StorageServiceMenus::~StorageServiceMenus()
{
    delete d;
}

KActionMenu *StorageServiceMenus::shareLinkServices(QWidget *parent) const
{
    QList<PimCommon::StorageServicePlugin::Capability> lstCapability;
    lstCapability << PimCommon::StorageServicePlugin::ShareLinkCapability;
    lstCapability << PimCommon::StorageServicePlugin::UploadFileCapability;
    return d->menuWithCapability(PimCommon::StorageServicePlugin::ShareLinkCapability, lstCapability, parent);
}

KActionMenu *StorageServiceMenus::downloadServices(QWidget *parent) const
{
    QList<PimCommon::StorageServicePlugin::Capability> lstCapability;
    lstCapability << PimCommon::StorageServicePlugin::DownloadFileCapability;
    return d->menuWithCapability(PimCommon::StorageServicePlugin::DownloadFileCapability, lstCapability, parent);
}

KActionMenu *StorageServiceMenus::uploadServices(QWidget *parent) const
{
    QList<PimCommon::StorageServicePlugin::Capability> lstCapability;
    lstCapability << PimCommon::StorageServicePlugin::UploadFileCapability;
    return d->menuWithCapability(PimCommon::StorageServicePlugin::UploadFileCapability, lstCapability, parent);
}

