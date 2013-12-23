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


#ifndef UBUNTUONEJOB_H
#define UBUNTUONEJOB_H

#include "storageservice/storageserviceabstractjob.h"
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
    void uploadFile(const QString &filename);
    void listFolder(const QString &folder = QString());
    void accountInfo();
    void createFolder(const QString &foldername);
    void shareLink(const QString &root, const QString &path);
    void createServiceFolder();
    void downloadFile(const QString &filename);
    void deleteFile(const QString &filename);
    void deleteFolder(const QString &foldername);


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
