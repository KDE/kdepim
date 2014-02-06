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

#ifndef STORAGESERVICEABSTRACTJOB_H
#define STORAGESERVICEABSTRACTJOB_H

#include <QObject>
#include <QNetworkReply>
#include "storageservice/storageserviceabstract.h"

class QFile;
class QNetworkAccessManager;
namespace PimCommon {
class AccountInfo;
class StorageServiceAbstractJob : public QObject
{
    Q_OBJECT
public:
    explicit StorageServiceAbstractJob(QObject *parent = 0);
    ~StorageServiceAbstractJob();

    virtual QNetworkReply *uploadFile(const QString &filename, const QString &uploadAsName, const QString &destination=QString()) = 0;
    virtual QNetworkReply *downloadFile(const QString &name, const QString &fileId, const QString &destination) = 0;

    virtual void requestTokenAccess() = 0;
    virtual void listFolder(const QString &folder = QString()) = 0;
    virtual void accountInfo() = 0;
    virtual void createFolder(const QString &filename, const QString &destination) = 0;
    virtual void shareLink(const QString &root, const QString &path) = 0;
    virtual void createServiceFolder() = 0;
    virtual void deleteFile(const QString &filename) = 0;
    virtual void deleteFolder(const QString &foldername) = 0;
    virtual void renameFolder(const QString &source, const QString &destination) = 0;
    virtual void renameFile(const QString &oldName, const QString &newName) = 0;
    virtual void moveFolder(const QString &source, const QString &destination) = 0;
    virtual void moveFile(const QString &source, const QString &destination) = 0;
    virtual void copyFile(const QString &source, const QString &destination) = 0;
    virtual void copyFolder(const QString &source, const QString &destination) = 0;

Q_SIGNALS:
    void actionFailed(const QString &data);
    void shareLinkDone(const QString &url);
    void accountInfoDone(const PimCommon::AccountInfo &data);
    void uploadDownloadFileProgress(qint64 done, qint64 total);
    void createFolderDone(const QString &folderName);
    void uploadFileDone(const QString &fileName);
    void uploadFileFailed(const QString &fileName);
    void listFolderDone(const QVariant &listFolder);
    void authorizationFailed(const QString &error);
    void downLoadFileDone(const QString &filename);
    void downLoadFileFailed(const QString &filename);
    void deleteFileDone(const QString &filename);
    void deleteFolderDone(const QString &filename);
    void renameFolderDone(const QString &folder);
    void renameFileDone(const QString &folder);
    void moveFolderDone(const QString &folder);
    void moveFileDone(const QString &folder);
    void copyFileDone(const QString &folder);
    void copyFolderDone(const QString &folder);


protected:
    void errorMessage(PimCommon::StorageServiceAbstract::ActionType type, const QString &errorStr);

    QNetworkAccessManager *mNetworkAccessManager;
    PimCommon::StorageServiceAbstract::ActionType mActionType;
    bool mError;
    QString mErrorMsg;

protected Q_SLOTS:
    void slotDownloadReadyRead();
    void slotuploadDownloadFileProgress(qint64 done, qint64 total);

private slots:
    void slotSslErrors(QNetworkReply *reply, const QList<QSslError> &error);
    void slotError(QNetworkReply::NetworkError);

protected:
    QPointer<QFile> mDownloadFile;
};
}

#endif // STORAGESERVICEABSTRACTJOB_H
