/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#ifndef STORAGESERVICETREEWIDGETITEM_H
#define STORAGESERVICETREEWIDGETITEM_H

#include <QTreeWidget>
#include <KDateTime>
#include "pimcommon_export.h"

namespace PimCommon {
class StorageServiceTreeWidget;
class PIMCOMMON_EXPORT StorageServiceTreeWidgetItem : public QTreeWidgetItem
{
public:
    StorageServiceTreeWidgetItem(StorageServiceTreeWidget *parent);
    bool operator<(const QTreeWidgetItem &other) const;
    void setSize(qulonglong size);
    void setDateCreated(const KDateTime &date);
    void setLastModification(const KDateTime &date);
    void setStoreInfo(const QVariantMap &data);
    QVariantMap storeInfo() const;


    KDateTime lastModificationDate() const;
    KDateTime createDate() const;
    qulonglong size() const;
private:
    KDateTime mCreateDate;
    KDateTime mLastModificationDate;
    qulonglong mSize;
};
}

#endif // STORAGESERVICETREEWIDGETITEM_H
