/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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
#include "storageservice/storageserviceabstractjob.h"
#include <QNetworkReply>
namespace PimCommon {
class AccountInfo;
class DropBoxJob : public PimCommon::StorageServiceAbstractJob
{
    Q_OBJECT
public:
    explicit DropBoxJob(QObject *parent=0);
    ~DropBoxJob();

    void requestTokenAccess();
    void uploadFile(const QString &filename);
    void listFolder(const QString &folder = QString());
    void accountInfo();
    void initializeToken(const QString &accessToken, const QString &accessTokenSecret, const QString &accessOauthSignature);
    void createFolder(const QString &filename=QString());
    void shareLink(const QString &root, const QString &path);
    void createServiceFolder();
    void downloadFile(const QString &filename);
    void deleteFile(const QString &filename);
    void deleteFolder(const QString &foldername);

private Q_SLOTS:
    void slotSendDataFinished(QNetworkReply *);    
    void slotUploadFileProgress(qint64 done, qint64 total);

Q_SIGNALS:
    void authorizationDone(const QString &accessToken, const QString &accessTokenSecret, const QString &accessOauthSignature);

private:
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
