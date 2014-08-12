/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#include "storageservicedeletedialog.h"

#include "storageservice/widgets/storageservicetreewidget.h"
#include "storageservice/widgets/storageservicetreewidgetitem.h"
#include "storageservice/widgets/storageserviceprogresswidget.h"
#include "storageservice/widgets/storageserviceprogressindicator.h"
#include "storageservice/storageserviceabstract.h"

#include <KLocalizedString>
#include <KSharedConfig>
#include <QMenu>

#include <QLabel>
#include <QTreeWidget>
#include <QTimer>
#include <QHeaderView>
#include <QVBoxLayout>

using namespace PimCommon;

StorageServiceDeleteTreeWidget::StorageServiceDeleteTreeWidget(PimCommon::StorageServiceDeleteDialog::DeleteType type, PimCommon::StorageServiceAbstract *storageService, QWidget *parent)
    : PimCommon::StorageServiceTreeWidget(storageService, parent),
      mDeleteType(type)
{
}

StorageServiceDeleteDialog::DeleteType StorageServiceDeleteTreeWidget::deleteType() const
{
    return mDeleteType;
}

void StorageServiceDeleteTreeWidget::createMenuActions(QMenu *menu)
{
    StorageServiceTreeWidget::createMenuActions(menu);
    const PimCommon::StorageServiceTreeWidget::ItemType type = StorageServiceTreeWidget::itemTypeSelected();

    switch(mDeleteType) {
    case PimCommon::StorageServiceDeleteDialog::DeleteAll:
        if (type != StorageServiceTreeWidget::UnKnown)
            menu->addAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18n("Delete"), this, SIGNAL(deleteFileFolder()));
        break;
    case PimCommon::StorageServiceDeleteDialog::DeleteFiles:
        if (type == StorageServiceTreeWidget::File)
            menu->addAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18n("Delete File"), this, SIGNAL(deleteFileFolder()));
        break;
    case PimCommon::StorageServiceDeleteDialog::DeleteFolders:
        if (type != StorageServiceTreeWidget::Folder)
            menu->addAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18n("Delete Folder"), this, SIGNAL(deleteFileFolder()));
        break;
    }
}

StorageServiceDeleteDialog::StorageServiceDeleteDialog(DeleteType type, PimCommon::StorageServiceAbstract *storage, QWidget *parent)
    : KDialog(parent),
      mDeleteType(type),
      mStorage(storage)
{
    switch(mDeleteType) {
    case DeleteAll:
        setCaption(i18n("Delete"));
        break;
    case DeleteFiles:
        setCaption(i18n("Delete Files"));
        break;
    case DeleteFolders:
        setCaption(i18n("Delete Folders"));
        break;
    }
    mStorageServiceProgressIndicator = new PimCommon::StorageServiceProgressIndicator(this);
    connect(mStorageServiceProgressIndicator, SIGNAL(updatePixmap(QPixmap)), this, SLOT(slotUpdatePixmap(QPixmap)));

    setButtons( User1 | Close );
    setButtonText(User1, i18n("Delete"));

    QWidget *w = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout;

    QHBoxLayout *hbox = new QHBoxLayout;
    vbox->addLayout(hbox);
    QLabel *lab = new QLabel(i18n("Select file/folder to delete:"));
    hbox->addWidget(lab);
    mLabelProgressIncator = new QLabel;
    hbox->addWidget(mLabelProgressIncator);
    hbox->setAlignment(mLabelProgressIncator, Qt::AlignLeft);
    mTreeWidget = new StorageServiceDeleteTreeWidget(mDeleteType, storage);
    connect(mTreeWidget, SIGNAL(deleteFileFolder()), this, SLOT(slotDelete()));

    vbox->addWidget(mTreeWidget);
    connect(mTreeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(slotItemDoubleClicked(QTreeWidgetItem*,int)));
    w->setLayout(vbox);
    setMainWidget(w);
    enableButton(User1, false);
    connect(this, SIGNAL(user1Clicked()), this, SLOT(slotDelete()));
    connect(mStorage, SIGNAL(listFolderDone(QString,QVariant)), this, SLOT(slotListFolderDone(QString,QVariant)));
    connect(mStorage, SIGNAL(actionFailed(QString,QString)), this, SLOT(slotActionFailed(QString,QString)));
    connect(mStorage,SIGNAL(deleteFileDone(QString,QString)), this, SLOT(slotDeleteFileDone(QString,QString)), Qt::UniqueConnection);
    connect(mStorage,SIGNAL(deleteFolderDone(QString,QString)), this, SLOT(slotDeleteFolderDone(QString,QString)), Qt::UniqueConnection);
    connect(mTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(slotItemActivated(QTreeWidgetItem*,int)));
    readConfig();
    slotRefreshList();
}

