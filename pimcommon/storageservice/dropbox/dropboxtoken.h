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


#ifndef DROPBOXTOKEN_H
#define DROPBOXTOKEN_H
#include "pimcommon_export.h"
#include <QObject>
#include <QNetworkReply>
class QNetworkAccessManager;
namespace PimCommon {
class PIMCOMMON_EXPORT DropBoxToken : public QObject
{
    Q_OBJECT
public:
    explicit DropBoxToken(QObject *parent=0);
    ~DropBoxToken();

    void getTokenAccess();
    void requestTokenAccess();
    void uploadFile(const QString &filename);
    void listFolders();
    void accountInfo();
    void initializeToken(const QString &accessToken, const QString &accessTokenSecret, const QString &accessOauthSignature);

    void createFolder(const QString &filename=QString());

private Q_SLOTS:
    void slotError(QNetworkReply::NetworkError /*error*/);    
    void slotSendDataFinished(QNetworkReply *);
    void slotGetToken();

Q_SIGNALS:
    void authorizationDone(const QString &accessToken, const QString &accessTokenSecret, const QString &accessOauthSignature);
    void authorizationFailed();

private:
    void parseRequestToken(const QString &result);
    void doAuthentification();
    void parseResponseAccessToken(const QString &data);
    enum ActionType {
        NoneAction = 0,
        RequestToken,
        AccessToken,
        UploadFiles,
        CreateFolder,
        ListFolder,
        AccountInfo
    };

    QNetworkAccessManager *mNetworkAccessManager;
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
    ActionType mActionType;
    bool mError;
};
}

#endif // DROPBOXTOKEN_H
