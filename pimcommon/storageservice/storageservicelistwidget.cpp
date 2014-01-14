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

#include "storageservicelistwidget.h"

#include <KIcon>

#include <QListWidgetItem>
#include <QDebug>

using namespace PimCommon;

StorageServiceListWidget::StorageServiceListWidget(QWidget *parent)
    : QListWidget(parent)
{
    setSortingEnabled(true);
}

StorageServiceListWidget::~StorageServiceListWidget()
{

}

void StorageServiceListWidget::addFolder(const QString &name, const QString &ident)
{
    StorageServiceListItem *item = new StorageServiceListItem(name, this);
    item->setData(Ident, ident);
    item->setData(ElementType, Folder);
    item->setIcon(KIcon(QLatin1String("folder")));
    //TODO add default icon etc.
}

void StorageServiceListWidget::addFile(const QString &name, const QString &ident)
{
    StorageServiceListItem *item = new StorageServiceListItem(name, this);
    item->setData(Ident, ident);
    item->setData(ElementType, File);
    //TODO add default icon etc.
}

StorageServiceListWidget::ItemType StorageServiceListWidget::itemTypeSelected() const
{
    QListWidgetItem *item = currentItem();
    if (item) {
        return static_cast<StorageServiceListWidget::ItemType>(item->data(StorageServiceListWidget::ElementType).toInt());
    }
    return StorageServiceListWidget::UnKnown;
}

StorageServiceListWidget::ItemType StorageServiceListWidget::type(QListWidgetItem *item) const
{
    if (item) {
        return static_cast<StorageServiceListWidget::ItemType>(item->data(StorageServiceListWidget::ElementType).toInt());
    }
    return StorageServiceListWidget::UnKnown;
}

QString StorageServiceListWidget::itemIdentifier(QListWidgetItem *item) const
{
    if (item) {
        return item->data(Ident).toString();
    }
    return QString();
}

QString StorageServiceListWidget::itemIdentifierSelected() const
{
    QListWidgetItem *item = currentItem();
    if (item) {
        return item->data(ElementType).toString();
    }
    return QString();
}


StorageServiceListItem::StorageServiceListItem(const QString &name, StorageServiceListWidget *parent)
    : QListWidgetItem(name, parent)
{

}

bool StorageServiceListItem::operator<(const QListWidgetItem &other) const
{
    StorageServiceListWidget::ItemType sourceType = static_cast<StorageServiceListWidget::ItemType>(data(StorageServiceListWidget::ElementType).toInt());
    StorageServiceListWidget::ItemType otherType = static_cast<StorageServiceListWidget::ItemType>(other.data(StorageServiceListWidget::ElementType).toInt());
    if (sourceType == otherType) {
        return text() < other.text();
    } else {
        if (sourceType == StorageServiceListWidget::Folder) {
            return true;
        } else {
            return false;
        }
    }
}

