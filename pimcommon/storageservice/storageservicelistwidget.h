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

#ifndef PIMCOMMON_STORAGESERVICELISTWIDGET_H
#define PIMCOMMON_STORAGESERVICELISTWIDGET_H

#include <QListWidget>
#include "pimcommon_export.h"

namespace PimCommon {
class StorageServiceListWidget;
class StorageServiceListItem : public QListWidgetItem
{
public:
    StorageServiceListItem(const QString &name, StorageServiceListWidget *parent);
    virtual bool operator<(const QListWidgetItem &other) const;
};

class PIMCOMMON_EXPORT StorageServiceListWidget : public QListWidget
{
    Q_OBJECT
public:
    enum ItemType {
        UnKnown = -1,
        Folder = 0,
        File = 1
    };
    enum StorageServiceData {
        ElementType = Qt::UserRole + 1,
        Ident = Qt::UserRole + 2,
        Size = Qt::UserRole + 3
    };

    explicit StorageServiceListWidget(QWidget *parent=0);
    ~StorageServiceListWidget();

    StorageServiceListItem *addFolder(const QString &name, const QString &ident);
    StorageServiceListItem *addFile(const QString &name, const QString &ident, const QString &mimetype = QString());

    StorageServiceListWidget::ItemType itemTypeSelected() const;
    StorageServiceListWidget::ItemType type(QListWidgetItem *item) const;
    QString itemIdentifier(QListWidgetItem *item) const;
    QString itemIdentifierSelected() const;
};
}

#endif // STORAGESERVICELISTWIDGET_H
