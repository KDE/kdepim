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

#include <KLocale>
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
    bar->addAction(QLatin1String("Authentification..."), this, SLOT(slotAuthentification()));
    bar->addAction(QLatin1String("List Folder..."), this, SLOT(slotListFolder()));
    bar->addAction(QLatin1String("Create Folder..."), this, SLOT(slotCreateFolder()));
    bar->addAction(QLatin1String("Account info..."), this, SLOT(slotAccountInfo()));
    bar->addAction(QLatin1String("Upload File..."), this, SLOT(slotUploadFile()));
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

void ServiceTestWidget::slotAuthentification()
{
    mStorageService->authentification();
}

void ServiceTestWidget::connectStorageService()
{
    connect(mStorageService, SIGNAL(actionFailed(QString,QString)), this, SLOT(slotActionFailed(QString, QString)));
    connect(mStorageService, SIGNAL(uploadFileProgress(QString,qint64,qint64)), this, SLOT(slotUploadFileProgress(QString,qint64,qint64)));
    connect(mStorageService, SIGNAL(shareLinkDone(QString,QString)), this, SLOT(slotShareLinkDone(QString,QString)));
    connect(mStorageService, SIGNAL(authentificationDone(QString)), this, SLOT(slotAuthentificationDone(QString)));
    connect(mStorageService, SIGNAL(authentificationFailed(QString)), this, SLOT(slotAuthentificationFailed(QString)));
    connect(mStorageService, SIGNAL(createFolderDone(QString)), this, SLOT(slotCreateFolderDone(QString)));
    connect(mStorageService, SIGNAL(uploadFileDone(QString)), this, SLOT(slotUploadFileDone(QString)));
    connect(mStorageService, SIGNAL(listFolderDone(QString)), this, SLOT(slotListFolderDone(QString)));
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

void ServiceTestWidget::slotAuthentificationFailed(const QString &serviceName)
{
    mEdit->insertPlainText(serviceName + QLatin1String(" Authentification failed\n"));
}

void ServiceTestWidget::slotAuthentificationDone(const QString &serviceName)
{
    mEdit->insertPlainText(serviceName + QLatin1String(" Authentification done\n"));
}

void ServiceTestWidget::slotCreateFolderDone(const QString &serviceName)
{
    mEdit->insertPlainText(serviceName + QLatin1String(" Create folder done done\n"));
}

void ServiceTestWidget::slotUploadFileDone(const QString &serviceName)
{
    mEdit->insertPlainText(serviceName + QLatin1String(" Upload file done\n"));
}

void ServiceTestWidget::slotListFolderDone(const QString &serviceName)
{
    mEdit->insertPlainText(serviceName + QLatin1String(" list folder done\n"));
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

