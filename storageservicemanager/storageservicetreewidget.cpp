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
#include "storageservice/dialog/storageservicechecknamedialog.h"
#include "storageservicemanagerglobalconfig.h"
#include "storageservice/storageserviceprogressmanager.h"

#include <QMenu>
#include <QInputDialog>
#include <KLocalizedString>
#include <KFileDialog>
#include <KGlobalSettings>
#include <KMessageBox>
#include <KLocalizedString>
#include <KUrl>
#include <KGlobal>

#include <QPainter>
#include <QHeaderView>
#include <QPointer>
#include <KSharedConfig>
#include <KFormat>
#include <QFontDatabase>
#include <QFileDialog>

StorageServiceTreeWidget::StorageServiceTreeWidget(PimCommon::StorageServiceAbstract *storageService, QWidget *parent)
    : PimCommon::StorageServiceTreeWidget(storageService, parent),
      mInitialized(false)
{
    mCapabilities = mStorageService->capabilities();
    //Single selection for the moment
    setSelectionMode(QAbstractItemView::SingleSelection);
    connect(this, &StorageServiceTreeWidget::fileDoubleClicked, this, &StorageServiceTreeWidget::slotFileDoubleClicked);
    connect(KGlobalSettings::self(), &KGlobalSettings::kdisplayFontChanged, this, &StorageServiceTreeWidget::slotGeneralFontChanged);
    connect(KGlobalSettings::self(), &KGlobalSettings::kdisplayPaletteChanged, this, &StorageServiceTreeWidget::slotGeneralPaletteChanged);
    readConfig();
}

StorageServiceTreeWidget::~StorageServiceTreeWidget()
{
    qDebug() << " StorageServiceTreeWidget::~StorageServiceTreeWidget()";
    writeConfig();
}

void StorageServiceTreeWidget::writeConfig()
{
    KConfigGroup grp(KSharedConfig::openConfig(), "StorageServiceTreeWidget");
    grp.writeEntry(mStorageService->storageServiceName(), header()->saveState());
}

void StorageServiceTreeWidget::readConfig()
{
    KConfigGroup grp(KSharedConfig::openConfig(), "StorageServiceTreeWidget");
    header()->restoreState(grp.readEntry(mStorageService->storageServiceName(), QByteArray()));
}

void StorageServiceTreeWidget::slotGeneralPaletteChanged()
{
    const QPalette palette = viewport()->palette();
    QColor color = palette.text().color();
    color.setAlpha(128);
    mTextColor = color;
}

void StorageServiceTreeWidget::slotGeneralFontChanged()
{
    setFont(QFontDatabase::systemFont(QFontDatabase::GeneralFont));
}

void StorageServiceTreeWidget::setIsInitialized()
{
    if (!mInitialized) {
        mInitialized = true;
        Q_EMIT listFileWasInitialized();
    }
}

