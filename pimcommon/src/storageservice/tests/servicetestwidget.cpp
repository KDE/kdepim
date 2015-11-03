/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include "servicetestwidget.h"
#include "storageservice/storageserviceabstract.h"
#include "storageservice/tests/testsettingsjob.h"
#include "storageservice/storageservicejobconfig.h"
#include "storageservice/dialog/storageservicedownloaddialog.h"

#include <KLocalizedString>

#include <QVBoxLayout>
#include <QToolBar>
#include <QInputDialog>
#include <QTextEdit>
#include <QAction>
#include <QPointer>
#include <QDir>
#include <QFileDialog>

ServiceTestWidget::ServiceTestWidget(QWidget *parent)
    : QWidget(parent),
      mStorageService(Q_NULLPTR)
{
    PimCommon::TestSettingsJob *settingsJob = new PimCommon::TestSettingsJob;
    PimCommon::StorageServiceJobConfig *configJob = PimCommon::StorageServiceJobConfig::self();
    configJob->registerConfigIf(settingsJob);

    mEdit = new QTextEdit;
    mEdit->setReadOnly(true);
    QVBoxLayout *lay = new QVBoxLayout;
    QToolBar *bar = new QToolBar;
    lay->addWidget(bar);
    mAuthenticationAction = bar->addAction(QStringLiteral("Authentication..."), this, SLOT(slotAuthentication()));
    mListFolderAction = bar->addAction(QStringLiteral("List Folder..."), this, SLOT(slotListFolder()));
    mCreateFolderAction = bar->addAction(QStringLiteral("Create Folder..."), this, SLOT(slotCreateFolder()));
    mAccountInfoAction = bar->addAction(QStringLiteral("Account info..."), this, SLOT(slotAccountInfo()));
    mUploadFileAction = bar->addAction(QStringLiteral("Upload File..."), this, SLOT(slotUploadFile()));
    mCreateServiceFolderAction = bar->addAction(QStringLiteral("Create Service Folder..."), this, SLOT(slotCreateServiceFolder()));
    mDeleteFileAction = bar->addAction(QStringLiteral("Delete File..."), this, SLOT(slotDeleteFile()));
    mDeleteFolderAction = bar->addAction(QStringLiteral("Delete Folder..."), this, SLOT(slotDeleteFolder()));
    mDownloadFileAction = bar->addAction(QStringLiteral("Download File..."), this, SLOT(slotDownloadFile()));
    lay->addWidget(mEdit);
    setLayout(lay);
}

ServiceTestWidget::~ServiceTestWidget()
{

}

void ServiceTestWidget::slotDeleteFile()
{
    const QString filename = QInputDialog::getText(this, i18n("Filename"), i18n("Filename:"));
    if (!filename.isEmpty()) {
        mStorageService->deleteFile(filename);
    }
}

void ServiceTestWidget::slotDeleteFolder()
{
    const QString folder = QInputDialog::getText(this, i18n("Folder Name"), i18n("Folder:"));
    if (!folder.isEmpty()) {
        mStorageService->deleteFolder(folder);
    }
}

void ServiceTestWidget::setStorageService(PimCommon::StorageServiceAbstract *service)
{
    mStorageService = service;
    updateButtons(mStorageService->capabilities());
    connectStorageService();
}

void ServiceTestWidget::slotAuthentication()
{
    mStorageService->authentication();
}

void ServiceTestWidget::connectStorageService()
{
    connect(mStorageService, &PimCommon::StorageServiceAbstract::actionFailed, this, &ServiceTestWidget::slotActionFailed);
    connect(mStorageService, &PimCommon::StorageServiceAbstract::uploadDownloadFileProgress, this, &ServiceTestWidget::slotuploadDownloadFileProgress);
    connect(mStorageService, &PimCommon::StorageServiceAbstract::shareLinkDone, this, &ServiceTestWidget::slotShareLinkDone);
    connect(mStorageService, &PimCommon::StorageServiceAbstract::authenticationDone, this, &ServiceTestWidget::slotAuthenticationDone);
    connect(mStorageService, &PimCommon::StorageServiceAbstract::authenticationFailed, this, &ServiceTestWidget::slotAuthenticationFailed);
    connect(mStorageService, &PimCommon::StorageServiceAbstract::createFolderDone, this, &ServiceTestWidget::slotCreateFolderDone);
    connect(mStorageService, &PimCommon::StorageServiceAbstract::uploadFileDone, this, &ServiceTestWidget::slotUploadFileDone);
    connect(mStorageService, &PimCommon::StorageServiceAbstract::listFolderDone, this, &ServiceTestWidget::slotListFolderDone);
    connect(mStorageService, &PimCommon::StorageServiceAbstract::accountInfoDone, this, &ServiceTestWidget::slotAccountInfoDone);
    connect(mStorageService, &PimCommon::StorageServiceAbstract::deleteFileDone, this, &ServiceTestWidget::slotDeleteFileDone);
    connect(mStorageService, &PimCommon::StorageServiceAbstract::deleteFolderDone, this, &ServiceTestWidget::slotDeleteFolderDone);
    connect(mStorageService, &PimCommon::StorageServiceAbstract::downLoadFileDone, this, &ServiceTestWidget::slotDownloadFileDone);
}

