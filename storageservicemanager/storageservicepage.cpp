/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "storageservicepage.h"
#include "storageservicetreewidget.h"
#include "storageservicewarning.h"
#include "storageserviceaccountinfodialog.h"
#include "storageservicenavigationbar.h"
#include "pimcommon/storageservice/storageserviceabstract.h"
#include "pimcommon/storageservice/widgets/storageserviceprogresswidget.h"
#include "pimcommon/storageservice/widgets/storageserviceprogressindicator.h"


#include <KLocalizedString>
#include <KFileDialog>
#include <KInputDialog>
#include <KMessageBox>

#include <QApplication>
#include <QClipboard>
#include <QVBoxLayout>
#include <QDebug>
#include <QPointer>
#include <QTimer>
#include <QProgressBar>

StorageServicePage::StorageServicePage(const QString &serviceName, PimCommon::StorageServiceAbstract *storageService, QWidget *parent)
    : QWidget(parent),
      mServiceName(serviceName),
      mStorageService(storageService)
{
    QVBoxLayout *vbox = new QVBoxLayout;
    setLayout(vbox);

    mProgressIndicator = new PimCommon::StorageServiceProgressIndicator(this);
    connect(mProgressIndicator, SIGNAL(updatePixmap(QPixmap)), this, SLOT(slotUpdatePixmap(QPixmap)));
    mStorageServiceNavigationBar = new StorageServiceNavigationBar(this);
    connect(mStorageServiceNavigationBar, SIGNAL(goHome()), this, SLOT(slotGoHome()));
    connect(mStorageServiceNavigationBar, SIGNAL(goToFolder(QString)), this, SLOT(slotGoToFolder(QString)));
    mStorageServiceNavigationBar->setEnabled(false);
    vbox->addWidget(mStorageServiceNavigationBar);

    mTreeWidget = new StorageServiceTreeWidget(mStorageService);
    connect(mTreeWidget, SIGNAL(uploadFile()), this, SLOT(slotUploadFile()));
    connect(mTreeWidget, SIGNAL(downloadFile()), this, SLOT(slotDownloadFile()));
    vbox->addWidget(mTreeWidget);
    mProgressWidget = new PimCommon::StorageServiceProgressWidget(storageService);
    vbox->addWidget(mProgressWidget);
    mProgressWidget->hide();
    mStorageServiceWarning = new StorageServiceWarning;
    vbox->addWidget(mStorageServiceWarning);
    connectStorageService();
}

StorageServicePage::~StorageServicePage()
{

}

QString StorageServicePage::serviceName() const
{
    return mStorageService->storageServiceName();
}

bool StorageServicePage::hasUploadDownloadProgress() const
{
    return mStorageService->hasUploadOrDownloadInProgress();
}

void StorageServicePage::slotUpdatePixmap(const QPixmap &pix)
{
    if (pix.isNull()) {
        Q_EMIT updateIcon(mStorageService->icon(), this);
    } else {
        Q_EMIT updateIcon(QIcon(pix), this);
    }
}

void StorageServicePage::connectStorageService()
{
    connect(mStorageService, SIGNAL(shareLinkDone(QString,QString)), this, SLOT(slotShareLinkDone(QString,QString)));

    connect(mStorageService, SIGNAL(authenticationDone(QString)), this, SLOT(slotAuthenticationDone(QString)));
    connect(mStorageService, SIGNAL(authenticationFailed(QString,QString)), this, SLOT(slotAuthenticationFailed(QString,QString)));

    connect(mStorageService, SIGNAL(actionFailed(QString,QString)), this, SLOT(slotActionFailed(QString,QString)));

    connect(mStorageService, SIGNAL(accountInfoDone(QString,PimCommon::AccountInfo)), this, SLOT(slotAccountInfoDone(QString,PimCommon::AccountInfo)));

    connect(mStorageService, SIGNAL(inProgress(bool)), this, SLOT(slotProgressStateChanged(bool)));

    connect(mStorageService, SIGNAL(listFolderDone(QString,QString)), this, SLOT(slotListFolderDone(QString,QString)));

    connect(mStorageService, SIGNAL(createFolderDone(QString,QString)), this, SLOT(slotCreateFolderDone(QString,QString)));

    connect(mStorageService, SIGNAL(deleteFolderDone(QString,QString)), this, SLOT(slotDeleteFolderDone(QString,QString)));

    connect(mStorageService, SIGNAL(deleteFileDone(QString,QString)), this, SLOT(slotDeleteFileDone(QString,QString)));

    connect(mStorageService, SIGNAL(renameFolderDone(QString,QString)), this, SLOT(slotRenameFolderDone(QString,QString)));

    connect(mStorageService, SIGNAL(renameFileDone(QString,QString)), this, SLOT(slotRenameFileDone(QString,QString)));

    connect(mStorageService, SIGNAL(moveFileDone(QString,QString)), this, SLOT(slotMoveFileDone(QString,QString)));

    connect(mStorageService, SIGNAL(moveFolderDone(QString,QString)), this, SLOT(slotMoveFolderDone(QString,QString)));

    connect(mStorageService, SIGNAL(copyFolderDone(QString,QString)), this, SLOT(slotCopyFolderDone(QString,QString)));

    connect(mStorageService, SIGNAL(copyFileDone(QString,QString)), this, SLOT(slotCopyFileDone(QString,QString)));

    connect(mStorageService, SIGNAL(downLoadFileDone(QString,QString)), this, SLOT(slotDownloadFileDone(QString,QString)));
    connect(mStorageService, SIGNAL(downLoadFileFailed(QString,QString)), this, SLOT(slotDownloadFileFailed(QString,QString)));

    connect(mStorageService, SIGNAL(uploadFileFailed(QString,QString)), this, SLOT(slotUploadFileFailed(QString,QString)));
    connect(mStorageService, SIGNAL(uploadFileDone(QString,QString)), this, SLOT(slotUploadFileDone(QString,QString)));
    connect(mStorageService, SIGNAL(uploadDownloadFileProgress(QString,qint64,qint64)), this, SLOT(slotuploadDownloadFileProgress(QString,qint64,qint64)));
}