void StorageServiceTreeWidget::createMenuActions(QMenu *menu)
{
    if (mInitialized) {
        createUpAction(menu);
        const PimCommon::StorageServiceTreeWidget::ItemType type = StorageServiceTreeWidget::itemTypeSelected();
        if (type != StorageServiceTreeWidget::UnKnown) {
            if (type == StorageServiceTreeWidget::File) {
                if (mCapabilities & PimCommon::StorageServiceAbstract::MoveFileCapability) {
                    menu->addAction(QIcon::fromTheme(QLatin1String("edit-cut")), i18n("Cut"), this, SLOT(slotCutFile()));
                }
                if (mCapabilities & PimCommon::StorageServiceAbstract::CopyFileCapability) {
                    menu->addAction(QIcon::fromTheme(QLatin1String("edit-copy")), i18n("Copy"), this, SLOT(slotCopyFile()));
                }
                QAction *act = new QAction(menu);
                act->setSeparator(true);
                menu->addAction(act);
                if (mCapabilities & PimCommon::StorageServiceAbstract::RenameFileCapabilitity) {
                    menu->addAction(i18n("Rename File..."), this, SLOT(slotRenameFile()));
                }
                if (mCapabilities & PimCommon::StorageServiceAbstract::ShareLinkCapability) {
                    menu->addAction(i18n("Share File"), this, SLOT(slotShareFile()));
                }
                act = new QAction(menu);
                act->setSeparator(true);
                menu->addAction(act);
                if (mCapabilities & PimCommon::StorageServiceAbstract::DownloadFileCapability) {
                    menu->addAction(QIcon::fromTheme(QLatin1String("download")), i18n("Download File"), this, SIGNAL(downloadFile()));
                }
                act = new QAction(menu);
                act->setSeparator(true);
                menu->addAction(act);
                if (mCapabilities & PimCommon::StorageServiceAbstract::DeleteFileCapability) {
                    menu->addAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18n("Delete File"), this, SLOT(slotDeleteFile()));
                }
            } else if (type == StorageServiceTreeWidget::Folder) {
                if (mCapabilities & PimCommon::StorageServiceAbstract::MoveFolderCapability) {
                    menu->addAction(QIcon::fromTheme(QLatin1String("edit-cut")), i18n("Cut"), this, SLOT(slotCutFolder()));
                }
                if (mCapabilities & PimCommon::StorageServiceAbstract::CopyFolderCapability) {
                    menu->addAction(QIcon::fromTheme(QLatin1String("edit-copy")), i18n("Copy"), this, SLOT(slotCopyFolder()));
                }
                QAction *act = new QAction(menu);
                act->setSeparator(true);
                menu->addAction(act);
                if (mCapabilities & PimCommon::StorageServiceAbstract::RenameFolderCapability) {
                    menu->addAction(i18n("Rename Folder..."), this, SLOT(slotRenameFolder()));
                }
                act = new QAction(menu);
                act->setSeparator(true);
                menu->addAction(act);
                if (mCapabilities & PimCommon::StorageServiceAbstract::DeleteFolderCapability) {
                    menu->addAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18n("Delete Folder"), this, SLOT(slotDeleteFolder()));
                }
            }
        }
        QAction *act = new QAction(menu);
        act->setSeparator(true);
        menu->addAction(act);
        if (mCapabilities & PimCommon::StorageServiceAbstract::UploadFileCapability) {
            menu->addAction(i18n("Upload File..."), this, SIGNAL(uploadFile()));
        }
        act = new QAction(menu);
        act->setSeparator(true);
        menu->addAction(act);
        if (mCapabilities & PimCommon::StorageServiceAbstract::CreateFolderCapability) {
            menu->addAction(QIcon::fromTheme(QLatin1String("folder-new")), i18n("Create Folder..."), this, SLOT(slotCreateFolder()));
        }

        act = new QAction(menu);
        act->setSeparator(true);
        menu->addAction(act);

        if (mCopyItem.moveItem) {
            if (mCopyItem.type == FileType) {
                if (mCapabilities & PimCommon::StorageServiceAbstract::MoveFileCapability) {
                    menu->addAction(QIcon::fromTheme(QLatin1String("edit-paste")), i18n("Paste"), this, SLOT(slotMoveFile()));
                }
            } else if (mCopyItem.type == FolderType) {
                if (mCapabilities & PimCommon::StorageServiceAbstract::MoveFolderCapability) {
                    menu->addAction(QIcon::fromTheme(QLatin1String("edit-paste")), i18n("Paste"), this, SLOT(slotMoveFolder()));
                }
            }
        } else {
            if (mCopyItem.type == FileType) {
                if (mCapabilities & PimCommon::StorageServiceAbstract::CopyFileCapability) {
                    menu->addAction(QIcon::fromTheme(QLatin1String("edit-paste")), i18n("Paste"), this, SLOT(slotPasteFile()));
                }
            } else if (mCopyItem.type == FolderType) {
                if (mCapabilities & PimCommon::StorageServiceAbstract::CopyFolderCapability) {
                    menu->addAction(QIcon::fromTheme(QLatin1String("edit-paste")), i18n("Paste"), this, SLOT(slotPasteFolder()));
                }
            }
        }
        if ((type == StorageServiceTreeWidget::File) || (type == StorageServiceTreeWidget::Folder)) {
            act = new QAction(menu);
            act->setSeparator(true);
            menu->addAction(act);
            createPropertiesAction(menu);
        }
    } else {
        menu->addAction(QIcon::fromTheme(QLatin1String("view-refresh")), i18n("Refresh"), this, SLOT(refreshList()));
    }
}

void StorageServiceTreeWidget::slotMoveFolder()
{
    mStorageService->moveFolder(mCopyItem.identifier, mCurrentFolder);
}

void StorageServiceTreeWidget::slotMoveFile()
{
    mStorageService->moveFile(mCopyItem.identifier, mCurrentFolder);
}

void StorageServiceTreeWidget::slotPasteFolder()
{
    mStorageService->copyFolder(mCopyItem.identifier, mCurrentFolder);
}

