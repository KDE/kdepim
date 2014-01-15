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
#include "storageserviceprogressindicator.h"
#include "storageservicewarning.h"
#include "storageserviceaccountinfodialog.h"
#include "pimcommon/storageservice/storageserviceabstract.h"

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
      mStorageService(storageService),
      mProgressBar(0)
{
    mProgressIndicator = new StorageServiceProgressIndicator(this);
    connect(mProgressIndicator, SIGNAL(updatePixmap(QPixmap)), this, SLOT(slotUpdatePixmap(QPixmap)));
    QVBoxLayout *vbox = new QVBoxLayout;
    setLayout(vbox);
    mListWidget = new StorageServiceTreeWidget(mStorageService);
    connect(mListWidget, SIGNAL(goToFolder(QString)), this, SLOT(slotGoToFolder(QString)));
    vbox->addWidget(mListWidget);

    if (mStorageService->hasProgressIndicatorSupport()) {
        mProgressBar = new QProgressBar;
        mProgressBar->hide();
        vbox->addWidget(mProgressBar);
    }
    mStorageServiceWarning = new StorageServiceWarning;
    vbox->addWidget(mStorageServiceWarning);
    connectStorageService();
}

StorageServicePage::~StorageServicePage()
{

}

void StorageServicePage::slotUpdatePixmap(const QPixmap &pix)
{
    Q_EMIT updatePixmap(pix, this);
}

void StorageServicePage::connectStorageService()
{
    connect(mStorageService, SIGNAL(uploadFileDone(QString,QString)), this, SLOT(slotUploadFileDone(QString,QString)));
    connect(mStorageService, SIGNAL(uploadFileProgress(QString,qint64,qint64)), this, SLOT(slotUploadFileProgress(QString,qint64,qint64)));
    connect(mStorageService, SIGNAL(shareLinkDone(QString,QString)), this, SLOT(slotShareLinkDone(QString,QString)));
    connect(mStorageService, SIGNAL(authenticationDone(QString)), this, SLOT(slotAuthenticationDone(QString)));
    connect(mStorageService, SIGNAL(authenticationFailed(QString,QString)), this, SLOT(slotAuthenticationFailed(QString,QString)));
    connect(mStorageService, SIGNAL(actionFailed(QString,QString)), this, SLOT(slotActionFailed(QString,QString)));
    connect(mStorageService, SIGNAL(accountInfoDone(QString,PimCommon::AccountInfo)), this, SLOT(slotAccountInfoDone(QString,PimCommon::AccountInfo)));
    connect(mStorageService, SIGNAL(inProgress(bool)), this, SLOT(slotProgressStateChanged(bool)));
    connect(mStorageService, SIGNAL(listFolderDone(QString,QString)), this, SLOT(slotListFolderDone(QString,QString)));
    connect(mStorageService, SIGNAL(createFolderDone(QString,QString)), this, SLOT(slotCreateFolderDone(QString, QString)));
    connect(mStorageService, SIGNAL(deleteFolderDone(QString,QString)), this, SLOT(slotDeleteFolderDone(QString,QString)));
    connect(mStorageService, SIGNAL(deleteFileDone(QString,QString)), this, SLOT(slotDeleteFileDone(QString,QString)));
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
        updateList(serviceName);
        KMessageBox::information(this, i18n("Upload File"), i18n("%1 was correctly uploaded", fileName));
    }
}

void StorageServicePage::slotUploadFileProgress(const QString &serviceName, qint64 done, qint64 total)
{
    if (verifyService(serviceName)) {
        //TODO
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
    mListWidget->slotCreateFolder();
}

void StorageServicePage::refreshList()
{
    mStorageService->listFolder(mCurrentFolder);
}

void StorageServicePage::accountInfo()
{
    mStorageService->accountInfo();
}

void StorageServicePage::uploadFile()
{
    mListWidget->slotUploadFile();
}

void StorageServicePage::deleteFile()
{
    mListWidget->slotDeleteFile();
}

void StorageServicePage::downloadFile()
{
    mListWidget->slotDownloadFile();
}

PimCommon::StorageServiceAbstract::Capabilities StorageServicePage::capabilities() const
{
    return mStorageService->capabilities();
}

void StorageServicePage::slotProgressStateChanged(bool state)
{
    mListWidget->setEnabled(!state);
    if (state) {
        mProgressIndicator->startAnimation();
    } else {
        mProgressIndicator->stopAnimation();
    }
}

void StorageServicePage::slotListFolderDone(const QString &serviceName, const QString &data)
{
    if (verifyService(serviceName)) {
        mStorageService->fillListWidget(mListWidget, data);
    }
}

void StorageServicePage::slotCreateFolderDone(const QString &serviceName, const QString &folder)
{
    Q_UNUSED(folder);
    updateList(serviceName);
    Q_EMIT updateStatusBarMessage(i18n("%1: Folder %2 was created.", serviceName, folder));
}

void StorageServicePage::slotDeleteFolderDone(const QString &serviceName, const QString &folder)
{
    Q_UNUSED(folder);
    updateList(serviceName);
    Q_EMIT updateStatusBarMessage(i18n("%1: Folder %2 was deleted.", serviceName, folder));
}

void StorageServicePage::slotDeleteFileDone(const QString &serviceName, const QString &filename)
{
    Q_UNUSED(filename);
    updateList(serviceName);
}

void StorageServicePage::slotGoToFolder(const QString &folder)
{
    //TODO verify it when we go up.
    mCurrentFolder = folder;
    QTimer::singleShot(0, this, SLOT(refreshList()));
}

void StorageServicePage::updateList(const QString &serviceName)
{
    if (verifyService(serviceName)) {
        QTimer::singleShot(0, this, SLOT(refreshList()));
    }
}
