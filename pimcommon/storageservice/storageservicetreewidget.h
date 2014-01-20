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

#ifndef PIMCOMMON_STORAGESERVICETREEWIDGET_H
#define PIMCOMMON_STORAGESERVICETREEWIDGET_H

#include <QTreeWidget>
#include "pimcommon_export.h"

namespace PimCommon {
class StorageServiceTreeWidget;
class PIMCOMMON_EXPORT StorageServiceListItem : public QTreeWidgetItem
{
public:
    StorageServiceListItem(StorageServiceTreeWidget *parent);
    bool operator<(const QTreeWidgetItem &other) const;
    void setSize(qulonglong size);
    void setDateCreated(const QString &date);
    void setLastModification(const QString &date);
};

class PIMCOMMON_EXPORT StorageServiceTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    enum ItemType {
        UnKnown = -1,
        Folder = 0,
        File = 1,
        MoveUpType = 2
    };
    enum StorageServiceData {
        ElementType = Qt::UserRole + 1,
        Ident = Qt::UserRole + 2
    };
    enum TreeWidgetColumn {
        ColumnName = 0,
        ColumnSize = 1,
        ColumnCreated = 2,
        ColumnLastModification = 3
    };

    explicit StorageServiceTreeWidget(QWidget *parent=0);
    ~StorageServiceTreeWidget();

    StorageServiceListItem *addFolder(const QString &name, const QString &ident);
    StorageServiceListItem *addFile(const QString &name, const QString &ident, const QString &mimetype = QString());

    StorageServiceTreeWidget::ItemType itemTypeSelected() const;
    StorageServiceTreeWidget::ItemType type(QTreeWidgetItem *item) const;
    QString itemIdentifier(QTreeWidgetItem *item) const;
    QString itemIdentifierSelected() const;
    void createMoveUpItem();
};
}

#endif // STORAGESERVICETREEWIDGET_H
