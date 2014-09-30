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
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>

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

    switch (mDeleteType) {
    case PimCommon::StorageServiceDeleteDialog::DeleteAll:
        if (type != StorageServiceTreeWidget::UnKnown) {
            menu->addAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18n("Delete"), this, SIGNAL(deleteFileFolder()));
        }
        break;
    case PimCommon::StorageServiceDeleteDialog::DeleteFiles:
        if (type == StorageServiceTreeWidget::File) {
            menu->addAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18n("Delete File"), this, SIGNAL(deleteFileFolder()));
        }
        break;
    case PimCommon::StorageServiceDeleteDialog::DeleteFolders:
        if (type != StorageServiceTreeWidget::Folder) {
            menu->addAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18n("Delete Folder"), this, SIGNAL(deleteFileFolder()));
        }
        break;
    }
}

StorageServiceDeleteDialog::StorageServiceDeleteDialog(DeleteType type, PimCommon::StorageServiceAbstract *storage, QWidget *parent)
    : QDialog(parent),
      mDeleteType(type),
      mStorage(storage)
{
    switch (mDeleteType) {
    case DeleteAll:
        setWindowTitle(i18n("Delete"));
        break;
    case DeleteFiles:
        setWindowTitle(i18n("Delete Files"));
        break;
    case DeleteFolders:
        setWindowTitle(i18n("Delete Folders"));
        break;
    }
    mStorageServiceProgressIndicator = new PimCommon::StorageServiceProgressIndicator(this);
    connect(mStorageServiceProgressIndicator, &PimCommon::StorageServiceProgressIndicator::updatePixmap, this, &StorageServiceDeleteDialog::slotUpdatePixmap);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mUser1Button = new QPushButton;
    buttonBox->addButton(mUser1Button, QDialogButtonBox::ActionRole);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &StorageServiceDeleteDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &StorageServiceDeleteDialog::reject);
    mUser1Button->setText(i18n("Delete"));

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
    connect(mTreeWidget, &StorageServiceDeleteTreeWidget::deleteFileFolder, this, &StorageServiceDeleteDialog::slotDelete);

    vbox->addWidget(mTreeWidget);
    connect(mTreeWidget, &StorageServiceDeleteTreeWidget::itemDoubleClicked, this, &StorageServiceDeleteDialog::slotItemDoubleClicked);
    w->setLayout(vbox);
    mainLayout->addWidget(w);
    mUser1Button->setEnabled(false);
    mainLayout->addWidget(buttonBox);

    connect(mUser1Button, &QPushButton::clicked, this, &StorageServiceDeleteDialog::slotDelete);
    connect(mStorage, &PimCommon::StorageServiceAbstract::listFolderDone, this, &StorageServiceDeleteDialog::slotListFolderDone);
    connect(mStorage, &PimCommon::StorageServiceAbstract::actionFailed, this, &StorageServiceDeleteDialog::slotActionFailed);
    connect(mStorage, &PimCommon::StorageServiceAbstract::deleteFileDone, this, &StorageServiceDeleteDialog::slotDeleteFileDone, Qt::UniqueConnection);
    connect(mStorage, &PimCommon::StorageServiceAbstract::deleteFolderDone, this, &StorageServiceDeleteDialog::slotDeleteFolderDone, Qt::UniqueConnection);
    connect(mTreeWidget, &StorageServiceDeleteTreeWidget::itemClicked, this, &StorageServiceDeleteDialog::slotItemActivated);
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
    mUser1Button->setEnabled(true);
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
    KConfigGroup group(KSharedConfig::openConfig(), "StorageServiceDeleteDialog");
    const QSize size = group.readEntry("Size", QSize(600, 400));
    mTreeWidget->header()->restoreState(group.readEntry(mStorage->storageServiceName(), QByteArray()));
    if (size.isValid()) {
        resize(size);
    }
}

void StorageServiceDeleteDialog::writeConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();

    KConfigGroup group = config->group(QLatin1String("StorageServiceDeleteDialog"));
    group.writeEntry("Size", size());
    group.writeEntry(mStorage->storageServiceName(), mTreeWidget->header()->saveState());
}

void StorageServiceDeleteDialog::slotItemActivated(QTreeWidgetItem *item, int)
{
    const bool canDelete = item && ((mTreeWidget->type(item) == StorageServiceTreeWidget::File) || (mTreeWidget->type(item) == StorageServiceTreeWidget::Folder));
    mUser1Button->setEnabled(canDelete);
}

void StorageServiceDeleteDialog::slotItemDoubleClicked(QTreeWidgetItem *item, int)
{
    StorageServiceTreeWidgetItem *storageServiceItem = dynamic_cast<StorageServiceTreeWidgetItem *>(item);
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
    StorageServiceTreeWidgetItem *storageServiceItem = dynamic_cast<StorageServiceTreeWidgetItem *>(mTreeWidget->currentItem());
    if (storageServiceItem) {
        if (mTreeWidget->type(storageServiceItem) == StorageServiceTreeWidget::File) {
            if ((mDeleteType == DeleteFiles) || (mDeleteType == DeleteAll)) {
                deleteFile(storageServiceItem);
            }
        } else if (mTreeWidget->type(storageServiceItem) == StorageServiceTreeWidget::Folder) {
            if ((mDeleteType == DeleteFolders) || (mDeleteType == DeleteAll)) {
                deleteFolder(storageServiceItem);
            }
        }
    }
}

