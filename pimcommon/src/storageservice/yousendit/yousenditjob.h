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

#ifndef YOUSENDITJOB_H
#define YOUSENDITJOB_H

#include <QObject>
#include "storageservice/job/storageserviceabstractjob.h"
class QNetworkReply;
namespace PimCommon
{
class YouSendItJob : public PimCommon::StorageServiceAbstractJob
{
    Q_OBJECT
public:
    explicit YouSendItJob(QObject *parent = Q_NULLPTR);
    ~YouSendItJob();

    void requestTokenAccess() Q_DECL_OVERRIDE;
    QNetworkReply *uploadFile(const QString &filename, const QString &uploadAsName, const QString &destination) Q_DECL_OVERRIDE;
    void listFolder(const QString &folder = QString()) Q_DECL_OVERRIDE;
    void accountInfo() Q_DECL_OVERRIDE;
    void createFolder(const QString &foldername, const QString &destination) Q_DECL_OVERRIDE;
    void shareLink(const QString &root, const QString &path) Q_DECL_OVERRIDE;
    void initializeToken(const QString &password, const QString &userName, const QString &token);
    void createServiceFolder() Q_DECL_OVERRIDE;
    QNetworkReply *downloadFile(const QString &name, const QString &fileId, const QString &destination) Q_DECL_OVERRIDE;
    void deleteFile(const QString &filename) Q_DECL_OVERRIDE;
    void deleteFolder(const QString &foldername) Q_DECL_OVERRIDE;
    void renameFolder(const QString &source, const QString &destination) Q_DECL_OVERRIDE;
    void renameFile(const QString &oldName, const QString &newName) Q_DECL_OVERRIDE;
    void moveFolder(const QString &source, const QString &destination) Q_DECL_OVERRIDE;
    void moveFile(const QString &source, const QString &destination) Q_DECL_OVERRIDE;
    void copyFile(const QString &source, const QString &destination) Q_DECL_OVERRIDE;
    void copyFolder(const QString &source, const QString &destination) Q_DECL_OVERRIDE;

Q_SIGNALS:
    void authorizationDone(const QString &password, const QString &username, const QString &token);

private Q_SLOTS:
    void slotSendDataFinished(QNetworkReply *reply);

private:
    void createFolderJob(const QString &foldername, const QString &destination);
    QNetworkRequest setDefaultHeader(const QUrl &url);
    void parseRenameFile(const QString &data);
    void parseMoveFolder(const QString &data);
    void parseMoveFile(const QString &data);
    void parseRenameFolder(const QString &data);
    void parseRequestToken(const QString &data);
    void parseAccountInfo(const QString &data);
    void parseCreateFolder(const QString &data);
    void parseUploadFile(const QString &data);
    void startUploadFile(const QString &fileId);
    void parseListFolder(const QString &data);
    void parseCreateServiceFolder(const QString &data);
    void parseDeleteFolder(const QString &data);
    void parseDeleteFile(const QString &data);
    void parseDownloadFile(const QString &data);
    void parseCopyFile(const QString &data);
    void parseCopyFolder(const QString &data);
    bool parseError(const QMap<QString, QVariant> &info);
    void parseShareLink(const QString &data);
    QString mPassword;
    QString mUsername;
    QString mDefaultUrl;
    QString mApiKey;
    QString mToken;
};
}

#endif // YOUSENDITJOB_H
