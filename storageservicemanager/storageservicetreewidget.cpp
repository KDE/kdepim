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
#include "storageservicemanagerglobalconfig.h"

#include <KMenu>
#include <KInputDialog>
#include <KLocalizedString>
#include <KFileDialog>
#include <KGlobalSettings>
#include <KMessageBox>

#include <QPainter>

StorageServiceTreeWidget::StorageServiceTreeWidget(PimCommon::StorageServiceAbstract *storageService, QWidget *parent)
    : PimCommon::StorageServiceTreeWidget(parent),
      mStorageService(storageService),
      mInitialized(false)
{
    mCapabilities = mStorageService->capabilities();
    //Single selection for the moment
    setSelectionMode(QAbstractItemView::SingleSelection);
    setContextMenuPolicy( Qt::CustomContextMenu );
    connect( this, SIGNAL(customContextMenuRequested(QPoint)), SLOT(slotContextMenu(QPoint)) );
    connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(slotItemDoubleClicked(QTreeWidgetItem*,int)));
    connect( KGlobalSettings::self(), SIGNAL(kdisplayFontChanged()), this, SLOT(slotGeneralFontChanged()));
    connect( KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()), this, SLOT(slotGeneralPaletteChanged()));

}

StorageServiceTreeWidget::~StorageServiceTreeWidget()
{

}

void StorageServiceTreeWidget::slotGeneralPaletteChanged()
{
    const QPalette palette = viewport()->palette();
    QColor color = palette.text().color();
    color.setAlpha( 128 );
    mTextColor = color;
}

void StorageServiceTreeWidget::slotGeneralFontChanged()
{
    setFont( KGlobalSettings::generalFont() );
}

void StorageServiceTreeWidget::setIsInitialized()
{
    mInitialized = true;
}

