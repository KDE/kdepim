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
#include "storageservicelistwidget.h"
#include "pimcommon/storageservice/storageserviceabstract.h"

#include <KLocalizedString>
#include <KFileDialog>
#include <KInputDialog>

#include <QVBoxLayout>

StorageServicePage::StorageServicePage(PimCommon::StorageServiceAbstract *storageService, QWidget *parent)
    : QWidget(parent),
      mStorageService(storageService)
{
    QVBoxLayout *vbox = new QVBoxLayout;
    setLayout(vbox);
    mListWidget = new StorageServiceListWidget;
    vbox->addWidget(mListWidget);
    connectStorageService();
}

StorageServicePage::~StorageServicePage()
{

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
}

void StorageServicePage::slotAccountInfoDone(const QString &serviceName, const PimCommon::AccountInfo &accountInfo)
{

}

void StorageServicePage::slotUploadFileDone(const QString &serviceName, const QString &fileName)
{

}

void StorageServicePage::slotUploadFileProgress(const QString &serviceName, qint64 done, qint64 total)
{

}

void StorageServicePage::slotShareLinkDone(const QString &serviceName, const QString &link)
{

}

void StorageServicePage::slotAuthenticationFailed(const QString &serviceName, const QString &error)
{

}

void StorageServicePage::slotAuthenticationDone(const QString &serviceName)
{

}

void StorageServicePage::slotActionFailed(const QString &serviceName, const QString &error)
{

}

void StorageServicePage::authenticate()
{
    mStorageService->authentication();
}

void StorageServicePage::createFolder()
{
    const QString folder = KInputDialog::getText(i18n("Folder Name"), i18n("Folder:"));
    if (!folder.isEmpty()) {
        mStorageService->createFolder(folder);
    }
}

void StorageServicePage::refreshList()
{
    //TODO
}

void StorageServicePage::accountInfo()
{
    mStorageService->accountInfo();
}

void StorageServicePage::uploadFile()
{
    const QString filename = KFileDialog::getOpenFileName(KUrl(), QLatin1String("*"), this);
    if (!filename.isEmpty())
        mStorageService->uploadFile(filename);
}

void StorageServicePage::deleteFile()
{
    //TODO use selected element
    const QString filename = KInputDialog::getText(i18n("Delete File"), i18n("Filename:"));
    if (!filename.isEmpty()) {
        mStorageService->deleteFile(filename);
    }
}

void StorageServicePage::downloadFile()
{
    //TODO use selected element
    const QString filename = KInputDialog::getText(i18n("Download File"), i18n("Filename:"));
    if (!filename.isEmpty()) {
        mStorageService->downloadFile(filename);
    }
}

PimCommon::StorageServiceAbstract::Capabilities StorageServicePage::capabilities() const
{
    return mStorageService->capabilities();
}
