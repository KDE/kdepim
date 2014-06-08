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
#include "storageservice/storageserviceabstract.h"
#include "storageservice/dialog/storageservicepropertiesdialog.h"

#include <KIcon>
#include <KLocalizedString>
#include <KGlobal>
#include <KLocale>
#include <KMimeType>
#include <QMenu>
#include <KDateTime>

#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QDebug>
#include <QTimer>

using namespace PimCommon;

StorageServiceTreeWidget::StorageServiceTreeWidget(StorageServiceAbstract *storageService, QWidget *parent)
    : QTreeWidget(parent),
      mStorageService(storageService)
{
    setContextMenuPolicy( Qt::CustomContextMenu );
    connect( this, SIGNAL(customContextMenuRequested(QPoint)), SLOT(slotContextMenu(QPoint)) );
    setSortingEnabled(true);
    setAlternatingRowColors(true);
    setRootIsDecorated(false);
    QStringList lst;
    lst << i18n("Name") << i18n("Size") << i18n("Created") << i18n("Last Modified");
    setHeaderLabels(lst);
    header()->setMovable(false);
    connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(slotItemDoubleClicked(QTreeWidgetItem*,int)));
}

StorageServiceTreeWidget::~StorageServiceTreeWidget()
{

}

void StorageServiceTreeWidget::slotMoveUp()
{
    if (parentFolder() == currentFolder())
        return;
    setCurrentFolder(parentFolder());
    QTimer::singleShot(0, this, SLOT(refreshList()));
}

void StorageServiceTreeWidget::createUpAction(QMenu *menu)
{
    menu->addAction( KIcon(QLatin1String("go-up")),  i18n("Up"), this, SLOT(slotMoveUp()));
}

void StorageServiceTreeWidget::createPropertiesAction(QMenu *menu)
{
    menu->addAction(KIcon(QLatin1String("document-properties")), i18n("Properties"), this, SLOT(slotProperties()));
}

void StorageServiceTreeWidget::createMenuActions(QMenu *menu)
{
    createUpAction(menu);
    const PimCommon::StorageServiceTreeWidget::ItemType type = StorageServiceTreeWidget::itemTypeSelected();
    if ((type == StorageServiceTreeWidget::File) || (type == StorageServiceTreeWidget::Folder)) {
        menu->addSeparator();
        createPropertiesAction(menu);
    }
}

void StorageServiceTreeWidget::slotContextMenu(const QPoint &pos)
{
    QMenu *menu = new QMenu( this );
    createMenuActions(menu);
    menu->exec( mapToGlobal( pos ) );
    delete menu;
}

void StorageServiceTreeWidget::createMoveUpItem()
{
    StorageServiceTreeWidgetItem *item = new StorageServiceTreeWidgetItem(this);
    item->setText(ColumnName, QLatin1String(".."));
    item->setData(ColumnName, ElementType, MoveUpType);
    item->setIcon(ColumnName, KIcon(QLatin1String("go-up")));
}

StorageServiceTreeWidgetItem *StorageServiceTreeWidget::addFolder(const QString &name, const QString &ident)
{
    StorageServiceTreeWidgetItem *item = new StorageServiceTreeWidgetItem(this);
    item->setText(ColumnName, name);
    item->setData(ColumnName, Ident, ident);
    item->setData(ColumnName, ElementType, Folder);
    item->setIcon(ColumnName, KIcon(QLatin1String("folder")));
    return item;
}

