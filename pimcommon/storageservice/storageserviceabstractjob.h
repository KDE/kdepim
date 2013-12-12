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

#ifndef STORAGESERVICEABSTRACTJOB_H
#define STORAGESERVICEABSTRACTJOB_H

#include <QObject>
#include <QNetworkReply>

class QNetworkAccessManager;
namespace PimCommon {
class AccountInfo;
class StorageServiceAbstractJob : public QObject
{
    Q_OBJECT
public:
    explicit StorageServiceAbstractJob(QObject *parent = 0);
    ~StorageServiceAbstractJob();

    virtual void requestTokenAccess() = 0;
    virtual void uploadFile(const QString &filename) = 0;
    virtual void listFolder() = 0;
    virtual void accountInfo() = 0;
    virtual void createFolder(const QString &filename=QString()) = 0;
    virtual void shareLink(const QString &root, const QString &path) = 0;

protected Q_SLOTS:
    void slotError(QNetworkReply::NetworkError);

Q_SIGNALS:
    void actionFailed(const QString &data);
    void shareLinkDone(const QString &url);
    void accountInfoDone(const PimCommon::AccountInfo &data);
    void uploadFileProgress(qint64 done, qint64 total);
    void createFolderDone(const QString &folderName);
    void uploadFileDone(const QString &fileName);
    void listFolderDone(const QStringList &listFolder);
    void authorizationFailed(const QString &error);

protected:
    enum ActionType {
        NoneAction = 0,
        RequestToken,
        AccessToken,
        UploadFiles,
        CreateFolder,
        ListFolder,
        AccountInfo,
        ShareLink,
        CreateServiceFolder
    };
    void errorMessage(PimCommon::StorageServiceAbstractJob::ActionType type, const QString &errorStr);

    QNetworkAccessManager *mNetworkAccessManager;
    ActionType mActionType;
    bool mError;
};
}

#endif // STORAGESERVICEABSTRACTJOB_H
