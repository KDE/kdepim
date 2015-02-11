/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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
#include <QDateTime>
#include "pimcommon_export.h"

namespace PimCommon
{
class StorageServiceTreeWidget;
class PIMCOMMON_EXPORT StorageServiceTreeWidgetItem : public QTreeWidgetItem
{
public:
    StorageServiceTreeWidgetItem(StorageServiceTreeWidget *parent);
    bool operator<(const QTreeWidgetItem &other) const Q_DECL_OVERRIDE;
    void setSize(qulonglong size);
    void setDateCreated(const QDateTime &date);
    void setLastModification(const QDateTime &date);
    void setStoreInfo(const QVariantMap &data);
    QVariantMap storeInfo() const;

    QDateTime lastModificationDate() const;
    QDateTime createDate() const;
    qulonglong size() const;
private:
    QDateTime mCreateDate;
    QDateTime mLastModificationDate;
    qulonglong mSize;
};
}

#endif // STORAGESERVICETREEWIDGETITEM_H