void ServiceTestWidget::slotDeleteFolderDone(const QString &serviceName, const QString &foldername)
{
    mEdit->insertPlainText(serviceName + QStringLiteral(" Delete Folder: %1\n").arg(foldername));
}

void ServiceTestWidget::slotDeleteFileDone(const QString &serviceName, const QString &filename)
{
    mEdit->insertPlainText(serviceName + QStringLiteral(" Delete File: %1\n").arg(filename));
}

void ServiceTestWidget::slotActionFailed(const QString &serviceName, const QString &error)
{
    mEdit->insertPlainText(serviceName + QStringLiteral(" return an error: %1\n").arg(error));
}

void ServiceTestWidget::slotuploadDownloadFileProgress(const QString &serviceName, qint64 done, qint64 total)
{
    mEdit->insertPlainText(serviceName + QStringLiteral(" upload in progress: send:%1 total:%2\n").arg(done).arg(total));
}

void ServiceTestWidget::slotShareLinkDone(const QString &serviceName, const QString &shareLink)
{
    mEdit->insertPlainText(serviceName + QStringLiteral(" return a share link: %1\n").arg(shareLink));
}

void ServiceTestWidget::slotAuthenticationFailed(const QString &serviceName, const QString &errorMessage)
{
    mEdit->insertPlainText(serviceName + QStringLiteral(" Authentication failed: %1\n").arg(errorMessage));
}

void ServiceTestWidget::slotAuthenticationDone(const QString &serviceName)
{
    mEdit->insertPlainText(serviceName + QStringLiteral(" Authentication done\n"));
}

void ServiceTestWidget::slotCreateFolderDone(const QString &serviceName, const QString &folderName)
{
    mEdit->insertPlainText(serviceName + QStringLiteral(" Create new folder \"%1\" done\n").arg(folderName));
}

void ServiceTestWidget::slotUploadFileDone(const QString &serviceName, const QString &fileName)
{
    mEdit->insertPlainText(serviceName + QStringLiteral(" Upload file done %1\n").arg(fileName));
}

void ServiceTestWidget::slotListFolderDone(const QString &serviceName, const QVariant &listFolder)
{
    mEdit->insertPlainText(serviceName + QLatin1String(" list folder done \n")/*.arg(listFolder)*/);
}

void ServiceTestWidget::slotAccountInfoDone(const QString &serviceName, const PimCommon::AccountInfo &info)
{
    mEdit->insertPlainText(serviceName + QStringLiteral(" account Info: size: %1, quota: %2, shared: %3, displayName: %4\n")
                           .arg(info.accountSize).arg(info.quota).arg(info.shared).arg(info.displayName));
}

void ServiceTestWidget::slotDownloadFileDone(const QString &serviceName, const QString &filename)
{
    mEdit->insertPlainText(serviceName + QStringLiteral(" download file done %1\n").arg(filename));
}

void ServiceTestWidget::slotAccountInfo()
{
    mStorageService->accountInfo();
}

void ServiceTestWidget::slotCreateFolder()
{
    const QString folder = QInputDialog::getText(this, i18n("Folder Name"), i18n("Folder:"));
    if (!folder.isEmpty()) {
        mStorageService->createFolder(folder, QString());
    }
}

void ServiceTestWidget::slotListFolder()
{
    mStorageService->listFolder();
}

void ServiceTestWidget::slotUploadFile()
{
    const QString filename = QFileDialog::getOpenFileName(this);
    if (!filename.isEmpty()) {
        mStorageService->uploadFile(filename, QString(), QString()); //TODO
    }
}

void ServiceTestWidget::slotCreateServiceFolder()
{
    mStorageService->createServiceFolder();
}

void ServiceTestWidget::slotDownloadFile()
{
    const QString destination = QDir::homePath();

    QPointer<PimCommon::StorageServiceDownloadDialog> dlg = new PimCommon::StorageServiceDownloadDialog(mStorageService, this);
    dlg->setDefaultDownloadPath(destination);
    if (dlg->exec()) {
        //TODO
    }
    delete dlg;
}

void ServiceTestWidget::updateButtons(PimCommon::StorageServiceAbstract::Capabilities capabilities)
{
    mListFolderAction->setEnabled(capabilities & PimCommon::StorageServiceAbstract::ListFolderCapability);
    mCreateFolderAction->setEnabled(capabilities & PimCommon::StorageServiceAbstract::CreateFolderCapability);
    mAccountInfoAction->setEnabled(capabilities & PimCommon::StorageServiceAbstract::AccountInfoCapability);
    mUploadFileAction->setEnabled(capabilities & PimCommon::StorageServiceAbstract::UploadFileCapability);
    mDeleteFileAction->setEnabled(capabilities & PimCommon::StorageServiceAbstract::DeleteFileCapability);
    mDeleteFolderAction->setEnabled(capabilities & PimCommon::StorageServiceAbstract::DeleteFolderCapability);
    mDownloadFileAction->setEnabled(capabilities & PimCommon::StorageServiceAbstract::DownloadFileCapability);
}
