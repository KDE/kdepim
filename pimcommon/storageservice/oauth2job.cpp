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

#include "oauth2job.h"
#include "storageservice/storageauthviewdialog.h"
#include "pimcommon/storageservice/storageservicejobconfig.h"

#include <KLocalizedString>

#include <qjson/parser.h>

#include <QNetworkAccessManager>
#include <QDebug>
#include <QNetworkReply>
#include <QPointer>

using namespace PimCommon;

OAuth2Job::OAuth2Job(QObject *parent)
    : PimCommon::StorageServiceAbstractJob(parent),
      mExpireInTime(0),
      mNeedRefreshToken(false)
{
    mRedirectUri = PimCommon::StorageServiceJobConfig::self()->oauth2RedirectUrl();
    connect(mNetworkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotSendDataFinished(QNetworkReply*)));
}

OAuth2Job::~OAuth2Job()
{

}

void OAuth2Job::initializeToken(const QString &refreshToken, const QString &token, const QDateTime &expireDateTime)
{
    mError = false;
    mRefreshToken = refreshToken;
    mToken = token;
    mNeedRefreshToken = (QDateTime::currentDateTime() >= expireDateTime);
}

void OAuth2Job::createServiceFolder()
{
    mActionType = PimCommon::StorageServiceAbstract::CreateServiceFolder;
    mError = false;
    //TODO
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    qDebug()<<" not implemented";
    deleteLater();
}

void OAuth2Job::downloadFile(const QString &filename)
{
    mActionType = PimCommon::StorageServiceAbstract::DownLoadFile;
    mError = false;
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    qDebug()<<" not implemented";
    deleteLater();
}

void OAuth2Job::deleteFile(const QString &filename)
{
    mActionType = PimCommon::StorageServiceAbstract::DeleteFile;
    mError = false;
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    qDebug()<<" not implemented";
    deleteLater();
}

void OAuth2Job::deleteFolder(const QString &foldername)
{
    mActionType = PimCommon::StorageServiceAbstract::DeleteFolder;
    mError = false;
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    qDebug()<<" not implemented";
    deleteLater();
}

void OAuth2Job::renameFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::RenameFolder;
    mError = false;
    qDebug()<<" not implemented";
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    //TODO
    deleteLater();
}

void OAuth2Job::renameFile(const QString &oldName, const QString &newName)
{
    mActionType = PimCommon::StorageServiceAbstract::RenameFile;
    mError = false;
    qDebug()<<" not implemented";
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    //TODO
    deleteLater();
}

void OAuth2Job::moveFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::MoveFolder;
    mError = false;
    qDebug()<<" not implemented";
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    //TODO
    deleteLater();
}

void OAuth2Job::moveFile(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::MoveFile;
    mError = false;
    qDebug()<<" not implemented";
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    //TODO
    deleteLater();
}

void OAuth2Job::copyFile(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::CopyFile;
    mError = false;
    qDebug()<<" not implemented";
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    //TODO
    deleteLater();
}

void OAuth2Job::requestTokenAccess()
{
    mError = false;
    mActionType = PimCommon::StorageServiceAbstract::RequestToken;
    QUrl url(mServiceUrl + mAuthorizePath );
    url.addQueryItem(QLatin1String("response_type"), QLatin1String("code"));
    url.addQueryItem(QLatin1String("client_id"), mClientId);
    url.addQueryItem(QLatin1String("redirect_uri"), mRedirectUri);
    if (!mScope.isEmpty())
        url.addQueryItem(QLatin1String("scope"),mScope);
    mAuthUrl = url;
    //qDebug()<<" url"<<url;
    delete mAuthDialog;
    mAuthDialog = new PimCommon::StorageAuthViewDialog;
    connect(mAuthDialog, SIGNAL(urlChanged(QUrl)), this, SLOT(slotRedirect(QUrl)));
    mAuthDialog->setUrl(url);
    if (mAuthDialog->exec()) {
        delete mAuthDialog;
    } else {
        Q_EMIT authorizationFailed(i18n("Authorization canceled."));
        delete mAuthDialog;
        deleteLater();
    }    
}

void OAuth2Job::slotRedirect(const QUrl &url)
{
    if (url != mAuthUrl) {
        //qDebug()<<" Redirect !"<<url;
        mAuthDialog->accept();
        parseRedirectUrl(url);
    }
}

