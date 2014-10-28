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

#include "storageserviceutils.h"
#include <KLocalizedString>
#include <cstdlib>

QString PimCommon::StorageServiceUtils::generateNonce(qint32 length)
{
    QString clng;

    for (int i = 0; i < length; ++i) {
        clng += QString::number(int(qrand() / (RAND_MAX + 1.0) * (16 + 1 - 0) + 0), 16).toUpper();
    }

    return clng;
}

bool PimCommon::StorageServiceUtils::hasCapabilities(PimCommon::StorageServiceAbstract::Capabilities capabilities, const QList<PimCommon::StorageServiceAbstract::Capability> &lstNeedCapabily)
{
    Q_FOREACH(PimCommon::StorageServiceAbstract::Capability cap, lstNeedCapabily) {
        if (capabilities & cap) {
            return true;
        }
    }
    return false;
}

bool PimCommon::StorageServiceUtils::hasExactCapabilities(PimCommon::StorageServiceAbstract::Capabilities capabilities, const QList<PimCommon::StorageServiceAbstract::Capability> &lstNeedCapabily)
{
    Q_FOREACH(PimCommon::StorageServiceAbstract::Capability cap, lstNeedCapabily) {
        if (!(capabilities & cap)) {
            return false;
        }
    }
    return true;
}

QString PimCommon::StorageServiceUtils::propertyNameToI18n(PropertyName type)
{
    QString result;
    switch (type) {
    case Type:
        result = i18n("Type:");
        break;
    case Name:
        result = i18n("Name:");
        break;
    case LastModified:
        result = i18n("Last Modified:");
        break;
    case Created:
        result = i18n("Created:");
        break;
    case Size:
        result = i18n("Size:");
        break;
    }
    return result;
}
