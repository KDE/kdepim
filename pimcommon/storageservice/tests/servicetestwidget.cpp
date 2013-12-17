/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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
#include "pimcommon/storageservice/storageserviceabstract.h"

#include <KLocalizedString>
#include <KFileDialog>

#include <QVBoxLayout>
#include <QToolBar>
#include <QInputDialog>
#include <QTextEdit>



ServiceTestWidget::ServiceTestWidget(QWidget *parent)
    : QWidget(parent),
      mStorageService(0)
{
    mEdit = new QTextEdit;
    mEdit->setReadOnly(true);
    QVBoxLayout *lay = new QVBoxLayout;
    QToolBar *bar = new QToolBar;
    lay->addWidget(bar);
    bar->addAction(QLatin1String("Authentication..."), this, SLOT(slotAuthentication()));
    bar->addAction(QLatin1String("List Folder..."), this, SLOT(slotListFolder()));
    bar->addAction(QLatin1String("Create Folder..."), this, SLOT(slotCreateFolder()));
    bar->addAction(QLatin1String("Account info..."), this, SLOT(slotAccountInfo()));
    bar->addAction(QLatin1String("Upload File..."), this, SLOT(slotUploadFile()));
    bar->addAction(QLatin1String("Create Service Folder..."), this, SLOT(slotCreateServiceFolder()));
    lay->addWidget(mEdit);
    setLayout(lay);
}

ServiceTestWidget::~ServiceTestWidget()
{

}

void ServiceTestWidget::setStorageService(PimCommon::StorageServiceAbstract *service)
{
    mStorageService = service;
    connectStorageService();
}

void ServiceTestWidget::slotAuthentication()
{
    mStorageService->authentication();
}

void ServiceTestWidget::connectStorageService()
{
    connect(mStorageService, SIGNAL(actionFailed(QString,QString)), this, SLOT(slotActionFailed(QString, QString)));
    connect(mStorageService, SIGNAL(uploadFileProgress(QString,qint64,qint64)), this, SLOT(slotUploadFileProgress(QString,qint64,qint64)));
    connect(mStorageService, SIGNAL(shareLinkDone(QString,QString)), this, SLOT(slotShareLinkDone(QString,QString)));
    connect(mStorageService, SIGNAL(authenticationDone(QString)), this, SLOT(slotAuthenticationDone(QString)));
    connect(mStorageService, SIGNAL(authenticationFailed(QString,QString)), this, SLOT(slotAuthenticationFailed(QString,QString)));
    connect(mStorageService, SIGNAL(createFolderDone(QString,QString)), this, SLOT(slotCreateFolderDone(QString,QString)));
    connect(mStorageService, SIGNAL(uploadFileDone(QString,QString)), this, SLOT(slotUploadFileDone(QString,QString)));
    connect(mStorageService, SIGNAL(listFolderDone(QString,QStringList)), this, SLOT(slotListFolderDone(QString,QStringList)));
    connect(mStorageService, SIGNAL(accountInfoDone(QString,PimCommon::AccountInfo)), this, SLOT(slotAccountInfoDone(QString,PimCommon::AccountInfo)));
}

void ServiceTestWidget::slotActionFailed(const QString &serviceName, const QString &error)
{
    mEdit->insertPlainText(serviceName + QString::fromLatin1(" return an error: %1\n").arg(error));
}

void ServiceTestWidget::slotUploadFileProgress(const QString &serviceName, qint64 done ,qint64 total)
{
    mEdit->insertPlainText(serviceName + QString::fromLatin1(" upload in progress: send:%1 total:%2\n").arg(done).arg(total));
}

void ServiceTestWidget::slotShareLinkDone(const QString &serviceName, const QString &shareLink)
{
    mEdit->insertPlainText(serviceName + QString::fromLatin1(" return a share link: %1\n").arg(shareLink));
}

void ServiceTestWidget::slotAuthenticationFailed(const QString &serviceName, const QString &errorMessage)
{
    mEdit->insertPlainText(serviceName + QString::fromLatin1(" Authentication failed: %1\n").arg(errorMessage));
}

void ServiceTestWidget::slotAuthenticationDone(const QString &serviceName)
{
    mEdit->insertPlainText(serviceName + QLatin1String(" Authentication done\n"));
}

void ServiceTestWidget::slotCreateFolderDone(const QString &serviceName, const QString &folderName)
{
    mEdit->insertPlainText(serviceName + QString::fromLatin1(" Create new folder \"%1\" done\n").arg(folderName));
}

void ServiceTestWidget::slotUploadFileDone(const QString &serviceName, const QString &fileName)
{
    mEdit->insertPlainText(serviceName + QString::fromLatin1(" Upload file done %1\n").arg(fileName));
}

void ServiceTestWidget::slotListFolderDone(const QString &serviceName, const QStringList &listFolder)
{
    mEdit->insertPlainText(serviceName + QString::fromLatin1(" list folder done %1\n").arg(listFolder.join(QLatin1String(";"))));
}

void ServiceTestWidget::slotAccountInfoDone(const QString &serviceName, const PimCommon::AccountInfo &info)
{
    mEdit->insertPlainText(serviceName + QString::fromLatin1(" account Info: size: %1, quota: %2, shared: %3, displayName: %4\n")
                           .arg(info.accountSize).arg(info.quota).arg(info.shared).arg(info.displayName));
}

void ServiceTestWidget::slotAccountInfo()
{
    mStorageService->accountInfo();
}

void ServiceTestWidget::slotCreateFolder()
{
    const QString folder = QInputDialog::getText(this,i18n("Folder Name"), i18n("Folder:"));
    if (!folder.isEmpty()) {
        mStorageService->createFolder(folder);
    }
}

void ServiceTestWidget::slotListFolder()
{
    mStorageService->listFolder();
}

void ServiceTestWidget::slotUploadFile()
{
    const QString filename = KFileDialog::getOpenFileName();
    if (!filename.isEmpty()) {
        mStorageService->uploadFile(filename);
    }
}

void ServiceTestWidget::slotCreateServiceFolder()
{
    //TODO
}
