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

#include "ubuntuonejob.h"
#include "pimcommon/storageservice/authdialog/logindialog.h"
#include "pimcommon/storageservice/utils/storageserviceutils.h"
#include "pimcommon/storageservice/storageservicejobconfig.h"

#include <KLocalizedString>

#include <qjson/parser.h>

#include <QNetworkAccessManager>
#include <QAuthenticator>
#include <QPointer>
#include <QDebug>
#include <QFile>

using namespace PimCommon;

UbuntuOneJob::UbuntuOneJob(QObject *parent)
    : PimCommon::StorageServiceAbstractJob(parent)
{
    mAttachmentVolume = PimCommon::StorageServiceJobConfig::self()->ubuntuOneAttachmentVolume();
    mOauthVersion = QLatin1String("1.0");
    mOauthSignatureMethod = QLatin1String("PLAINTEXT");
    mNonce = PimCommon::StorageServiceUtils::generateNonce(8);
    mTimestamp = QString::number(QDateTime::currentMSecsSinceEpoch()/1000);
    connect(mNetworkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotSendDataFinished(QNetworkReply*)));
    connect(mNetworkAccessManager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), SLOT(slotAuthenticationRequired(QNetworkReply*,QAuthenticator*)));
}

UbuntuOneJob::~UbuntuOneJob()
{

}

void UbuntuOneJob::copyFile(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::CopyFile;
    mError = false;
    qDebug()<<" not implemented";
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    //TODO
    deleteLater();
}

void UbuntuOneJob::copyFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::CopyFolder;
    mError = false;
    qDebug()<<" not implemented";
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    //TODO
    deleteLater();
}

