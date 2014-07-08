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
#include <KDateTime>
#include "pimcommon_export.h"
class KMenu;
namespace PimCommon {
class StorageServiceAbstract;
class StorageServiceTreeWidgetItem;
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
        Ident = Qt::UserRole + 2,
        Info = Qt::UserRole + 3
    };
    enum TreeWidgetColumn {
        ColumnName = 0,
        ColumnSize = 1,
        ColumnCreated = 2,
        ColumnLastModification = 3
    };

    explicit StorageServiceTreeWidget(PimCommon::StorageServiceAbstract *storageService, QWidget *parent=0);
    ~StorageServiceTreeWidget();

    void setCurrentFolder(const QString &folder);
    QString currentFolder() const;

    void setParentFolder(const QString &folder);
    QString parentFolder() const;

    void goToFolder(const QString &folder, bool addToHistory = true);

    StorageServiceTreeWidgetItem *addFolder(const QString &name, const QString &ident);
    StorageServiceTreeWidgetItem *addFile(const QString &name, const QString &ident, const QString &mimetype = QString());

    StorageServiceTreeWidget::ItemType itemTypeSelected() const;
    StorageServiceTreeWidget::ItemType type(QTreeWidgetItem *item) const;
    QString itemIdentifier(QTreeWidgetItem *item) const;
    QString itemIdentifierSelected() const;
    void createMoveUpItem();

    QVariantMap itemInformationSelected() const;

    void createUpAction(KMenu *menu);
    void createPropertiesAction(KMenu *menu);

Q_SIGNALS:
    void fileDoubleClicked();
    void changeFolder(const QString &currentFolder, const QString &parentFolder);

public Q_SLOTS:
    void refreshList();
    void slotListFolderDone(const QString &serviceName, const QVariant &data);


protected:
    QString mCurrentFolder;
    QString mParentFolder;
    PimCommon::StorageServiceAbstract *mStorageService;    
    virtual void createMenuActions(KMenu *menu);

private Q_SLOTS:
    void slotItemDoubleClicked(QTreeWidgetItem *item, int column);
    void slotMoveUp();
    void slotContextMenu(const QPoint &pos);
    void slotProperties();    
};
}

#endif // STORAGESERVICETREEWIDGET_H
