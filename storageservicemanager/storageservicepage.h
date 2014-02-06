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

#ifndef STORAGESERVICEPAGE_H
#define STORAGESERVICEPAGE_H

#include <QWidget>
#include "pimcommon/storageservice/storageserviceabstract.h"

class QProgressBar;
namespace PimCommon {
class StorageServiceAbstract;
class StorageServiceProgressWidget;
class StorageServiceProgressIndicator;
}
class StorageServiceWarning;
class StorageServiceTreeWidget;
class StorageServiceNavigationBar;
class StorageServicePage : public QWidget
{
    Q_OBJECT
public:
    explicit StorageServicePage(const QString &serviceName, PimCommon::StorageServiceAbstract *storageService, QWidget *parent=0);
    ~StorageServicePage();

    void authenticate();
    void createFolder();

    void accountInfo();
    void uploadFile();
    void deleteFile();
    PimCommon::StorageServiceAbstract::Capabilities capabilities() const;
    QString serviceName() const;
    bool hasUploadDownloadProgress() const;
    void setNetworkIsDown(bool state);
    void refreshList();
    void showLog();
Q_SIGNALS:
    void updateIcon(const QIcon &pix, StorageServicePage *page);
    void updateStatusBarMessage(const QString &msg);

public Q_SLOTS:
    void slotUploadFile();
    void slotDownloadFile();

private Q_SLOTS:
    void slotAccountInfoDone(const QString &serviceName, const PimCommon::AccountInfo &accountInfo);
    void slotUploadFileDone(const QString &serviceName, const QString &fileName);
    void slotuploadDownloadFileProgress(const QString &serviceName, qint64 done, qint64 total);
    void slotShareLinkDone(const QString &serviceName, const QString &link);
    void slotAuthenticationFailed(const QString &serviceName, const QString &error);
    void slotAuthenticationDone(const QString &serviceName);
    void slotActionFailed(const QString &serviceName, const QString &error);
    void slotProgressStateChanged(bool state);
    void slotUpdatePixmap(const QPixmap &pix);
    void slotListFolderDone(const QString &serviceName, const QVariant &data);
    void slotCreateFolderDone(const QString &serviceName, const QString &folder);
    void slotDeleteFolderDone(const QString &serviceName, const QString &folder);
    void slotDeleteFileDone(const QString &serviceName, const QString &filename);
    void slotRenameFolderDone(const QString &serviceName, const QString &fileName);
    void slotRenameFileDone(const QString &serviceName, const QString &fileName);
    void slotMoveFileDone(const QString &serviceName, const QString &filename);
    void slotMoveFolderDone(const QString &serviceName, const QString &filename);
    void slotCopyFileDone(const QString &serviceName, const QString &filename);
    void slotCopyFolderDone(const QString &serviceName, const QString &filename);
    void slotDownloadFileDone(const QString &serviceName, const QString &filename);
    void slotUploadFileFailed(const QString &serviceName, const QString &filename);
    void slotDownloadFileFailed(const QString &serviceName, const QString &filename);
    void slotGoHome();
    void slotGoToFolder(const QString &folder);

private:
    bool verifyService(const QString &serviceName);
    inline void updateList(const QString &serviceName);
    void connectStorageService();
    QString mServiceName;
    PimCommon::StorageServiceAbstract *mStorageService;
    StorageServiceTreeWidget *mTreeWidget;
    PimCommon::StorageServiceProgressIndicator *mProgressIndicator;
    StorageServiceWarning *mStorageServiceWarning;
    StorageServiceNavigationBar *mStorageServiceNavigationBar;
    PimCommon::StorageServiceProgressWidget *mProgressWidget;
};

#endif // STORAGESERVICEPAGE_H