void StorageServiceTreeWidget::slotContextMenu(const QPoint &pos)
{
    if (!mInitialized)
        return;

    KMenu *menu = new KMenu( this );
    menu->addAction( i18n("Up"), this, SLOT(slotMoveUp()));
    const PimCommon::StorageServiceTreeWidget::ItemType type = StorageServiceTreeWidget::itemTypeSelected();
    if (type != StorageServiceTreeWidget::UnKnown) {
        QAction *act = new QAction(menu);
        act->setSeparator(true);
        menu->addAction(act);
        if (type == StorageServiceTreeWidget::File) {
            if (mCapabilities & PimCommon::StorageServiceAbstract::RenameFileCapabilitity)
                menu->addAction(i18n("Rename File"), this, SLOT(slotRenameFile()));
            if (mCapabilities & PimCommon::StorageServiceAbstract::DeleteFileCapability)
                menu->addAction(i18n("Delete File"), this, SLOT(slotDeleteFile()));
            if (mCapabilities & PimCommon::StorageServiceAbstract::ShareLinkCapability)
                menu->addAction(i18n("Share File"), this, SLOT(slotShareFile()));
            if (mCapabilities & PimCommon::StorageServiceAbstract::DownloadFileCapability)
                menu->addAction(i18n("Download File"), this, SLOT(slotDownloadFile()));
            if (mCapabilities & PimCommon::StorageServiceAbstract::MoveFileCapability)
                menu->addAction(i18n("Cut"), this, SLOT(slotCutFile()));
            if (mCapabilities & PimCommon::StorageServiceAbstract::CopyFileCapability)
                menu->addAction(i18n("Copy"), this, SLOT(slotCopyFile()));

        } else if (type == StorageServiceTreeWidget::Folder) {
            if (mCapabilities & PimCommon::StorageServiceAbstract::RenameFolderCapability)
                menu->addAction(i18n("Rename Folder"), this, SLOT(slotRenameFolder()));
            if (mCapabilities & PimCommon::StorageServiceAbstract::DeleteFolderCapability)
                menu->addAction(i18n("Delete Folder"), this, SLOT(slotDeleteFolder()));
            if (mCapabilities & PimCommon::StorageServiceAbstract::MoveFolderCapability)
                menu->addAction(i18n("Cut"), this, SLOT(slotCutFolder()));
            if (mCapabilities & PimCommon::StorageServiceAbstract::CopyFolderCapability)
                menu->addAction(i18n("Copy"), this, SLOT(slotCopyFolder()));
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

    if (mCopyItem.moveItem) {
        if (mCopyItem.type == FileType) {
            if (mCapabilities & PimCommon::StorageServiceAbstract::MoveFileCapability) {
                menu->addAction(i18n("Paste"), this, SLOT(slotMoveFile()));
            }
        } else if (mCopyItem.type == FolderType) {
            if (mCapabilities & PimCommon::StorageServiceAbstract::MoveFolderCapability) {
                menu->addAction(i18n("Paste"), this, SLOT(slotMoveFolder()));
            }
        }
    } else {
        if (mCopyItem.type == FileType) {
            if (mCapabilities & PimCommon::StorageServiceAbstract::CopyFileCapability) {
                menu->addAction(i18n("Paste"), this, SLOT(slotPasteFile()));
            }
        } else if (mCopyItem.type == FolderType) {
            if (mCapabilities & PimCommon::StorageServiceAbstract::CopyFolderCapability) {
                menu->addAction(i18n("Paste"), this, SLOT(slotPasteFolder()));
            }
        }
    }

    menu->exec( mapToGlobal( pos ) );
    delete menu;
}

void StorageServiceTreeWidget::slotMoveFolder()
{
    //TODO
    const QString destination;
    mStorageService->moveFolder(mCopyItem.identifier, destination);
}

void StorageServiceTreeWidget::slotMoveFile()
{
    //TODO
    const QString destination;
    mStorageService->moveFile(mCopyItem.identifier, destination);
}

void StorageServiceTreeWidget::slotPasteFolder()
{
    //TODO
    const QString destination;
    mStorageService->copyFolder(mCopyItem.identifier, destination);
}

void StorageServiceTreeWidget::slotPasteFile()
{
    //TODO
    const QString destination;
    mStorageService->copyFile(mCopyItem.identifier, destination);
}


void StorageServiceTreeWidget::slotRenameFolder()
{
    const QString oldFolderName = itemIdentifierSelected();
    const QString folder = KInputDialog::getText(i18n("Rename Folder Name"), i18n("Folder:"));
    if (!folder.isEmpty()) {
        if (oldFolderName != folder) {
            mStorageService->renameFolder(oldFolderName, folder);
        }
    }
}

void StorageServiceTreeWidget::slotRenameFile()
{
    const QString oldFolderName = itemIdentifierSelected();
    const QString folder = KInputDialog::getText(i18n("Rename Filename"), i18n("Filename:"));
    if (!folder.isEmpty()) {
        if (oldFolderName != folder) {
            mStorageService->renameFolder(oldFolderName, folder);
        }
    }
}

void StorageServiceTreeWidget::slotCreateFolder()
{
    const QString folder = KInputDialog::getText(i18n("Folder Name"), i18n("Folder:"));
    if (!folder.isEmpty()) {
        mStorageService->createFolder(folder);
    }
}

void StorageServiceTreeWidget::slotDeleteFolder()
{
    if (itemTypeSelected() == StorageServiceTreeWidget::Folder) {
        const QString folder = itemIdentifierSelected();
        const QString name = currentItem()->text(0);
        if (KMessageBox::Yes == KMessageBox::questionYesNo(this, i18n("Are you sure that you want to delete \"%1\"?", name))) {
            if (!folder.isEmpty()) {
                mStorageService->deleteFolder(folder);
            }
        }
    }
}

void StorageServiceTreeWidget::slotMoveUp()
{
    Q_EMIT moveUp();
}

void StorageServiceTreeWidget::slotDeleteFile()
{
    if (itemTypeSelected() == StorageServiceTreeWidget::File) {
        const QString filename = itemIdentifierSelected();
        const QString name = currentItem()->text(0);
        if (KMessageBox::Yes == KMessageBox::questionYesNo(this, i18n("Are you sure that you want to delete \"%1\"?", name) )) {
            if (!filename.isEmpty()) {
                mStorageService->deleteFile(filename);
            }
        }
    }
}

void StorageServiceTreeWidget::slotShareFile()
{
    if (itemTypeSelected() == StorageServiceTreeWidget::File) {
        const QString filename = itemIdentifierSelected();
        if (!filename.isEmpty()) {
            //mStorageService->shareLink(filename);
        }
    }
}

void StorageServiceTreeWidget::slotDownloadFile()
{
    if (itemTypeSelected() == StorageServiceTreeWidget::File) {
        const QString filename = itemIdentifierSelected();
        if (!filename.isEmpty()) {
            QString destination = StorageServiceManagerGlobalConfig::self()->downloadDirectory();
            if (destination.isEmpty()) {
                destination = KFileDialog::getExistingDirectory(KUrl(), this);
                if (destination.isEmpty())
                    return;
            }
            mStorageService->downloadFile(filename, destination);
        }
    }
}

void StorageServiceTreeWidget::slotUploadFile()
{
    const QString filename = KFileDialog::getOpenFileName(KUrl(), QLatin1String("*"), this);
    if (!filename.isEmpty())
        mStorageService->uploadFile(filename);
}

void StorageServiceTreeWidget::slotItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    if (item) {
        if (type(item) == StorageServiceTreeWidget::Folder) {
            const QString folder = itemIdentifierSelected();
            Q_EMIT goToFolder(folder);
        }
    }
}

void StorageServiceTreeWidget::paintEvent( QPaintEvent *event )
{
    if ( mInitialized ) {
        PimCommon::StorageServiceTreeWidget::paintEvent(event);
    } else {
        QPainter p( viewport() );

        QFont font = p.font();
        font.setItalic( true );
        p.setFont( font );

        if (!mTextColor.isValid()) {
            slotGeneralPaletteChanged();
        }
        p.setPen( mTextColor );
        p.drawText( QRect( 0, 0, width(), height() ), Qt::AlignCenter, i18n("Storage service not initialized.") );
    }
}


void StorageServiceTreeWidget::slotCutFile()
{
    mCopyItem.moveItem = true;
    mCopyItem.type = FileType;
    mCopyItem.identifier = itemIdentifierSelected();
}

void StorageServiceTreeWidget::slotCutFolder()
{
    mCopyItem.moveItem = true;
    mCopyItem.type = FolderType;
    mCopyItem.identifier = itemIdentifierSelected();
}

void StorageServiceTreeWidget::slotCopyFile()
{
    mCopyItem.moveItem = false;
    mCopyItem.type = FileType;
    mCopyItem.identifier = itemIdentifierSelected();
}

void StorageServiceTreeWidget::slotCopyFolder()
{
    mCopyItem.moveItem = false;
    mCopyItem.type = FolderType;
    mCopyItem.identifier = itemIdentifierSelected();
}