void UbuntuOneJob::requestTokenAccess()
{
    mActionType = PimCommon::StorageServiceAbstract::RequestToken;
    mError = false;
    QUrl url(QLatin1String("https://login.ubuntu.com/api/1.0/authentications"));
    url.addQueryItem(QLatin1String("ws.op"), QLatin1String("authenticate"));
    url.addQueryItem(QLatin1String("token_name"), QLatin1String("Ubuntu One @ ") + PimCommon::StorageServiceJobConfig::self()->ubuntuOneTokenName() );
    qDebug()<<" postData "<<url;
    QNetworkRequest request(url);
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

QNetworkReply *UbuntuOneJob::uploadFile(const QString &filename, const QString &uploadAsName, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::UploadFile;
    mError = false;
    QFile *file = new QFile(filename);
    if (file->exists()) {
        mActionType = PimCommon::StorageServiceAbstract::UploadFile;
        mError = false;
        if (file->open(QIODevice::ReadOnly)) {
            //TODO
            delete file;
        } else {
            delete file;
        }
    }
    qDebug()<<" upload file not implemented";
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    deleteLater();
    return 0;
}

void UbuntuOneJob::listFolder(const QString &folder)
{
    mActionType = PimCommon::StorageServiceAbstract::ListFolder;
    mError = false;
    QUrl url(QString::fromLatin1("https://one.ubuntu.com/api/file_storage/v1/~/Ubuntu%20One/%1/?include_children=true").arg(folder));
    url.addQueryItem(QLatin1String("oauth_consumer_key"), mCustomerKey);
    url.addQueryItem(QLatin1String("oauth_nonce"), mNonce);

    const QString mAccessOauth = mCustomerSecret + QLatin1String("%26") + mTokenSecret;

    url.addQueryItem(QLatin1String("oauth_signature"), mAccessOauth);
    url.addQueryItem(QLatin1String("oauth_signature_method"), mOauthSignatureMethod);
    url.addQueryItem(QLatin1String("oauth_timestamp"), mTimestamp);
    url.addQueryItem(QLatin1String("oauth_version"), mOauthVersion);
    url.addQueryItem(QLatin1String("oauth_token"), mToken);
    QNetworkRequest request(url);
    qDebug()<<" url "<<url;
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));

    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void UbuntuOneJob::accountInfo()
{
    mActionType = PimCommon::StorageServiceAbstract::AccountInfo;
    mError = false;
    QUrl url(QLatin1String("https://one.ubuntu.com/api/quota/"));
    url.addQueryItem(QLatin1String("oauth_consumer_key"), mCustomerKey);
    url.addQueryItem(QLatin1String("oauth_nonce"), mNonce);

    const QString mAccessOauth = mCustomerSecret + QLatin1String("%26") + mTokenSecret;

    url.addQueryItem(QLatin1String("oauth_signature"), mAccessOauth);
    url.addQueryItem(QLatin1String("oauth_signature_method"), mOauthSignatureMethod);
    url.addQueryItem(QLatin1String("oauth_timestamp"), mTimestamp);
    url.addQueryItem(QLatin1String("oauth_version"), mOauthVersion);
    url.addQueryItem(QLatin1String("oauth_token"), mToken);
    QNetworkRequest request(url);
    qDebug()<<" url "<<url;
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));

    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void UbuntuOneJob::createFolder(const QString &foldername, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::CreateFolder;
    mError = false;
    if (foldername.isEmpty()) {
        qDebug()<<" foldername is empty";
    }
    QNetworkRequest request(QUrl(QString::fromLatin1("https://one.ubuntu.com/api/file_storage/v1/volumes/~/%1").arg(destination) + foldername));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    QUrl postData;
    const QString mAccessOauth = mCustomerSecret + QLatin1String("%26") + mTokenSecret;
/*
    postData.addQueryItem(QLatin1String("oauth_signature"), mAccessOauth);
    postData.addQueryItem(QLatin1String("oauth_signature_method"), mOauthSignatureMethod);
    postData.addQueryItem(QLatin1String("oauth_token"), mToken);

    postData.addQueryItem(QLatin1String("token_secret"), mTokenSecret);
    postData.addQueryItem(QLatin1String("consumer_secret"), mCustomerSecret);
    postData.addQueryItem(QLatin1String("consumer_key"), mCustomerKey);
    */
    postData.addQueryItem(QLatin1String("oauth_signature"), mAccessOauth);
    postData.addQueryItem(QLatin1String("oauth_signature_method"), mOauthSignatureMethod);
    postData.addQueryItem(QLatin1String("oauth_timestamp"), mTimestamp);
    postData.addQueryItem(QLatin1String("oauth_version"), mOauthVersion);
    postData.addQueryItem(QLatin1String("oauth_token"), mToken);

    QNetworkReply *reply = mNetworkAccessManager->put(request, postData.encodedQuery());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void UbuntuOneJob::shareLink(const QString &root, const QString &path)
{
    mActionType = PimCommon::StorageServiceAbstract::ShareLink;
    mError = false;
    qDebug()<<" not implemented";
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    deleteLater();
}

void UbuntuOneJob::initializeToken(const QString &customerSecret, const QString &token, const QString &customerKey, const QString &tokenSecret)
{
    mCustomerSecret = customerSecret;
    mToken = token;
    mCustomerKey = customerKey;
    mTokenSecret = tokenSecret;
}

void UbuntuOneJob::slotSendDataFinished(QNetworkReply *reply)
{
    const QString data = QString::fromUtf8(reply->readAll());
    reply->deleteLater();
    if (mError) {
        qDebug()<<" error type "<<data;

        const QString errorStr = data;
        switch(mActionType) {
        case PimCommon::StorageServiceAbstract::NoneAction:
            deleteLater();
            break;
        case PimCommon::StorageServiceAbstract::RequestToken:
            Q_EMIT authorizationFailed(errorStr);
            deleteLater();
            break;
        case PimCommon::StorageServiceAbstract::AccessToken:
            Q_EMIT authorizationFailed(errorStr);
            deleteLater();
            break;
        case PimCommon::StorageServiceAbstract::UploadFile:
            Q_EMIT uploadFileFailed(errorStr);
            errorMessage(mActionType, errorStr);
            deleteLater();
            break;
        case PimCommon::StorageServiceAbstract::DownLoadFile:
            Q_EMIT downLoadFileFailed(errorStr);
            errorMessage(mActionType, errorStr);
            deleteLater();
            break;
        case PimCommon::StorageServiceAbstract::CreateFolder:
        case PimCommon::StorageServiceAbstract::AccountInfo:
        case PimCommon::StorageServiceAbstract::ListFolder:
        case PimCommon::StorageServiceAbstract::CreateServiceFolder:
        case PimCommon::StorageServiceAbstract::DeleteFile:
        case PimCommon::StorageServiceAbstract::DeleteFolder:
        case PimCommon::StorageServiceAbstract::RenameFolder:
        case PimCommon::StorageServiceAbstract::RenameFile:
        case PimCommon::StorageServiceAbstract::MoveFolder:
        case PimCommon::StorageServiceAbstract::MoveFile:
        case PimCommon::StorageServiceAbstract::CopyFile:
        case PimCommon::StorageServiceAbstract::CopyFolder:
        case PimCommon::StorageServiceAbstract::ShareLink:
            errorMessage(mActionType, errorStr);
            deleteLater();
            break;
        }
        return;
    }
    switch(mActionType) {
    case PimCommon::StorageServiceAbstract::NoneAction:
        deleteLater();
        break;
    case PimCommon::StorageServiceAbstract::RequestToken:
        parseRequestToken(data);
        break;
    case PimCommon::StorageServiceAbstract::AccessToken:
        parseAccessToken(data);
        break;
    case PimCommon::StorageServiceAbstract::UploadFile:
        parseUploadFile(data);
        break;
    case PimCommon::StorageServiceAbstract::CreateFolder:
        parseCreateFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::AccountInfo:
        parseAccountInfo(data);
        break;
    case PimCommon::StorageServiceAbstract::ListFolder:
        parseListFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::CreateServiceFolder:
        parseCreateServiceFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::DeleteFile:
        parseDeleteFile(data);
        break;
    case PimCommon::StorageServiceAbstract::RenameFolder:
        parseRenameFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::RenameFile:
        parseRenameFile(data);
        break;
    case PimCommon::StorageServiceAbstract::DownLoadFile:
        parseDownloadFile(data);
        break;
    case PimCommon::StorageServiceAbstract::DeleteFolder:
        parseDeleteFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::MoveFolder:
        parseMoveFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::MoveFile:
        parseMoveFile(data);
        break;
    case PimCommon::StorageServiceAbstract::CopyFile:
        parseCopyFile(data);
        break;
    case PimCommon::StorageServiceAbstract::CopyFolder:
        parseCopyFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::ShareLink:
        parseShareLink(data);
        break;
    }
}

void UbuntuOneJob::parseShareLink(const QString &data)
{
    qDebug()<<" data"<<data;
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    deleteLater();
}

void UbuntuOneJob::parseCopyFolder(const QString &data)
{
    qDebug()<<" data"<<data;
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    deleteLater();
}

void UbuntuOneJob::parseCopyFile(const QString &data)
{
    qDebug()<<" data"<<data;
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    deleteLater();
}


void UbuntuOneJob::parseMoveFolder(const QString &data)
{
    qDebug()<<" data"<<data;
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    deleteLater();
}

void UbuntuOneJob::parseDeleteFolder(const QString &data)
{
    qDebug()<<" data"<<data;
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    deleteLater();
}

void UbuntuOneJob::parseMoveFile(const QString &data)
{
    qDebug()<<" data"<<data;
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    deleteLater();
}

void UbuntuOneJob::parseDeleteFile(const QString &data)
{
    qDebug()<<" data"<<data;
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    deleteLater();
}

void UbuntuOneJob::parseDownloadFile(const QString &data)
{
    qDebug()<<" data"<<data;
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    deleteLater();
}

void UbuntuOneJob::parseRenameFile(const QString &data)
{
    qDebug()<<" data"<<data;
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    deleteLater();
}

void UbuntuOneJob::parseRenameFolder(const QString &data)
{
    qDebug()<<" data"<<data;
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    deleteLater();
}

void UbuntuOneJob::parseCreateServiceFolder(const QString &data)
{
    qDebug()<<" create service folder not implemented";
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    deleteLater();
}

void UbuntuOneJob::parseListFolder(const QString &data)
{
    qDebug()<<" data "<<data;

    Q_EMIT listFolderDone(data);
    deleteLater();
}

void UbuntuOneJob::createServiceFolder()
{
    mActionType = PimCommon::StorageServiceAbstract::CreateServiceFolder;
    mError = false;
    QNetworkRequest request(QUrl(QLatin1String("https://one.ubuntu.com/api/file_storage/v1/volumes/") + mAttachmentVolume));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    QUrl postData;
    const QString mAccessOauth = mCustomerSecret + QLatin1String("%26") + mTokenSecret;

    postData.addQueryItem(QLatin1String("oauth_signature"), mAccessOauth);
    postData.addQueryItem(QLatin1String("token"), mToken);
    postData.addQueryItem(QLatin1String("token_secret"), mTokenSecret);
    postData.addQueryItem(QLatin1String("consumer_secret"), mCustomerSecret);
    postData.addQueryItem(QLatin1String("consumer_key"), mCustomerKey);
    QNetworkReply *reply = mNetworkAccessManager->put(request, postData.encodedQuery());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

QNetworkReply *UbuntuOneJob::downloadFile(const QString &name, const QString &fileId, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::DownLoadFile;
    mError = false;
#if 0
    QFile *file = new QFile(destination);
    if (file->exists()) {
        mActionType = PimCommon::StorageServiceAbstract::UploadFile;
        mError = false;
        if (file->open(QIODevice::WriteOnly)) {
            //TODO
            delete file;

        } else {
            delete file;
        }
    }
    qDebug()<<" upload file not implemented";
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    deleteLater();
    return 0;


#endif
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    deleteLater();
    return 0;
}

void UbuntuOneJob::deleteFile(const QString &filename)
{
    mActionType = PimCommon::StorageServiceAbstract::DeleteFile;
    mError = false;

    qDebug()<<" filename"<<filename;
    QNetworkRequest request(QUrl(QLatin1String("https://one.ubuntu.com/api/file_storage/v1/volumes/~/Ubuntu One/") + filename));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    QUrl postData;
    const QString mAccessOauth = mCustomerSecret + QLatin1String("%26") + mTokenSecret;

    postData.addQueryItem(QLatin1String("oauth_signature"), mAccessOauth);
    postData.addQueryItem(QLatin1String("token"), mToken);
    postData.addQueryItem(QLatin1String("token_secret"), mTokenSecret);
    postData.addQueryItem(QLatin1String("consumer_secret"), mCustomerSecret);
    postData.addQueryItem(QLatin1String("consumer_key"), mCustomerKey);
    QNetworkReply *reply = mNetworkAccessManager->deleteResource(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void UbuntuOneJob::deleteFolder(const QString &foldername)
{
    mActionType = PimCommon::StorageServiceAbstract::DeleteFolder;
    mError = false;
    QNetworkRequest request(QUrl(QLatin1String("https://one.ubuntu.com/api/file_storage/v1/volumes/~/Ubuntu One/") + foldername));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    QUrl postData;
    const QString mAccessOauth = mCustomerSecret + QLatin1String("%26") + mTokenSecret;

    postData.addQueryItem(QLatin1String("oauth_signature"), mAccessOauth);
    postData.addQueryItem(QLatin1String("token"), mToken);
    postData.addQueryItem(QLatin1String("token_secret"), mTokenSecret);
    postData.addQueryItem(QLatin1String("consumer_secret"), mCustomerSecret);
    postData.addQueryItem(QLatin1String("consumer_key"), mCustomerKey);
    QNetworkReply *reply = mNetworkAccessManager->deleteResource(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void UbuntuOneJob::renameFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::RenameFolder;
    mError = false;
    qDebug()<<" not implemented";
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    //TODO
    deleteLater();
}

void UbuntuOneJob::renameFile(const QString &oldName, const QString &newName)
{
    mActionType = PimCommon::StorageServiceAbstract::RenameFile;
    mError = false;
    QNetworkRequest request(QUrl(QLatin1String("https://one.ubuntu.com/api/file_storage/v1/volumes/~/Ubuntu One/") + oldName));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));

#if 0

    //https://one.ubuntu.com/api/file_storage/v1/~/Ubuntu One/ss?oauth_consumer_key=xze7Bw3&
                                                                 oauth_nonce=13922099193c5&
                                                                 oauth_signature=s4mmPGu164pA94R18SFfdqBW3As%3D&
                                                                 oauth_signature_method=HMAC-SHA1&
                                                                 oauth_timestamp=1392209919&
                                                                 oauth_token=CLmkMwIYhWeQQVxbKRgYhwDeWpURoYvbEjfpzgUvgamUrJdMyo&
                                                                 oauth_version=1.0

                                                                 oauth_signature=aKYAypiqhSkxUYRUUGNbqRYJHnSRsi%26FhpzokZIMhMYlVXDgUGMnBgXetcxMHzmCTbWIYpVfKkvpfnBQa&token=MPNifxdxiGzAkyOGOshegdKkNxomjTTWAZKZYoMPqNsOhEODdB&token_secret=FhpzokZIMhMYlVXDgUGMnBgXetcxMHzmCTbWIYpVfKkvpfnBQa&consumer_secret=aKYAypiqhSkxUYRUUGNbqRYJHnSRsi&consumer_key=xze7Bw3;
#endif
    QUrl postData;



    const QString mAccessOauth = mCustomerSecret + QLatin1String("%26") + mTokenSecret;

    postData.addQueryItem(QLatin1String("oauth_signature"), mAccessOauth);



    postData.addQueryItem(QLatin1String("oauth_token"), mToken);
    postData.addQueryItem(QLatin1String("oauth_token_secret"), mTokenSecret);
    //postData.addQueryItem(QLatin1String("oauth_consumer_secret"), mCustomerSecret);
    postData.addQueryItem(QLatin1String("oauth_consumer_key"), mCustomerKey);
    qDebug()<<" postData"<<postData;
    QUrl url = QUrl(QLatin1String("https://one.ubuntu.com/api/file_storage/v1/volumes/~/Ubuntu One/ss?oauth_consumer_key=xze7Bw3&oauth_nonce=13922099193c5&oauth_signature=s4mmPGu164pA94R18SFfdqBW3As%3D&oauth_signature_method=HMAC-SHA1&oauth_timestamp=1392209919&oauth_token=CLmkMwIYhWeQQVxbKRgYhwDeWpURoYvbEjfpzgUvgamUrJdMyo&oauth_version=1.0"));
    request = QNetworkRequest(url);
    QString t = QString::fromLatin1("{\"path\": \"%1\"}").arg(newName);
    QNetworkReply *reply = mNetworkAccessManager->put(request, t.toUtf8());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void UbuntuOneJob::moveFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::MoveFolder;
    mError = false;
    qDebug()<<" not implemented";
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    //TODO
    deleteLater();

}

void UbuntuOneJob::moveFile(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::MoveFile;
    mError = false;
    qDebug()<<" not implemented";
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    //TODO
    deleteLater();
}

void UbuntuOneJob::parseCreateFolder(const QString &data)
{
    //TODO
    qDebug()<<" data "<<data;
    Q_EMIT createFolderDone(QString());
    deleteLater();
}

void UbuntuOneJob::parseUploadFile(const QString &data)
{
    qDebug()<<" data "<<data;
    //TODO
    Q_EMIT uploadFileDone(QString());
    deleteLater();
}

void UbuntuOneJob::parseAccountInfo(const QString &data)
{
    QJson::Parser parser;
    bool ok;

    QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    PimCommon::AccountInfo accountInfo;
    if (info.contains(QLatin1String("used"))) {
       accountInfo.shared = info.value(QLatin1String("used")).toLongLong();
    }
    if (info.contains(QLatin1String("total"))) {
        accountInfo.accountSize = info.value(QLatin1String("total")).toLongLong();
    }
    Q_EMIT accountInfoDone(accountInfo);
    deleteLater();
}

void UbuntuOneJob::slotAuthenticationRequired(QNetworkReply *, QAuthenticator *auth)
{
    QPointer<LoginDialog> dlg = new LoginDialog;
    dlg->setCaption(i18n("UbuntuOne"));
    if (dlg->exec()) {
        auth->setUser(dlg->username());
        auth->setPassword(dlg->password());
    } else {
        Q_EMIT authorizationFailed(i18n("Authentication Canceled."));
        deleteLater();
    }
    delete dlg;
}

void UbuntuOneJob::parseRequestToken(const QString &data)
{
    QJson::Parser parser;
    bool ok;

    QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    qDebug()<<" info"<<info;
    if (info.contains(QLatin1String("consumer_secret"))) {
        mCustomerSecret = info.value(QLatin1String("consumer_secret")).toString();
    }
    if (info.contains(QLatin1String("token"))) {
        mToken = info.value(QLatin1String("token")).toString();
    }
    if (info.contains(QLatin1String("consumer_key"))) {
        mCustomerKey = info.value(QLatin1String("consumer_key")).toString();

    }
    if (info.contains(QLatin1String("token_secret"))) {
        mTokenSecret = info.value(QLatin1String("token_secret")).toString();
    }
    Q_EMIT authorizationDone(mCustomerSecret, mToken, mCustomerKey, mTokenSecret);
    finishGetToken();
}


void UbuntuOneJob::finishGetToken()
{
    mError = false;
    mActionType = PimCommon::StorageServiceAbstract::AccessToken;
    QUrl url(QUrl(QLatin1String("https://one.ubuntu.com/oauth/sso-finished-so-get-tokens/")));

    url.addQueryItem(QLatin1String("oauth_consumer_key"), mCustomerKey);
    url.addQueryItem(QLatin1String("oauth_nonce"), mNonce);

    const QString mAccessOauth = mCustomerSecret + QLatin1String("%26") + mTokenSecret;

    url.addQueryItem(QLatin1String("oauth_signature"), mAccessOauth);
    url.addQueryItem(QLatin1String("oauth_signature_method"), mOauthSignatureMethod);
    url.addQueryItem(QLatin1String("oauth_timestamp"), mTimestamp);
    url.addQueryItem(QLatin1String("oauth_version"), mOauthVersion);
    url.addQueryItem(QLatin1String("oauth_token"), mToken);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    //qDebug()<<" url "<<url;

    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void UbuntuOneJob::parseAccessToken(const QString &data)
{
    qDebug()<<" data "<<data;
    //Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    deleteLater();
}

#include "moc_ubuntuonejob.cpp"
