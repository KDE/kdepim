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

#ifndef STORAGESERVICECOMBOBOX_H
#define STORAGESERVICECOMBOBOX_H

#include <QComboBox>
#include "storageservice/storageservicemanager.h"

namespace PimCommon
{
class StorageServiceComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit StorageServiceComboBox(const QList<PimCommon::StorageServiceAbstract::Capability> &lstCap, const QStringList &excludeService, QWidget *parent = 0);
    ~StorageServiceComboBox();

    PimCommon::StorageServiceManager::ServiceType service() const;
private:
    void initialize(const QList<PimCommon::StorageServiceAbstract::Capability> &lstCap, const QStringList &excludeService);
};
}
#endif // STORAGESERVICECOMBOBOX_H