StorageServiceTreeWidgetItem *StorageServiceTreeWidget::addFile(const QString &name, const QString &ident, const QString &mimetype)
{
    StorageServiceTreeWidgetItem *item = new StorageServiceTreeWidgetItem(this);
    item->setText(ColumnName, name);
    item->setData(ColumnName, Ident, ident);
    item->setData(ColumnName, ElementType, File);
    if (!mimetype.isEmpty()) {
        KMimeType::Ptr mime = KMimeType::mimeType( mimetype, KMimeType::ResolveAliases );
        if (mime)
            item->setIcon(ColumnName, KIcon(mime->iconName()));
    }
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

QVariantMap StorageServiceTreeWidget::itemInformationSelected() const
{
    QTreeWidgetItem *item = currentItem();
    if (item) {
        return static_cast<StorageServiceTreeWidgetItem*>(item)->storeInfo();
    }
    return QVariantMap();
}

void StorageServiceTreeWidget::setCurrentFolder(const QString &folder)
{
    mCurrentFolder = folder;
}

QString StorageServiceTreeWidget::currentFolder() const
{
    return mCurrentFolder;
}

void StorageServiceTreeWidget::setParentFolder(const QString &folder)
{
    mParentFolder = folder;
}

QString StorageServiceTreeWidget::parentFolder() const
{
    return mParentFolder;
}

void StorageServiceTreeWidget::refreshList()
{
    mStorageService->listFolder(mCurrentFolder);
}

void StorageServiceTreeWidget::slotListFolderDone(const QString &serviceName, const QVariant &data)
{
    Q_UNUSED(serviceName);
    const QString parentFolder = mStorageService->fillListWidget(this, data, currentFolder());
    setParentFolder(parentFolder);
}

void StorageServiceTreeWidget::goToFolder(const QString &folder, bool addToHistory)
{
    if (folder == currentFolder())
        return;
    if (addToHistory)
        Q_EMIT changeFolder(currentFolder(), parentFolder());
    setCurrentFolder(folder);
    QTimer::singleShot(0, this, SLOT(refreshList()));
}

void StorageServiceTreeWidget::slotItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    if (item) {
        StorageServiceTreeWidget::ItemType itemType = type(item);
        switch (itemType) {
        case StorageServiceTreeWidget::Folder:
        {
            const QString folder = itemIdentifierSelected();            
            goToFolder(folder);
            break;
        }
        case StorageServiceTreeWidget::MoveUpType:
            slotMoveUp();
            break;
        case StorageServiceTreeWidget::File:
            Q_EMIT fileDoubleClicked();
            break;
        default:
            break;
        }
    }
}

void StorageServiceTreeWidget::slotProperties()
{
    const QMap<QString, QString> information = mStorageService->itemInformation(itemInformationSelected());
    if (!information.isEmpty()) {
        QPointer<StorageServicePropertiesDialog> dlg = new StorageServicePropertiesDialog(information, this);
        dlg->exec();
        delete dlg;
    }
}

StorageServiceTreeWidgetItem::StorageServiceTreeWidgetItem(StorageServiceTreeWidget *parent)
    : QTreeWidgetItem(parent)
{

}

bool StorageServiceTreeWidgetItem::operator<(const QTreeWidgetItem &other) const
{
    StorageServiceTreeWidget::ItemType sourceType = static_cast<StorageServiceTreeWidget::ItemType>(data(StorageServiceTreeWidget::ColumnName, StorageServiceTreeWidget::ElementType).toInt());
    StorageServiceTreeWidget::ItemType otherType = static_cast<StorageServiceTreeWidget::ItemType>(other.data(StorageServiceTreeWidget::ColumnName, StorageServiceTreeWidget::ElementType).toInt());
    if (sourceType == StorageServiceTreeWidget::MoveUpType) {
        return false;
    } else if (otherType == StorageServiceTreeWidget::MoveUpType) {
        return false;
    } else if (sourceType == otherType) {
        return text(StorageServiceTreeWidget::ColumnName) < other.text(StorageServiceTreeWidget::ColumnName);
    } else {
        if (sourceType == StorageServiceTreeWidget::Folder) {
            return false;
        } else {
            return true;
        }
    }
}

void StorageServiceTreeWidgetItem::setSize(qulonglong size)
{
    setText(StorageServiceTreeWidget::ColumnSize, KLocale::global()->formatByteSize(size));
}

void StorageServiceTreeWidgetItem::setDateCreated(const KDateTime &date)
{
    setText(StorageServiceTreeWidget::ColumnCreated, KLocale::global()->formatDateTime(date));
}

void StorageServiceTreeWidgetItem::setLastModification(const KDateTime &date)
{
    setText(StorageServiceTreeWidget::ColumnLastModification, KLocale::global()->formatDateTime(date));
}

void StorageServiceTreeWidgetItem::setStoreInfo(const QVariantMap &data)
{
    setData(StorageServiceTreeWidget::ColumnName, StorageServiceTreeWidget::Info, data);
}

QVariantMap StorageServiceTreeWidgetItem::storeInfo() const
{
    return data(StorageServiceTreeWidget::ColumnName, StorageServiceTreeWidget::Info).value<QVariantMap>();
}

#include "moc_storageservicetreewidget.cpp"
