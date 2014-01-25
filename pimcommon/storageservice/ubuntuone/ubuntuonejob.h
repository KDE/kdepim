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


#ifndef UBUNTUONEJOB_H
#define UBUNTUONEJOB_H

#include "storageservice/job/storageserviceabstractjob.h"
#include "storageservice/storageserviceabstract.h"
#include <QNetworkReply>
class QAuthenticator;
namespace PimCommon {
class UbuntuOneJob : public PimCommon::StorageServiceAbstractJob
{
    Q_OBJECT
public:
    explicit UbuntuOneJob(QObject *parent=0);
    ~UbuntuOneJob();

    void requestTokenAccess();
    QNetworkReply *uploadFile(const QString &filename, const QString &destination);
    void listFolder(const QString &folder = QString());
    void accountInfo();
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

    void initializeToken(const QString &customerSecret, const QString &token, const QString &customerKey, const QString &tokenSecret);

private Q_SLOTS:
    void slotSendDataFinished(QNetworkReply *reply);
    void slotAuthenticationRequired(QNetworkReply*,QAuthenticator*);

Q_SIGNALS:
    void authorizationDone(const QString &customerSecret, const QString &token, const QString &customerKey, const QString &tokenSecret);

private:
    void parseRequestToken(const QString &data);
    void finishGetToken();
    void parseAccountInfo(const QString &data);
    void parseListFolder(const QString &data);
    void parseCreateFolder(const QString &data);
    void parseUploadFile(const QString &data);
    void parseAccessToken(const QString &data);
    void parseCreateServiceFolder(const QString &data);
    void parseDeleteFolder(const QString &data);
    void parseRenameFile(const QString &data);
    void parseRenameFolder(const QString &data);
    void parseCopyFolder(const QString &data);
    void parseCopyFile(const QString &data);
    void parseMoveFolder(const QString &data);
    void parseMoveFile(const QString &data);
    void parseDeleteFile(const QString &data);
    void parseDownloadFile(const QString &data);
    void parseShareLink(const QString &data);
    QString mCustomerSecret;
    QString mToken;
    QString mCustomerKey;
    QString mTokenSecret;
    QString mOauthVersion;
    QString mOauthSignatureMethod;
    QString mNonce;
    QString mTimestamp;
    QString mAttachmentVolume;
};
}

#endif // UBUNTUONEJOB_H
