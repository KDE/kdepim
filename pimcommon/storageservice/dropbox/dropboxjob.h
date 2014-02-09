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


#ifndef DROPBOXJOB_H
#define DROPBOXJOB_H
#include "storageservice/job/storageserviceabstractjob.h"
#include <QNetworkReply>
class QFile;
namespace PimCommon {
class AccountInfo;
class DropBoxJob : public PimCommon::StorageServiceAbstractJob
{
    Q_OBJECT
public:
    explicit DropBoxJob(QObject *parent=0);
    ~DropBoxJob();

    void requestTokenAccess();
    QNetworkReply *uploadFile(const QString &filename, const QString &uploadAsName, const QString &destination);
    void listFolder(const QString &folder = QString());
    void accountInfo();
    void initializeToken(const QString &accessToken, const QString &accessTokenSecret, const QString &accessOauthSignature);
    void createFolder(const QString &foldername, const QString &destination);
    void shareLink(const QString &root, const QString &path);
    void createServiceFolder();
    QNetworkReply *downloadFile(const QString &name, const QString &fileId, const QString &destination);
    void deleteFile(const QString &filename);
    void deleteFolder(const QString &foldername);
    void renameFolder(const QString &source, const QString &destination);
    void renameFile(const QString &oldName, const QString &newName);
    void moveFolder(const QString &source, const QString &destination);
    void moveFile(const QString &source, const QString &destination);
    void copyFile(const QString &source, const QString &destination);
    void copyFolder(const QString &source, const QString &destination);

private Q_SLOTS:
    void slotSendDataFinished(QNetworkReply *);

Q_SIGNALS:
    void authorizationDone(const QString &accessToken, const QString &accessTokenSecret, const QString &accessOauthSignature);

private:
    void addDefaultUrlItem(QUrl &url);
    void getTokenAccess();
    void parseRequestToken(const QString &result);
    void doAuthentication();
    void parseResponseAccessToken(const QString &data);
    void parseAccountInfo(const QString &data);
    void parseUploadFile(const QString &data);
    void parseCreateFolder(const QString &data);
    void parseShareLink(const QString &data);
    void parseListFolder(const QString &data);
    void parseDeleteFile(const QString &data);
    void parseDeleteFolder(const QString &data);
    void parseDownLoadFile(const QString &data);
    void parseRenameFolder(const QString &data);
    void parseRenameFile(const QString &data);
    void parseMoveFolder(const QString &data);
    void parseMoveFile(const QString &data);
    void parseCopyFile(const QString &data);
    void parseCopyFolder(const QString &data);
    QString nonce;
    QString mOauthconsumerKey;
    QString mOauthSignature;
    QString mOauthVersion;
    QString mOauthSignatureMethod;
    QString mTimestamp;
    QString mNonce;
    QString mOauthTokenSecret;
    QString mAccessOauthSignature;
    QString mOauthToken;
    QString mRootPath;
    QString mApiPath;
};
}

#endif // DROPBOXJOB_H