StorageServiceDeleteDialog::~StorageServiceDeleteDialog()
{
    writeConfig();
}

void StorageServiceDeleteDialog::slotRefreshList()
{
    mStorageServiceProgressIndicator->startAnimation();
    mTreeWidget->setEnabled(false);
    mTreeWidget->refreshList();
}

void StorageServiceDeleteDialog::slotDeleteFolderDone(const QString &serviceName, const QString &filename)
{
    Q_EMIT deleteFolderDone(serviceName, filename);
    QTimer::singleShot(0, this, SLOT(slotRefreshList()));
}

void StorageServiceDeleteDialog::slotDeleteFileDone(const QString &serviceName, const QString &filename)
{
    Q_EMIT deleteFileDone(serviceName, filename);
    QTimer::singleShot(0, this, SLOT(slotRefreshList()));
}

void StorageServiceDeleteDialog::slotActionFailed(const QString &serviceName, const QString &data)
{
    Q_UNUSED(serviceName);
    Q_UNUSED(data);
    mStorageServiceProgressIndicator->stopAnimation();
    mTreeWidget->setEnabled(true);
    enableButton(User1, true);
}

void StorageServiceDeleteDialog::slotListFolderDone(const QString &serviceName, const QVariant &data)
{
    mTreeWidget->setEnabled(true);
    mStorageServiceProgressIndicator->stopAnimation();
    mTreeWidget->slotListFolderDone(serviceName, data);
}

void StorageServiceDeleteDialog::slotUpdatePixmap(const QPixmap &pix)
{
    mLabelProgressIncator->setPixmap(pix);
}

void StorageServiceDeleteDialog::readConfig()
{
    KConfigGroup group( KSharedConfig::openConfig(), "StorageServiceDeleteDialog" );
    const QSize size = group.readEntry( "Size", QSize(600, 400) );
    mTreeWidget->header()->restoreState( group.readEntry( mStorage->storageServiceName(), QByteArray() ) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void StorageServiceDeleteDialog::writeConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();

    KConfigGroup group = config->group( QLatin1String("StorageServiceDeleteDialog") );
    group.writeEntry( "Size", size() );
    group.writeEntry(mStorage->storageServiceName(), mTreeWidget->header()->saveState());
}

void StorageServiceDeleteDialog::slotItemActivated(QTreeWidgetItem *item, int)
{
    const bool canDelete = item && ((mTreeWidget->type(item) == StorageServiceTreeWidget::File) || (mTreeWidget->type(item) == StorageServiceTreeWidget::Folder));
    enableButton(User1, canDelete);
}

void StorageServiceDeleteDialog::slotItemDoubleClicked(QTreeWidgetItem *item, int)
{
    StorageServiceTreeWidgetItem *storageServiceItem = dynamic_cast<StorageServiceTreeWidgetItem*>(item);
    if (storageServiceItem) {
        if (mTreeWidget->type(storageServiceItem) == StorageServiceTreeWidget::File) {
            deleteFile(storageServiceItem);
        } else if (mTreeWidget->type(storageServiceItem) == StorageServiceTreeWidget::Folder) {
            deleteFolder(storageServiceItem);
        }
    }
}

void StorageServiceDeleteDialog::deleteFile(StorageServiceTreeWidgetItem *storageServiceItem)
{
    mStorage->deleteFile(mTreeWidget->itemIdentifier(storageServiceItem));
}

void StorageServiceDeleteDialog::deleteFolder(StorageServiceTreeWidgetItem *storageServiceItem)
{
    mStorage->deleteFolder(mTreeWidget->itemIdentifier(storageServiceItem));
}

void StorageServiceDeleteDialog::slotDelete()
{
    StorageServiceTreeWidgetItem *storageServiceItem = dynamic_cast<StorageServiceTreeWidgetItem*>(mTreeWidget->currentItem());
    if (storageServiceItem) {
        if (mTreeWidget->type(storageServiceItem) == StorageServiceTreeWidget::File) {
            if ((mDeleteType == DeleteFiles) || (mDeleteType == DeleteAll))
                deleteFile(storageServiceItem);
        } else if (mTreeWidget->type(storageServiceItem) == StorageServiceTreeWidget::Folder) {
            if ((mDeleteType == DeleteFolders) || (mDeleteType == DeleteAll))
                deleteFolder(storageServiceItem);
        }
    }
}

#include "moc_storageservicedeletedialog.cpp"