void StorageServiceTreeWidget::slotPasteFile()
{
    mStorageService->copyFile(mCopyItem.identifier, mCurrentFolder);
}

void StorageServiceTreeWidget::renameItem()
{
    switch (itemTypeSelected()) {
    case StorageServiceTreeWidget::Folder:
        slotRenameFolder();
        break;
    case StorageServiceTreeWidget::File:
        slotRenameFile();
        break;
    default:
        break;
    }
}

void StorageServiceTreeWidget::slotRenameFolder()
{
    const QString oldFolderName = itemIdentifierSelected();
    const QString name = currentItem()->text(0);
    const QString folder = QInputDialog::getText(this, i18n("Rename Folder Name"), i18n("Folder:"), QLineEdit::Normal, name);
    if (!folder.isEmpty()) {
        if (name != folder) {
            if (!checkName(folder)) {
                return;
            }
            mStorageService->renameFolder(oldFolderName, folder);
        }
    }
}

void StorageServiceTreeWidget::slotRenameFile()
{
    const QString oldFileName = itemIdentifierSelected();
    const QString name = currentItem()->text(0);
    const QString filename = QInputDialog::getText(this, i18n("Rename Filename"), i18n("Filename:"), QLineEdit::Normal, name);
    if (!filename.isEmpty()) {
        if (name != filename) {
            if (!checkName(filename)) {
                return;
            }
            mStorageService->renameFile(oldFileName, filename);
        }
    }
}

bool StorageServiceTreeWidget::checkName(const QString &name)
{
    const QRegExp disallowedSymbols = mStorageService->disallowedSymbols();
    if (!disallowedSymbols.isEmpty()) {
        if (name.contains(disallowedSymbols)) {
            KMessageBox::error(this, i18n("The following characters aren't allowed by %1:\n%2", mStorageService->storageServiceName(), mStorageService->disallowedSymbolsStr()), i18n("Create folder"));
            return false;
        }
    }
    if (name == QLatin1String(".") || name == QLatin1String("..")) {
        KMessageBox::error(this, i18n("You cannot name a folder or file . or .."), i18n("Create Folder"));
        return false;
    }
    return true;
}

void StorageServiceTreeWidget::slotCreateFolder()
{
    const QString folder = QInputDialog::getText(this, i18n("Folder Name"), i18n("Folder:"));
    if (!folder.isEmpty()) {
        if (!checkName(folder)) {
            return;
        }
        qDebug() << " mCurrentFolder" << mCurrentFolder;
        mStorageService->createFolder(folder, mCurrentFolder);
    }
}

void StorageServiceTreeWidget::deleteItem()
{
    switch (itemTypeSelected()) {
    case StorageServiceTreeWidget::Folder:
        deleteFolder();
        break;
    case StorageServiceTreeWidget::File:
        deleteFile();
        break;
    default:
        break;
    }
}

void StorageServiceTreeWidget::deleteFolder()
{
    const QString folder = itemIdentifierSelected();
    const QString name = currentItem()->text(0);
    if (KMessageBox::Yes == KMessageBox::questionYesNo(this, i18n("Are you sure that you want to delete \"%1\"?", name))) {
        if (!folder.isEmpty()) {
            mStorageService->deleteFolder(folder);
        }
    }
}

void StorageServiceTreeWidget::deleteFile()
{
    const QString filename = itemIdentifierSelected();
    const QString name = currentItem()->text(0);
    if (KMessageBox::Yes == KMessageBox::questionYesNo(this, i18n("Are you sure that you want to delete \"%1\"?", name))) {
        if (!filename.isEmpty()) {
            mStorageService->deleteFile(filename);
        }
    }
}

void StorageServiceTreeWidget::slotDeleteFolder()
{
    if (itemTypeSelected() == StorageServiceTreeWidget::Folder) {
        deleteFolder();
    }
}

void StorageServiceTreeWidget::slotDeleteFile()
{
    if (itemTypeSelected() == StorageServiceTreeWidget::File) {
        deleteFile();
    }
}

void StorageServiceTreeWidget::slotShareFile()
{
    if (itemTypeSelected() == StorageServiceTreeWidget::File) {
        const QString filename = itemIdentifierSelected();
        if (!filename.isEmpty()) {
            const QString rootShareFile = mStorageService->fileShareRoot(itemInformationSelected());
            mStorageService->shareLink(rootShareFile, filename);
        }
    }
}

