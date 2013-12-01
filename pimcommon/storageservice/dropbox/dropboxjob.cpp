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


#include "dropboxjob.h"
#include "storageservice/storageauthviewdialog.h"
#include "storageservice/storageserviceabstract.h"

#include <KLocale>

#include <qjson/parser.h>
#include <QFile>
#include <QFileInfo>

#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QDateTime>
#include <QStringList>
#include <QDebug>
#include <QPointer>
#include <QFile>

using namespace PimCommon;

QString generateNonce(qint32 length)
{
    QString clng;

    for(int i=0; i<length; ++i) {
        clng += QString::number(int( qrand() / (RAND_MAX + 1.0) * (16 + 1 - 0) + 0 ), 16).toUpper();
    }

    return clng;
}

DropBoxJob::DropBoxJob(QObject *parent)
    : PimCommon::StorageServiceAbstractJob(parent),
      mInitialized(false),
      mError(false)
{
    mOauthconsumerKey = QLatin1String("e40dvomckrm48ci");
    mOauthSignature = QLatin1String("0icikya464lny9g&");
    mOauthVersion = QLatin1String("1.0");
    mOauthSignatureMethod = QLatin1String("PLAINTEXT");
    mTimestamp = QString::number(QDateTime::currentMSecsSinceEpoch()/1000);
    mNonce = generateNonce(8);
    connect(mNetworkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotSendDataFinished(QNetworkReply*)));
}

DropBoxJob::~DropBoxJob()
{

}

void DropBoxJob::initializeToken(const QString &accessToken, const QString &accessTokenSecret, const QString &accessOauthSignature)
{
    mOauthToken = accessToken;
    mOauthTokenSecret = accessTokenSecret;
    mAccessOauthSignature = accessOauthSignature;
    mInitialized = true;
}

