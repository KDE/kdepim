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

#include "storageservicetreewidgetitem.h"
#include "storageservicetreewidget.h"
#include <KGlobal>
#include <KLocale>
using namespace PimCommon;

StorageServiceTreeWidgetItem::StorageServiceTreeWidgetItem(StorageServiceTreeWidget *parent)
    : QTreeWidgetItem(parent),
      mSize(-1)
{

}

bool StorageServiceTreeWidgetItem::operator<(const QTreeWidgetItem &other) const
{
    const QTreeWidgetItem *otherItem = &other;
    const StorageServiceTreeWidgetItem *storageItem = static_cast<const StorageServiceTreeWidgetItem*>( otherItem );

    StorageServiceTreeWidget::ItemType sourceType = static_cast<StorageServiceTreeWidget::ItemType>(data(StorageServiceTreeWidget::ColumnName, StorageServiceTreeWidget::ElementType).toInt());
    StorageServiceTreeWidget::ItemType otherType = static_cast<StorageServiceTreeWidget::ItemType>(other.data(StorageServiceTreeWidget::ColumnName, StorageServiceTreeWidget::ElementType).toInt());
    if (sourceType == StorageServiceTreeWidget::MoveUpType) {
        return false;
    } else if (otherType == StorageServiceTreeWidget::MoveUpType) {
        return false;
    }

    switch(treeWidget()->sortColumn()) {
    case StorageServiceTreeWidget::ColumnName: {
        if (sourceType == otherType) {
            return text(StorageServiceTreeWidget::ColumnName) < other.text(StorageServiceTreeWidget::ColumnName);
        } else {
            if (sourceType == StorageServiceTreeWidget::Folder) {
                return false;
            } else {
                return true;
            }
        }
        break;
    }
    case StorageServiceTreeWidget::ColumnSize: {
        if (sourceType == otherType) {
            return size() < storageItem->size();
        } else {
            if (sourceType == StorageServiceTreeWidget::Folder) {
                return false;
            } else {
                return true;
            }
        }
        break;
    }
    case StorageServiceTreeWidget::ColumnCreated: {
        if (sourceType == otherType) {
            return createDate() < storageItem->createDate();
        } else {
            if (sourceType == StorageServiceTreeWidget::Folder) {
                return false;
            } else {
                return true;
            }
        }
        break;
    }
    case StorageServiceTreeWidget::ColumnLastModification: {
        if (sourceType == otherType) {
            return lastModificationDate() < storageItem->lastModificationDate();
        } else {
            if (sourceType == StorageServiceTreeWidget::Folder) {
                return false;
            } else {
                return true;
            }
        }
        break;
    }
    }
    return QTreeWidgetItem::operator < ( other );
}

void StorageServiceTreeWidgetItem::setSize(qulonglong size)
{
    if (mSize != size) {
        mSize = size;
        setText(StorageServiceTreeWidget::ColumnSize, KGlobal::locale()->formatByteSize(mSize));
    }
}

void StorageServiceTreeWidgetItem::setDateCreated(const KDateTime &date)
{
    if (date != mCreateDate) {
        mCreateDate = date;
        setText(StorageServiceTreeWidget::ColumnCreated, KGlobal::locale()->formatDateTime(mCreateDate));
    }
}

void StorageServiceTreeWidgetItem::setLastModification(const KDateTime &date)
{
    if (date != mLastModificationDate) {
        mLastModificationDate = date;
        setText(StorageServiceTreeWidget::ColumnLastModification, KGlobal::locale()->formatDateTime(mLastModificationDate));
    }
}

void StorageServiceTreeWidgetItem::setStoreInfo(const QVariantMap &data)
{
    setData(StorageServiceTreeWidget::ColumnName, StorageServiceTreeWidget::Info, data);
}

QVariantMap StorageServiceTreeWidgetItem::storeInfo() const
{
    return data(StorageServiceTreeWidget::ColumnName, StorageServiceTreeWidget::Info).value<QVariantMap>();
}

KDateTime StorageServiceTreeWidgetItem::lastModificationDate() const
{
    return mLastModificationDate;
}

KDateTime StorageServiceTreeWidgetItem::createDate() const
{
    return mCreateDate;
}

qulonglong StorageServiceTreeWidgetItem::size() const
{
    return mSize;
}