void StorageServicePage::slotRenameFolderDone(const QString &serviceName, const QString &folderName)
{
    if (folderName.isEmpty())
        Q_EMIT updateStatusBarMessage(i18n("%1: Folder was renamed.", serviceName));
    else
        Q_EMIT updateStatusBarMessage(i18n("%1: Folder was renamed to '%2'.", serviceName, folderName));
    updateList(serviceName);
}

void StorageServicePage::slotRenameFileDone(const QString &serviceName, const QString &fileName)
{
    Q_UNUSED(fileName);
    updateList(serviceName);
}

void StorageServicePage::slotAccountInfoDone(const QString &serviceName, const PimCommon::AccountInfo &accountInfo)
{
    if (verifyService(serviceName)) {
        QPointer<StorageServiceAccountInfoDialog> dlg = new StorageServiceAccountInfoDialog(accountInfo, this);
        dlg->exec();
        delete dlg;
    }
}

void StorageServicePage::slotUploadFileDone(const QString &serviceName, const QString &fileName)
{
    if (verifyService(serviceName)) {
        mProgressWidget->reset();
        mProgressWidget->hide();
        updateList(serviceName);
        KMessageBox::information(this, fileName.isEmpty() ? i18n("Filename was correctly uploaded") : i18n("%1 was correctly uploaded",fileName), i18n("Upload File"));
    }
}

void StorageServicePage::slotuploadDownloadFileProgress(const QString &serviceName, qint64 done, qint64 total)
{
    if (verifyService(serviceName)) {
        mProgressWidget->setProgressValue(done, total);
    }
}

void StorageServicePage::slotShareLinkDone(const QString &serviceName, const QString &link)
{
    if (verifyService(serviceName)) {
        QClipboard* const cb = QApplication::clipboard();
        cb->setText( link, QClipboard::Clipboard );
        KMessageBox::information(this, i18n("Link \'%1\' stored in clipboard",link),i18n("Shared Link"));
    }
}

void StorageServicePage::slotAuthenticationFailed(const QString &serviceName, const QString &error)
{
    if (verifyService(serviceName)) {
        mStorageServiceWarning->addLog(error);
        mStorageServiceWarning->animatedShow();
    }
}

void StorageServicePage::slotAuthenticationDone(const QString &serviceName)
{
    updateList(serviceName);
}

void StorageServicePage::slotActionFailed(const QString &serviceName, const QString &error)
{
    if (verifyService(serviceName)) {
        mStorageServiceWarning->addLog(error);
        mStorageServiceWarning->animatedShow();
    }
}

bool StorageServicePage::verifyService(const QString &serviceName)
{
    if (serviceName != mServiceName) {
        qDebug()<<" Error in signal/Slots";
        return false;
    }
    return true;
}

void StorageServicePage::authenticate()
{
    mStorageService->authentication();
}

void StorageServicePage::createFolder()
{
    mTreeWidget->slotCreateFolder();
}

void StorageServicePage::accountInfo()
{
    mStorageService->accountInfo();
}