void OAuth2Job::parseRedirectUrl(const QUrl &url)
{
    const QList<QPair<QString, QString> > listQuery = url.queryItems();
    //qDebug()<< "listQuery "<<listQuery;

    QString authorizeCode;
    QString errorStr;
    QString errorDescription;
    for (int i = 0; i < listQuery.size(); ++i) {
        const QPair<QString, QString> item = listQuery.at(i);
        if (item.first == QLatin1String("code")) {
            authorizeCode = item.second;
            break;
        } else if (item.first == QLatin1String("error")) {
            errorStr = item.second;
        } else if (item.first == QLatin1String("error_description")) {
            errorDescription = item.second;
        }
    }
    if (!authorizeCode.isEmpty()) {
        getTokenAccess(authorizeCode);
    } else {
        Q_EMIT authorizationFailed(errorStr + QLatin1Char(' ') + errorDescription);
        deleteLater();
    }
}

void OAuth2Job::getTokenAccess(const QString &authorizeCode)
{
    mActionType = PimCommon::StorageServiceAbstract::AccessToken;
    mError = false;
    QNetworkRequest request(QUrl(mServiceUrl + mPathToken));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    QUrl postData;
    postData.addQueryItem(QLatin1String("code"), authorizeCode);
    postData.addQueryItem(QLatin1String("redirect_uri"), mRedirectUri);
    postData.addQueryItem(QLatin1String("grant_type"), QLatin1String("authorization_code"));
    postData.addQueryItem(QLatin1String("client_id"), mClientId);
    postData.addQueryItem(QLatin1String("client_secret"), mClientSecret);
    QNetworkReply *reply = mNetworkAccessManager->post(request, postData.encodedQuery());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void OAuth2Job::uploadFile(const QString &filename)
{
    mActionType = PimCommon::StorageServiceAbstract::UploadFile;
    mError = false;
    QUrl url;
    url.setUrl(mApiUrl + mFileInfoPath + QLatin1String("content"));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    request.setRawHeader("Authorization", "Bearer "+ mToken.toLatin1());
    qDebug()<<" request "<<request.rawHeaderList()<<" reqyest "<<request.url();
    QUrl postData;
    //TODO
    QNetworkReply *reply = mNetworkAccessManager->post(request, postData.encodedQuery());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void OAuth2Job::listFolder(const QString &folder)
{
    mActionType = PimCommon::StorageServiceAbstract::ListFolder;
    mError = false;
    QUrl url;
    url.setUrl(mApiUrl + mFolderInfoPath + (folder.isEmpty() ? QLatin1String("0") : folder));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    request.setRawHeader("Authorization", "Bearer "+ mToken.toLatin1());
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void OAuth2Job::accountInfo()
{
    mActionType = PimCommon::StorageServiceAbstract::AccountInfo;
    mError = false;
    QUrl url;
    url.setUrl(mApiUrl + mCurrentAccountInfoPath);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    request.setRawHeader("Authorization", "Bearer "+ mToken.toLatin1());
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void OAuth2Job::createFolder(const QString &foldername)
{
    mActionType = PimCommon::StorageServiceAbstract::CreateFolder;
    mError = false;
    QUrl url;
    url.setUrl(mApiUrl + mFolderInfoPath);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    request.setRawHeader("Authorization", "Bearer "+ mToken.toLatin1());
    qDebug()<<" request "<<request.rawHeaderList()<<" url "<<request.url();
    QUrl postData;
    postData.addQueryItem(QLatin1String("name"), foldername);
    //postData.addQueryItem(QLatin1String("parent"), QLatin1String("{\'id\': \'0\'}"));
    QNetworkReply *reply = mNetworkAccessManager->post(request, postData.encodedQuery());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void OAuth2Job::shareLink(const QString &fileId)
{
    mActionType = PimCommon::StorageServiceAbstract::ShareLink;
    mError = false;
    QUrl url;
    url.setUrl(mApiUrl + mFileInfoPath + fileId);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    request.setRawHeader("Authorization", "Bearer "+ mToken.toLatin1());
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void OAuth2Job::shareLink(const QString &root, const QString &path)
{
    mActionType = PimCommon::StorageServiceAbstract::ShareLink;
    mError = false;
    QUrl url;
    QString fileId; //TODO
    url.setUrl(mApiUrl + mFileInfoPath + fileId);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    request.setRawHeader("Authorization", "Bearer "+ mToken.toLatin1());
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}


void OAuth2Job::slotSendDataFinished(QNetworkReply *reply)
{
    const QString data = QString::fromUtf8(reply->readAll());
    reply->deleteLater();
    if (mError) {
        qDebug()<<" error type "<<data;
        QJson::Parser parser;
        bool ok;

        QMap<QString, QVariant> error = parser.parse(data.toUtf8(), &ok).toMap();
        qDebug()<<" error "<<error;
        if (error.contains(QLatin1String("message"))) {
            const QString errorStr = error.value(QLatin1String("message")).toString();
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
            case PimCommon::StorageServiceAbstract::CreateFolder:
            case PimCommon::StorageServiceAbstract::AccountInfo:
            case PimCommon::StorageServiceAbstract::ListFolder:
            case PimCommon::StorageServiceAbstract::DownLoadFile:
            case PimCommon::StorageServiceAbstract::CreateServiceFolder:
            case PimCommon::StorageServiceAbstract::DeleteFile:
            case PimCommon::StorageServiceAbstract::DeleteFolder:
            case PimCommon::StorageServiceAbstract::RenameFolder:
            case PimCommon::StorageServiceAbstract::RenameFile:
            case PimCommon::StorageServiceAbstract::MoveFolder:
            case PimCommon::StorageServiceAbstract::MoveFile:
            case PimCommon::StorageServiceAbstract::CopyFile:
                errorMessage(mActionType, errorStr);
                deleteLater();
                break;
            default:
                qDebug()<<" Action Type unknown:"<<mActionType;
                Q_EMIT actionFailed(QLatin1String("Action Type Unknown"));
                deleteLater();
                break;
            }
        } else {
            errorMessage(mActionType, i18n("Unknown Error \"%1\"", data));
            deleteLater();
        }
        return;
    }
    qDebug()<<" data: "<<data;
    switch(mActionType) {
    case PimCommon::StorageServiceAbstract::NoneAction:
        deleteLater();
        break;
    case PimCommon::StorageServiceAbstract::RequestToken:
        deleteLater();
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
    case PimCommon::StorageServiceAbstract::DownLoadFile:
    case PimCommon::StorageServiceAbstract::DeleteFile:
    case PimCommon::StorageServiceAbstract::DeleteFolder:
    case PimCommon::StorageServiceAbstract::RenameFolder:
    case PimCommon::StorageServiceAbstract::RenameFile:
    case PimCommon::StorageServiceAbstract::MoveFolder:
    case PimCommon::StorageServiceAbstract::MoveFile:
    case PimCommon::StorageServiceAbstract::CopyFile:
        Q_EMIT actionFailed(QLatin1String("Not Implemented"));
        deleteLater();
        break;
    default:
        qDebug()<<" Action Type unknown:"<<mActionType;
        Q_EMIT actionFailed(QLatin1String("Action type unknown"));
        deleteLater();
        break;
    }
}

void OAuth2Job::parseCreateServiceFolder(const QString &data)
{
    QJson::Parser parser;
    bool ok;

    const QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    qDebug()<<" info"<<info;
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    deleteLater();
}

void OAuth2Job::parseListFolder(const QString &data)
{
    Q_EMIT listFolderDone(data);
    deleteLater();
}

void OAuth2Job::parseAccountInfo(const QString &)
{
    //TODO reimplement in derivated function
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    deleteLater();
}

void OAuth2Job::parseCreateFolder(const QString &data)
{
    //TODO
    qDebug()<<" data "<<data;
    Q_EMIT createFolderDone(QString());
    deleteLater();
}

void OAuth2Job::parseUploadFile(const QString &data)
{
    //TODO
    qDebug()<<" data "<<data;
    Q_EMIT uploadFileDone(QString());
    deleteLater();
}


void OAuth2Job::parseAccessToken(const QString &data)
{
    QJson::Parser parser;
    bool ok;

    const QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    qDebug()<<" info"<<info;
    if (info.contains(QLatin1String("refresh_token"))) {
        mRefreshToken = info.value(QLatin1String("refresh_token")).toString();
    }
    if (info.contains(QLatin1String("access_token"))) {
        mToken = info.value(QLatin1String("access_token")).toString();
    }
    if (info.contains(QLatin1String("expires_in"))) {
        mExpireInTime = info.value(QLatin1String("expires_in")).toLongLong();
    }
    Q_EMIT authorizationDone(mRefreshToken, mToken, mExpireInTime);
    //TODO save it.
    deleteLater();
}


void OAuth2Job::refreshToken()
{
    mActionType = PimCommon::StorageServiceAbstract::AccessToken;
    QNetworkRequest request(QUrl(mServiceUrl + QLatin1String("/oauth/token")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));

    QUrl postData;

    postData.addQueryItem(QLatin1String("refresh_token"), mRefreshToken);
    postData.addQueryItem(QLatin1String("grant_type"), QLatin1String("refresh_token"));
    postData.addQueryItem(QLatin1String("client_id"), mClientId);
    postData.addQueryItem(QLatin1String("client_secret"), mClientSecret);
    qDebug()<<"refreshToken postData: "<<postData;

    QNetworkReply *reply = mNetworkAccessManager->post(request, postData.encodedQuery());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}
