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

#include <KMenu>
#include <KInputDialog>
#include <KLocalizedString>
#include <KFileDialog>

StorageServiceListWidget::StorageServiceListWidget(PimCommon::StorageServiceAbstract *storageService, QWidget *parent)
    : PimCommon::StorageServiceListWidget(parent),
      mStorageService(storageService)
{
    mCapabilities = mStorageService->capabilities();
    //Single selection for the moment
    setSelectionMode(QAbstractItemView::SingleSelection);
    setContextMenuPolicy( Qt::CustomContextMenu );
    connect( this, SIGNAL(customContextMenuRequested(QPoint)), SLOT(slotContextMenu(QPoint)) );
    connect(this, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(slotItemDoubleClicked(QListWidgetItem*)));
}

StorageServiceListWidget::~StorageServiceListWidget()
{

}

void StorageServiceListWidget::slotContextMenu(const QPoint &pos)
{
    KMenu *menu = new KMenu( this );
    menu->addAction( i18n("Up"), this, SLOT(slotMoveUp()));
    const PimCommon::StorageServiceListWidget::ItemType type = StorageServiceListWidget::itemTypeSelected();
    if (type != StorageServiceListWidget::UnKnown) {
        QAction *act = new QAction(menu);
        act->setSeparator(true);
        menu->addAction(act);
        if (type == StorageServiceListWidget::File) {
            if (mCapabilities & PimCommon::StorageServiceAbstract::DeleteFileCapability)
                menu->addAction(i18n("Delete File"), this, SLOT(slotDeleteFile()));
            if (mCapabilities & PimCommon::StorageServiceAbstract::ShareLinkCapability)
                menu->addAction(i18n("Share File"), this, SLOT(slotShareFile()));
            if (mCapabilities & PimCommon::StorageServiceAbstract::DownloadFileCapability)
                menu->addAction(i18n("Download File"), this, SLOT(slotDownloadFile()));
        } else if (type == StorageServiceListWidget::Folder) {
            if (mCapabilities & PimCommon::StorageServiceAbstract::DeleteFolderCapability)
                menu->addAction(i18n("Delete Folder"), this, SLOT(slotDeleteFolder()));
        }
    }
    QAction *act = new QAction(menu);
    act->setSeparator(true);
    menu->addAction(act);
    if (mCapabilities & PimCommon::StorageServiceAbstract::UploadFileCapability)
        menu->addAction(i18n("Upload File"), this, SLOT(slotUploadFile()));
    act = new QAction(menu);
    act->setSeparator(true);
    menu->addAction(act);
    if (mCapabilities & PimCommon::StorageServiceAbstract::CreateFolderCapability)
        menu->addAction(i18n("Create Folder"), this, SLOT(slotCreateFolder()));
    //TODO
    menu->exec( mapToGlobal( pos ) );
    delete menu;
}

void StorageServiceListWidget::slotCreateFolder()
{
    const QString folder = KInputDialog::getText(i18n("Folder Name"), i18n("Folder:"));
    if (!folder.isEmpty()) {
        mStorageService->createFolder(folder);
    }
}

void StorageServiceListWidget::slotDeleteFolder()
{
    if (itemTypeSelected() == StorageServiceListWidget::Folder) {

    }
}

void StorageServiceListWidget::slotMoveUp()
{
    //TODO ?
}

void StorageServiceListWidget::slotDeleteFile()
{
    if (itemTypeSelected() == StorageServiceListWidget::File) {

    }
}

void StorageServiceListWidget::slotShareFile()
{
    if (itemTypeSelected() == StorageServiceListWidget::File) {

    }
}

void StorageServiceListWidget::slotDownloadFile()
{
    if (itemTypeSelected() == StorageServiceListWidget::File) {

    }
}

void StorageServiceListWidget::slotUploadFile()
{
    const QString filename = KFileDialog::getOpenFileName(KUrl(), QLatin1String("*"), this);
    if (!filename.isEmpty())
        mStorageService->uploadFile(filename);
}

void StorageServiceListWidget::slotItemDoubleClicked(QListWidgetItem *item)
{
    if (item) {
        if (item->type() == StorageServiceListWidget::Folder) {
            //Move into directory
        }
    }
    //TODO
}
