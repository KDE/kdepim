/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#ifndef SERVICETESTWIDGET_H
#define SERVICETESTWIDGET_H

#include <QWidget>
#include "storageservice/storageserviceabstract.h"
class QTextEdit;

namespace PimCommon
{
class StorageServiceAbstract;
class AccountInfo;
}
class ServiceTestWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ServiceTestWidget(QWidget *parent);
    ~ServiceTestWidget();

    void setStorageService(PimCommon::StorageServiceAbstract *service);

private Q_SLOTS:
    void slotListFolder();

    void slotCreateFolder();
    void slotAccountInfo();

    void slotActionFailed(const QString &serviceName, const QString &error);
    void slotuploadDownloadFileProgress(const QString &serviceName, qint64 done, qint64 total);
    void slotShareLinkDone(const QString &serviceName, const QString &shareLink);
    void slotAuthenticationDone(const QString &serviceName);
    void slotAuthenticationFailed(const QString &serviceName, const QString &errorMessage);
    void slotCreateFolderDone(const QString &serviceName, const QString &folderName);
    void slotUploadFileDone(const QString &serviceName, const QString &fileName);
    void slotListFolderDone(const QString &serviceName, const QVariant &listFolder);
    void slotAccountInfoDone(const QString &serviceName, const PimCommon::AccountInfo &info);
    void slotDownloadFileDone(const QString &serviceName, const QString &filename);
    void slotUploadFile();
    void slotAuthentication();
    void slotCreateServiceFolder();
    void slotDeleteFile();
    void slotDeleteFolder();

    void slotDeleteFolderDone(const QString &serviceName, const QString &foldername);
    void slotDeleteFileDone(const QString &serviceName, const QString &filename);
    void slotDownloadFile();

private:
    void connectStorageService();
    void updateButtons(PimCommon::StorageServiceAbstract::Capabilities capabilities);
    PimCommon::StorageServiceAbstract *mStorageService;
    QTextEdit *mEdit;
    QAction *mAuthenticationAction;
    QAction *mListFolderAction;
    QAction *mCreateFolderAction;
    QAction *mAccountInfoAction;
    QAction *mUploadFileAction;
    QAction *mCreateServiceFolderAction;
    QAction *mDeleteFileAction;
    QAction *mDeleteFolderAction;
    QAction *mDownloadFileAction;
};

#endif // SERVICETESTWIDGET_H