void StorageServicePage::slotUploadFile()
{
    if (mTreeWidget->uploadFileToService()) {
        mProgressWidget->reset();
        mProgressWidget->setProgressBarType(PimCommon::StorageServiceProgressWidget::UploadBar);
        mProgressWidget->setBusyIndicator(false);
        mProgressWidget->show();
    }
}

void StorageServicePage::deleteFile()
{
    mTreeWidget->slotDeleteFile();
}

void StorageServicePage::slotDownloadFile()
{
    mProgressWidget->reset();
    mProgressWidget->setProgressBarType(PimCommon::StorageServiceProgressWidget::DownloadBar);
    mProgressWidget->setBusyIndicator(false);
    mProgressWidget->show();
    mTreeWidget->slotDownloadFile();
}

PimCommon::StorageServiceAbstract::Capabilities StorageServicePage::capabilities() const
{
    return mStorageService->capabilities();
}

void StorageServicePage::slotProgressStateChanged(bool state)
{
    mTreeWidget->setEnabled(!state);
    mStorageServiceNavigationBar->setEnabled(!state);
    if (state) {
        mProgressIndicator->startAnimation();
    } else {
        mProgressIndicator->stopAnimation();
    }
}

void StorageServicePage::slotListFolderDone(const QString &serviceName, const QString &data)
{
    if (verifyService(serviceName)) {
        mTreeWidget->setIsInitialized();
        mTreeWidget->slotListFolderDone(serviceName, data);
    }
}

void StorageServicePage::slotCreateFolderDone(const QString &serviceName, const QString &folder)
{
    updateList(serviceName);
    if (folder.isEmpty())
        Q_EMIT updateStatusBarMessage(i18n("%1: Folder was created.", serviceName));
    else
        Q_EMIT updateStatusBarMessage(i18n("%1: Folder %2 was created.", serviceName, folder));
}

void StorageServicePage::slotDeleteFolderDone(const QString &serviceName, const QString &folder)
{
    updateList(serviceName);
    if (folder.isEmpty())
        Q_EMIT updateStatusBarMessage(i18n("%1: Folder was deleted.", serviceName));
    else
        Q_EMIT updateStatusBarMessage(i18n("%1: Folder %2 was deleted.", serviceName, folder));
}

void StorageServicePage::slotDeleteFileDone(const QString &serviceName, const QString &filename)
{
    Q_UNUSED(filename);
    updateList(serviceName);
}

void StorageServicePage::slotMoveFileDone(const QString &serviceName, const QString &filename)
{
    Q_UNUSED(filename);
    updateList(serviceName);
}

void StorageServicePage::slotMoveFolderDone(const QString &serviceName, const QString &filename)
{
    Q_UNUSED(filename);
    updateList(serviceName);
}

void StorageServicePage::slotCopyFileDone(const QString &serviceName, const QString &filename)
{
    Q_UNUSED(filename);
    updateList(serviceName);
}

void StorageServicePage::slotCopyFolderDone(const QString &serviceName, const QString &filename)
{
    Q_UNUSED(filename);
    updateList(serviceName);
}

void StorageServicePage::slotDownloadFileDone(const QString &serviceName, const QString &filename)
{
    if (verifyService(serviceName)) {
        mProgressWidget->reset();
        mProgressWidget->hide();
        updateList(serviceName);
        QString msg;
        if (filename.isEmpty()) {
            msg = i18n("File was correctly downloaded");
        } else {
            msg = i18n("%1 was correctly downloaded", filename);
        }
        KMessageBox::information(this, msg, i18n("Download File"));
    }
}

void StorageServicePage::updateList(const QString &serviceName)
{
    if (verifyService(serviceName)) {
        refreshList();
    }
}

void StorageServicePage::refreshList()
{
    QTimer::singleShot(0, mTreeWidget, SLOT(refreshList()));
}

void StorageServicePage::slotDownloadFileFailed(const QString &serviceName, const QString &filename)
{
    if (verifyService(serviceName)) {
        mProgressWidget->hide();
    }
    KMessageBox::error(this, i18n("Download Failed"), i18n("Download"));
}

void StorageServicePage::slotUploadFileFailed(const QString &serviceName, const QString &filename)
{
    if (verifyService(serviceName)) {
        mProgressWidget->hide();
    }
    KMessageBox::error(this, i18n("Upload Failed"), i18n("Upload"));
}

void StorageServicePage::slotGoHome()
{
    mTreeWidget->goToFolder(QString());
}

void StorageServicePage::slotGoToFolder(const QString &folder)
{
    mTreeWidget->goToFolder(folder);
}
