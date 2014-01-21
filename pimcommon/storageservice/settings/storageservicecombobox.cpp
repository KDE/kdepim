/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "storageservicecombobox.h"
#include "storageservice/storageservicemanager.h"

using namespace PimCommon;

StorageServiceComboBox::StorageServiceComboBox(PimCommon::StorageServiceAbstract::Capability cap, const QStringList &excludeService, QWidget *parent)
    : QComboBox(parent)
{
    initialize(cap, excludeService);
}

StorageServiceComboBox::~StorageServiceComboBox()
{

}

void StorageServiceComboBox::initialize(PimCommon::StorageServiceAbstract::Capability cap, const QStringList &excludeService)
{
    for (int i=0; i < PimCommon::StorageServiceManager::EndListService; ++i) {
        const PimCommon::StorageServiceManager::ServiceType type = static_cast<PimCommon::StorageServiceManager::ServiceType>(i);
        if (!excludeService.contains(PimCommon::StorageServiceManager::serviceName(type))) {
            const QString iconName = PimCommon::StorageServiceManager::icon(type);
            const PimCommon::StorageServiceAbstract::Capabilities capabilities = PimCommon::StorageServiceManager::capabilities(type);
            if (capabilities & cap) {
                if (iconName.isEmpty()) {
                    addItem(PimCommon::StorageServiceManager::serviceToI18n(type), type);
                } else {
                    const KIcon icon = KIcon(iconName);
                    addItem(icon, PimCommon::StorageServiceManager::serviceToI18n(type), type);
                }
            }
        }
    }
}

PimCommon::StorageServiceManager::ServiceType StorageServiceComboBox::service() const
{
    if (currentIndex()!=-1) {
        const PimCommon::StorageServiceManager::ServiceType serviceType = static_cast<PimCommon::StorageServiceManager::ServiceType>(itemData(currentIndex()).toInt());
        return serviceType;
    }
    return PimCommon::StorageServiceManager::Unknown;
}