void StorageServiceTreeWidget::slotDownloadFile()
{
    if (itemTypeSelected() == StorageServiceTreeWidget::File) {
        const QString filename = currentItem()->text(0);
        if (!filename.isEmpty()) {
            QString destination = StorageServiceManagerGlobalConfig::self()->downloadDirectory();
            if (destination.isEmpty()) {
                destination = QFileDialog::getExistingDirectory(this, QString());
                if (destination.isEmpty()) {
                    return;
                }
            }
            QFileInfo fileInfo(destination + QLatin1Char('/') + filename);
            if (fileInfo.exists()) {
                if (KMessageBox::No == KMessageBox::questionYesNo(this, i18n("Filename already exists. Do you want to overwrite it?"), i18n("Overwrite file"))) {
                    return;
                }
            }
            const QString fileId = mStorageService->fileIdentifier(itemInformationSelected());
            PimCommon::StorageServiceProgressManager::self()->addProgress(mStorageService, PimCommon::StorageServiceProgressManager::DownLoad);
            mStorageService->downloadFile(filename, fileId, destination);
        }
    }
}

bool StorageServiceTreeWidget::uploadFileToService()
{
    const QString filename = QFileDialog::getOpenFileName(this, QString(), QString(), QLatin1String("*"));
    if (!filename.isEmpty()) {
        const QRegExp disallowedSymbols = mStorageService->disallowedSymbols();
        const qlonglong maximumLimit =  mStorageService->maximumUploadFileSize();
        qDebug() << " maximumLimit" << maximumLimit;
        QFileInfo info(filename);
        if (maximumLimit > 0 && (info.size() > maximumLimit)) {
            KMessageBox::error(this, i18n("File size (%1) is larger than limit (%2)", KFormat().formatByteSize(info.size(), 1), KFormat().formatByteSize(maximumLimit, 1)));
            return false;
        }
        if (filename == QLatin1String(".") || filename == QLatin1String("..")) {
            KMessageBox::error(this, i18n("You are trying to use unauthorized characters."));
            return false;
        }
        QString newName = info.fileName();
        if (!disallowedSymbols.isEmpty()) {
            if (newName.contains(disallowedSymbols)) {
                QPointer<PimCommon::StorageServiceCheckNameDialog> dlg = new PimCommon::StorageServiceCheckNameDialog(this);
                dlg->setOldName(newName);
                dlg->setDisallowedSymbols(disallowedSymbols);
                dlg->setDisallowedSymbolsStr(mStorageService->disallowedSymbolsStr());
                if (dlg->exec()) {
                    newName = dlg->newName();
                    delete dlg;
                } else {
                    delete dlg;
                    return false;
                }
            }
        }
        PimCommon::StorageServiceProgressManager::self()->addProgress(mStorageService, PimCommon::StorageServiceProgressManager::Upload);
        mStorageService->uploadFile(filename, newName, mCurrentFolder);
        return true;
    } else {
        return false;
    }
}

void StorageServiceTreeWidget::canDownloadFile()
{
    if (itemTypeSelected() == StorageServiceTreeWidget::File) {
        Q_EMIT downloadFile();
    } else {
        KMessageBox::error(this, i18n("Please select a file to download."), i18n("Download File"));
    }
}

void StorageServiceTreeWidget::paintEvent(QPaintEvent *event)
{
    if (mInitialized) {
        PimCommon::StorageServiceTreeWidget::paintEvent(event);
    } else {
        QPainter p(viewport());

        QFont font = p.font();
        font.setItalic(true);
        p.setFont(font);

        if (!mTextColor.isValid()) {
            slotGeneralPaletteChanged();
        }
        p.setPen(mTextColor);
        p.drawText(QRect(0, 0, width(), height()), Qt::AlignCenter, i18n("Storage service not initialized."));
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

void StorageServiceTreeWidget::slotFileDoubleClicked()
{
    if (mCapabilities & PimCommon::StorageServiceAbstract::DownloadFileCapability) {
        if (KMessageBox::Yes == KMessageBox::questionYesNo(this, i18n("Do you want to download this file?"), i18n("Download File"))) {
            Q_EMIT downloadFile();
        }
    }
}

bool StorageServiceTreeWidget::listFolderWasLoaded() const
{
    return mInitialized;
}

void StorageServiceTreeWidget::logout()
{
    mInitialized = false;
    clear();
    mStorageService->logout();
}
