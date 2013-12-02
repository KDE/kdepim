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
    void listFolder();
    void accountInfo();
    void initializeToken(const QString &accessToken, const QString &accessTokenSecret, const QString &accessOauthSignature);
    void createFolder(const QString &filename=QString());

private Q_SLOTS:
    void slotError(QNetworkReply::NetworkError /*error*/);    
    void slotSendDataFinished(QNetworkReply *);    
    void slotUploadFileProgress(qint64 done, qint64 total);

Q_SIGNALS:
    void authorizationDone(const QString &accessToken, const QString &accessTokenSecret, const QString &accessOauthSignature);

    void createFolderDone();
    void uploadFileDone();
    void listFolderDone();

    void accountInfoDone(const PimCommon::AccountInfo &data);

    void actionFailed(const QString &data);
    void authorizationFailed();

private:
    void getTokenAccess();
    void parseRequestToken(const QString &result);
    void doAuthentification();
    void parseResponseAccessToken(const QString &data);
    void parseAccountInfo(const QString &data);
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
    bool mInitialized;
};
}

#endif // DROPBOXJOB_H
