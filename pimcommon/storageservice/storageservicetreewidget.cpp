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

#include "storageservicetreewidget.h"

#include <KIcon>
#include <KLocalizedString>

#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QDebug>

using namespace PimCommon;

StorageServiceTreeWidget::StorageServiceTreeWidget(QWidget *parent)
    : QTreeWidget(parent)
{
    setSortingEnabled(true);
    setAlternatingRowColors(true);
    setRootIsDecorated(false);
    QStringList lst;
    lst << i18n("Name") << i18n("Size") << i18n("Created") << i18n("Last Modified");
    setHeaderLabels(lst);
    header()->setMovable(false);
}

StorageServiceTreeWidget::~StorageServiceTreeWidget()
{

}

void StorageServiceTreeWidget::createMoveUpItem()
{
    StorageServiceListItem *item = new StorageServiceListItem(this);
    item->setText(ColumnName, QLatin1String(".."));
    item->setData(ColumnName, ElementType, MoveUpType);
    item->setIcon(ColumnName, KIcon(QLatin1String("go-up")));
}

StorageServiceListItem *StorageServiceTreeWidget::addFolder(const QString &name, const QString &ident)
{
    StorageServiceListItem *item = new StorageServiceListItem(this);
    item->setText(ColumnName, name);
    item->setData(ColumnName, Ident, ident);
    item->setData(ColumnName, ElementType, Folder);
    item->setIcon(ColumnName, KIcon(QLatin1String("folder")));
    return item;
}

StorageServiceListItem *StorageServiceTreeWidget::addFile(const QString &name, const QString &ident, const QString &mimetype)
{
    StorageServiceListItem *item = new StorageServiceListItem(this);
    item->setText(ColumnName, name);
    item->setData(ColumnName, Ident, ident);
    item->setData(ColumnName, ElementType, File);
    //TODO fix mimetype;
    return item;
}

StorageServiceTreeWidget::ItemType StorageServiceTreeWidget::itemTypeSelected() const
{
    QTreeWidgetItem *item = currentItem();
    if (item) {
        return static_cast<StorageServiceTreeWidget::ItemType>(item->data(ColumnName, StorageServiceTreeWidget::ElementType).toInt());
    }
    return StorageServiceTreeWidget::UnKnown;
}

StorageServiceTreeWidget::ItemType StorageServiceTreeWidget::type(QTreeWidgetItem *item) const
{
    if (item) {
        return static_cast<StorageServiceTreeWidget::ItemType>(item->data(ColumnName, StorageServiceTreeWidget::ElementType).toInt());
    }
    return StorageServiceTreeWidget::UnKnown;
}

QString StorageServiceTreeWidget::itemIdentifier(QTreeWidgetItem *item) const
{
    if (item) {
        return item->data(ColumnName, Ident).toString();
    }
    return QString();
}

QString StorageServiceTreeWidget::itemIdentifierSelected() const
{
    QTreeWidgetItem *item = currentItem();
    if (item) {
        return item->data(ColumnName, Ident).toString();
    }
    return QString();
}

StorageServiceListItem::StorageServiceListItem(StorageServiceTreeWidget *parent)
    : QTreeWidgetItem(parent)
{

}

bool StorageServiceListItem::operator<(const QTreeWidgetItem &other) const
{
    StorageServiceTreeWidget::ItemType sourceType = static_cast<StorageServiceTreeWidget::ItemType>(data(StorageServiceTreeWidget::ColumnName, StorageServiceTreeWidget::ElementType).toInt());
    StorageServiceTreeWidget::ItemType otherType = static_cast<StorageServiceTreeWidget::ItemType>(other.data(StorageServiceTreeWidget::ColumnName, StorageServiceTreeWidget::ElementType).toInt());
    if (sourceType == StorageServiceTreeWidget::MoveUpType) {
        return false;
    } else if (sourceType == otherType) {
        return text(StorageServiceTreeWidget::ColumnName) < other.text(StorageServiceTreeWidget::ColumnName);
    } else {
        if (sourceType == StorageServiceTreeWidget::Folder) {
            return true;
        } else {
            return false;
        }
    }
}

void StorageServiceListItem::setSize(qulonglong size)
{
    setText(StorageServiceTreeWidget::ColumnSize, QString::number(size));
}

void StorageServiceListItem::setDateCreated(const QString &date)
{
    setText(StorageServiceTreeWidget::ColumnCreated, date);
}

void StorageServiceListItem::setLastModification(const QString &date)
{
    setText(StorageServiceTreeWidget::ColumnLastModification, date);
}