void DropBoxJob::requestTokenAccess()
{
    mActionType = RequestToken;
    QNetworkRequest request(QUrl(QLatin1String("https://api.dropbox.com/1/oauth/request_token")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));

    QUrl postData;

    postData.addQueryItem(QLatin1String("oauth_consumer_key"), mOauthconsumerKey);
    postData.addQueryItem(QLatin1String("oauth_nonce"), mNonce);
    postData.addQueryItem(QLatin1String("oauth_signature"), mOauthSignature);
    postData.addQueryItem(QLatin1String("oauth_signature_method"),mOauthSignatureMethod);
    postData.addQueryItem(QLatin1String("oauth_timestamp"), mTimestamp);
    postData.addQueryItem(QLatin1String("oauth_version"), mOauthVersion);
    qDebug()<<" https://api.dropbox.com/1/oauth/request_token"<<postData;

    QNetworkReply *reply = mNetworkAccessManager->post(request, postData.encodedQuery());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void DropBoxJob::getTokenAccess()
{
    mActionType = AccessToken;
    mError = false;
    QNetworkRequest request(QUrl(QLatin1String("https://api.dropbox.com/1/oauth/access_token")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));

    QUrl postData;

    postData.addQueryItem(QLatin1String("oauth_consumer_key"), mOauthconsumerKey);
    qDebug()<<" mAccessOauthSignature"<<mAccessOauthSignature;
    postData.addQueryItem(QLatin1String("oauth_nonce"), mNonce);
    postData.addQueryItem(QLatin1String("oauth_signature"), mAccessOauthSignature);
    postData.addQueryItem(QLatin1String("oauth_signature_method"),mOauthSignatureMethod);
    postData.addQueryItem(QLatin1String("oauth_timestamp"), mTimestamp);
    postData.addQueryItem(QLatin1String("oauth_version"), mOauthVersion);
    postData.addQueryItem(QLatin1String("oauth_token"), mOauthToken);
    qDebug()<<"getTokenAccess  postdata"<<postData;

    QNetworkReply *reply = mNetworkAccessManager->post(request, postData.encodedQuery());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void DropBoxJob::slotError(QNetworkReply::NetworkError /*error*/)
{
    qDebug()<<" Error ";
    mError = true;
}

void DropBoxJob::slotSendDataFinished(QNetworkReply *reply)
{
    const QString data = QString::fromUtf8(reply->readAll());
    reply->deleteLater();
    if (mError) {
        qDebug()<<" error type "<<data;
        QJson::Parser parser;
        bool ok;

        QMap<QString, QVariant> error = parser.parse(data.toUtf8(), &ok).toMap();
        if (error.contains(QLatin1String("error"))) {
            const QString errorStr = error.value(QLatin1String("error")).toString();
            switch(mActionType) {
            case NoneAction:
                break;
            case RequestToken:
                break;
            case AccessToken:
                break;
            case UploadFiles:
                Q_EMIT actionFailed(i18n("Upload File returns an error: %1",errorStr));
                deleteLater();
                break;
            case CreateFolder:
                Q_EMIT actionFailed(i18n("Create Folder returns an error: %1",errorStr));
                deleteLater();
                break;
            case AccountInfo:
                Q_EMIT actionFailed(i18n("Get account info returns an error: %1",errorStr));
                deleteLater();
                break;
            case ListFolder:
                Q_EMIT actionFailed(i18n("List folder returns an error: %1",errorStr));
                deleteLater();
                break;
            default:
                qDebug()<<" Action Type unknown:"<<mActionType;
                deleteLater();
                break;
            }
        }
        return;
    }
    switch(mActionType) {
    case NoneAction:
        break;
    case RequestToken:
        parseRequestToken(data);
        break;
    case AccessToken:
        parseResponseAccessToken(data);
        break;
    case UploadFiles:
        Q_EMIT uploadFileDone();
        deleteLater();
        break;
    case CreateFolder:
        Q_EMIT createFolderDone();
        deleteLater();
        break;
    case AccountInfo:
        parseAccountInfo(data);
        break;
    case ListFolder:
        Q_EMIT listFolderDone();
        deleteLater();
        break;
    default:
        qDebug()<<" Action Type unknown:"<<mActionType;
    }
}

void DropBoxJob::parseAccountInfo(const QString &data)
{
    QJson::Parser parser;
    bool ok;

    QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    PimCommon::AccountInfo accountInfo;
    if (info.contains(QLatin1String("display_name")))
        accountInfo.displayName = info.value(QLatin1String("display_name")).toString();
    if (info.contains(QLatin1String("quota_info"))) {
        QMap<QString, QVariant> quotaInfo = info.value(QLatin1String("quota_info")).toMap();
        if (quotaInfo.contains(QLatin1String("quota"))) {
            accountInfo.quota = quotaInfo.value(QLatin1String("quota")).toLongLong();
        }
        if (quotaInfo.contains(QLatin1String("normal"))) {
            accountInfo.accountSize = quotaInfo.value(QLatin1String("normal")).toLongLong();
        }
        if (quotaInfo.contains(QLatin1String("shared"))) {
            accountInfo.shared = quotaInfo.value(QLatin1String("shared")).toLongLong();
        }
    }


    Q_EMIT accountInfoDone(accountInfo);
    deleteLater();
}

void DropBoxJob::parseResponseAccessToken(const QString &data)
{
    if(data.contains(QLatin1String("error"))) {
        qDebug()<<" return error !";
        Q_EMIT authorizationFailed();
    } else {
        QStringList split           = data.split(QLatin1Char('&'));
        QStringList tokenSecretList = split.at(0).split(QLatin1Char('='));
        mOauthTokenSecret          = tokenSecretList.at(1);
        QStringList tokenList       = split.at(1).split(QLatin1Char('='));
        mOauthToken = tokenList.at(1);
        mAccessOauthSignature = mOauthSignature + mOauthTokenSecret;

        Q_EMIT authorizationDone(mOauthToken, mOauthTokenSecret, mAccessOauthSignature);
    }
    deleteLater();
}

void DropBoxJob::parseRequestToken(const QString &result)
{
    const QStringList split = result.split(QLatin1Char('&'));
    if (split.count() == 2) {
        const QStringList tokenSecretList = split.at(0).split(QLatin1Char('='));
        mOauthTokenSecret = tokenSecretList.at(1);
        const QStringList tokenList = split.at(1).split(QLatin1Char('='));
        mOauthToken = tokenList.at(1);
        mAccessOauthSignature = mOauthSignature + mOauthTokenSecret;

        qDebug()<<" mOauthToken" <<mOauthToken<<"mAccessOauthSignature "<<mAccessOauthSignature<<" mOauthSignature"<<mOauthSignature;

    } else {
        qDebug()<<" data is not good: "<<result;
    }
    doAuthentification();
}

void DropBoxJob::doAuthentification()
{
    QUrl url(QLatin1String("https://api.dropbox.com/1/oauth/authorize"));
    url.addQueryItem(QLatin1String("oauth_token"), mOauthToken);
    qDebug()<<" void DropBoxJob::doAuthentification()"<<url;
    QPointer<StorageAuthViewDialog> dlg = new StorageAuthViewDialog;
    dlg->setUrl(url);
    if (dlg->exec()) {
        getTokenAccess();
    }
    delete dlg;
}

void DropBoxJob::createFolder(const QString &folder)
{
    mActionType = CreateFolder;
    mError = false;
    QUrl url(QLatin1String("https://api.dropbox.com/1/fileops/create_folder"));
    url.addQueryItem(QLatin1String("root"), QLatin1String("dropbox"));
    url.addQueryItem(QLatin1String("path"), folder );
    url.addQueryItem(QLatin1String("oauth_consumer_key"),mOauthconsumerKey );
    url.addQueryItem(QLatin1String("oauth_nonce"), mNonce);
    url.addQueryItem(QLatin1String("oauth_signature"), mAccessOauthSignature.replace(QLatin1Char('&'),QLatin1String("%26")));
    url.addQueryItem(QLatin1String("oauth_signature_method"), mOauthSignatureMethod);
    url.addQueryItem(QLatin1String("oauth_timestamp"), mTimestamp);
    url.addQueryItem(QLatin1String("oauth_version"), mOauthVersion);
    url.addQueryItem(QLatin1String("oauth_token"), mOauthToken);
    qDebug()<<" postData "<<url;
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void DropBoxJob::uploadFile(const QString &filename)
{
    QFile *file = new QFile(filename);
    if (file->exists()) {
        mActionType = UploadFiles;
        mError = false;
        //QNetworkRequest request(QUrl(QString::fromLatin1("https://api.dropbox.com/1/files_put/dropbox/test/%1").arg(file->fileName()))); //Add path (need to store default path
        QFileInfo info(filename);
        QUrl url(QString::fromLatin1("https://api.dropbox.com/1/files_put/dropbox/test/%1").arg(info.fileName()));
        url.addQueryItem(QLatin1String("oauth_consumer_key"),mOauthconsumerKey );
        url.addQueryItem(QLatin1String("oauth_nonce"), mNonce);
        url.addQueryItem(QLatin1String("oauth_signature"), mAccessOauthSignature.replace(QLatin1Char('&'),QLatin1String("%26")));
        url.addQueryItem(QLatin1String("oauth_signature_method"), mOauthSignatureMethod);
        url.addQueryItem(QLatin1String("oauth_timestamp"), mTimestamp);
        url.addQueryItem(QLatin1String("oauth_version"), mOauthVersion);
        url.addQueryItem(QLatin1String("oauth_token"), mOauthToken);
        url.addQueryItem(QLatin1String("overwrite"), QLatin1String("false"));


        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader,QLatin1String("application/octet-stream"));
        /*
        //request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
        QUrl postData;
        postData.addQueryItem(QLatin1String("oauth_consumer_key"),mOauthconsumerKey );
        postData.addQueryItem(QLatin1String("oauth_nonce"), mNonce);
        postData.addQueryItem(QLatin1String("oauth_signature"), mAccessOauthSignature.replace(QLatin1Char('&'),QLatin1String("%26")));
        postData.addQueryItem(QLatin1String("oauth_signature_method"), mOauthSignatureMethod);
        postData.addQueryItem(QLatin1String("oauth_timestamp"), mTimestamp);
        postData.addQueryItem(QLatin1String("oauth_version"), mOauthVersion);
        postData.addQueryItem(QLatin1String("oauth_token"), mOauthToken);
        postData.addQueryItem(QLatin1String("overwrite"), QLatin1String("false"));
*/
        qDebug()<<"getTokenAccess  postdata"<<url;
        if (!file->open(QIODevice::ReadOnly)) {
            delete file;
            return;
        } else {
            QNetworkReply *reply = mNetworkAccessManager->put(request, file);
            file->setParent(reply);
            connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
        }
    }
}

void DropBoxJob::accountInfo()
{
    mActionType = AccountInfo;
    mError = false;
    QUrl url(QLatin1String("https://api.dropbox.com/1/account/info"));
    url.addQueryItem(QLatin1String("oauth_consumer_key"), mOauthconsumerKey);
    url.addQueryItem(QLatin1String("oauth_nonce"), mNonce);
    url.addQueryItem(QLatin1String("oauth_signature"), mAccessOauthSignature.replace(QLatin1Char('&'),QLatin1String("%26")));
    url.addQueryItem(QLatin1String("oauth_signature_method"),mOauthSignatureMethod);
    url.addQueryItem(QLatin1String("oauth_timestamp"), mTimestamp);
    url.addQueryItem(QLatin1String("oauth_version"), mOauthVersion);
    url.addQueryItem(QLatin1String("oauth_token"), mOauthToken);
    qDebug()<<" accountInfo "<<url;
    QNetworkRequest request(url);
    qDebug()<<" url "<<url;
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));

    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void DropBoxJob::listFolder()
{
    qDebug()<<" void DropBoxJob::listFolders()";
    mActionType = ListFolder;
    mError = false;
    QUrl url(QLatin1String("https://api.dropbox.com/1/metadata/dropbox/"));
    url.addQueryItem(QLatin1String("oauth_consumer_key"),mOauthconsumerKey);
    url.addQueryItem(QLatin1String("oauth_nonce"), nonce);
    url.addQueryItem(QLatin1String("oauth_signature"), mAccessOauthSignature.replace(QLatin1Char('&'),QLatin1String("%26")));
    url.addQueryItem(QLatin1String("oauth_signature_method"),mOauthSignatureMethod);
    url.addQueryItem(QLatin1String("oauth_timestamp"), mTimestamp);
    url.addQueryItem(QLatin1String("oauth_version"),mOauthVersion);
    url.addQueryItem(QLatin1String("oauth_token"),mOauthToken);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));

    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}
