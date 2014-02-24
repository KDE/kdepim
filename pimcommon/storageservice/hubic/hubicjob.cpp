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

#include "hubicjob.h"
#include "pimcommon/storageservice/storageserviceabstract.h"
#include "pimcommon/storageservice/storageservicejobconfig.h"
#include "storageservice/authdialog/storageauthviewdialog.h"

#include <qjson/parser.h>

#include <KLocalizedString>

#include <QDebug>
#include <QFile>

using namespace PimCommon;

HubicJob::HubicJob(QObject *parent)
    : PimCommon::StorageServiceAbstractJob(parent)
{
    connect(mNetworkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotSendDataFinished(QNetworkReply*)));
    mScope = PimCommon::StorageServiceJobConfig::self()->hubicScope();
    mClientId = PimCommon::StorageServiceJobConfig::self()->hubicClientId();
    mClientSecret = PimCommon::StorageServiceJobConfig::self()->hubicClientSecret();
    mRedirectUri = PimCommon::StorageServiceJobConfig::self()->oauth2RedirectUrl();
    mServiceUrl = QLatin1String("https://api.hubic.com");
    mApiUrl = QLatin1String("https://api.hubic.com");
    mAuthorizePath = QLatin1String("/oauth/auth/");
    mPathToken = QLatin1String("/oauth/token/");
    mCurrentAccountInfoPath = QLatin1String("/1.0/account/usage");
    mSwiftTokenPath = QLatin1String("/1.0/account/credentials");
}

HubicJob::~HubicJob()
{

}

void HubicJob::initializeToken(const QString &refreshToken, const QString &token)
{
    mError = false;
    mRefreshToken = refreshToken;
    mToken = token;
}

void HubicJob::createServiceFolder()
{
    mActionType = PimCommon::StorageServiceAbstract::CreateServiceFolder;
    mError = false;
    //TODO
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    qDebug()<<" not implemented";
    deleteLater();
}

void HubicJob::requestTokenAccess()
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

void HubicJob::slotRedirect(const QUrl &url)
{
    if (url != mAuthUrl) {
        //qDebug()<<" Redirect !"<<url;
        mAuthDialog->accept();
        parseRedirectUrl(url);
    }
}

void HubicJob::parseRedirectUrl(const QUrl &url)
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

void HubicJob::getTokenAccess(const QString &authorizeCode)
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


void HubicJob::slotSendDataFinished(QNetworkReply *reply)
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
                Q_EMIT uploadFileFailed(errorStr);
                errorMessage(mActionType, errorStr);
                deleteLater();
                break;
            case PimCommon::StorageServiceAbstract::DownLoadFile:
                Q_EMIT downLoadFileFailed(errorStr);
                errorMessage(mActionType, errorStr);
                deleteLater();
                break;
            case PimCommon::StorageServiceAbstract::DeleteFile:
            case PimCommon::StorageServiceAbstract::CreateFolder:
            case PimCommon::StorageServiceAbstract::AccountInfo:
            case PimCommon::StorageServiceAbstract::ListFolder:
            case PimCommon::StorageServiceAbstract::CreateServiceFolder:
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
        } else {
            if (!mErrorMsg.isEmpty()) {
                errorMessage(mActionType, mErrorMsg);
            } else {
                errorMessage(mActionType, i18n("Unknown Error \"%1\"", data));
            }
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
    case PimCommon::StorageServiceAbstract::DeleteFile:
        parseDeleteFile(data);
        break;
    case PimCommon::StorageServiceAbstract::DeleteFolder:
        parseDeleteFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::CopyFile:
        parseCopyFile(data);
        break;
    case PimCommon::StorageServiceAbstract::CopyFolder:
        parseCopyFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::RenameFile:
        parseRenameFile(data);
        break;
    case PimCommon::StorageServiceAbstract::RenameFolder:
        parseRenameFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::MoveFolder:
        parseMoveFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::MoveFile:
        parseMoveFile(data);
        break;
    case PimCommon::StorageServiceAbstract::ShareLink:
        parseShareLink(data);
        break;
    case PimCommon::StorageServiceAbstract::DownLoadFile:
        parseDownloadFile(data);
        break;
    }
}


void HubicJob::parseAccountInfo(const QString &data)
{
    QJson::Parser parser;
    bool ok;

    const QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    //qDebug()<<" info"<<info;
    PimCommon::AccountInfo accountInfo;
    if (info.contains(QLatin1String("used"))) {
        accountInfo.shared = info.value(QLatin1String("used")).toLongLong();
    }
    if (info.contains(QLatin1String("quota"))) {
        accountInfo.quota = info.value(QLatin1String("quota")).toLongLong();
    }
    Q_EMIT accountInfoDone(accountInfo);
    deleteLater();
}

void HubicJob::refreshToken()
{
    mActionType = PimCommon::StorageServiceAbstract::AccessToken;
    QNetworkRequest request(QUrl(QLatin1String("https://api.hubic.com/oauth/token/")));
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


void HubicJob::deleteFile(const QString &filename)
{
    mActionType = PimCommon::StorageServiceAbstract::DeleteFile;
    mError = false;
}

void HubicJob::deleteFolder(const QString &foldername)
{
    mActionType = PimCommon::StorageServiceAbstract::DeleteFolder;
    mError = false;
}

void HubicJob::renameFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::RenameFolder;
    mError = false;
}

void HubicJob::renameFile(const QString &oldName, const QString &newName)
{
    mActionType = PimCommon::StorageServiceAbstract::RenameFile;
    mError = false;
}

void HubicJob::moveFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::MoveFolder;
    mError = false;
}

void HubicJob::moveFile(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::MoveFile;
    mError = false;
}

void HubicJob::copyFile(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::CopyFile;
    mError = false;
}

void HubicJob::copyFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::CopyFolder;
    mError = false;
}

QNetworkReply *HubicJob::uploadFile(const QString &filename, const QString &uploadAsName, const QString &destination)
{
    QFile *file = new QFile(filename);
    if (file->exists()) {
        mActionType = PimCommon::StorageServiceAbstract::UploadFile;
        mError = false;
        if (file->open(QIODevice::ReadOnly)) {
#if 0
            QUrl url;
            url.setUrl(QLatin1String("https://upload.box.com/api/2.0/files/content"));
            QNetworkRequest request(url);
            request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
            request.setRawHeader("Authorization", "Bearer "+ mToken.toLatin1());
            QUrl postData;
            postData.addQueryItem(QLatin1String("parent_id"), destination);
            postData.addQueryItem(QLatin1String("filename"), uploadAsName);
            QNetworkReply *reply = mNetworkAccessManager->post(request, postData.encodedQuery());
            file->setParent(reply);
            connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
            return reply;
#else
            return 0;
#endif
        }
    }
    delete file;
    return 0;
}

void HubicJob::listFolder(const QString &folder)
{
    mActionType = PimCommon::StorageServiceAbstract::ListFolder;
    mError = false;
    QUrl url;
    url.setUrl(mApiUrl + mSwiftTokenPath);
    qDebug()<<"url"<<url;
    QNetworkRequest request(url);
    qDebug()<<" mToken"<<mToken;
    request.setRawHeader("X-Auth-Token", mToken.toLatin1());
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));

#if 0
    QUrl url;
    url.setUrl(mApiUrl + mFolderInfoPath + (folder.isEmpty() ? QLatin1String("0") : folder) + QLatin1String("/items?fields=name,created_at,size,modified_at,id"));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    request.setRawHeader("Authorization", "Bearer "+ mToken.toLatin1());
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
#endif
}

void HubicJob::accountInfo()
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

void HubicJob::createFolder(const QString &foldername, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::CreateFolder;
    mError = false;
}


void HubicJob::shareLink(const QString &root, const QString &fileId)
{
    Q_UNUSED(root);
    mActionType = PimCommon::StorageServiceAbstract::ShareLink;
    mError = false;
}

void HubicJob::parseDeleteFolder(const QString &data)
{
    /*
    QJson::Parser parser;
    bool ok;

    const QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    qDebug()<<" info"<<info;
    */
    Q_EMIT deleteFolderDone(QString());
}

void HubicJob::parseDeleteFile(const QString &data)
{
    /*
    QJson::Parser parser;
    bool ok;

    const QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    qDebug()<<" info"<<info;
    */
    Q_EMIT deleteFileDone(QString());
}

void HubicJob::parseCreateServiceFolder(const QString &data)
{
#if 0
    QJson::Parser parser;
    bool ok;

    const QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    qDebug()<<" info"<<info;
#endif
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    deleteLater();
}

void HubicJob::parseListFolder(const QString &data)
{
    Q_EMIT listFolderDone(data);
    deleteLater();
}

void HubicJob::parseCreateFolder(const QString &data)
{
    const QString folderName = parseNameInfo(data);
    Q_EMIT createFolderDone(folderName);
    deleteLater();
}

void HubicJob::parseUploadFile(const QString &data)
{
    const QString folderName = parseNameInfo(data);
    qDebug()<<" data"<<data;
    Q_EMIT uploadFileDone(folderName);
    //shareLink(QString());
    deleteLater();
}

void HubicJob::parseCopyFile(const QString &data)
{
    const QString filename = parseNameInfo(data);
    Q_EMIT copyFileDone(filename);
    deleteLater();
}

void HubicJob::parseRenameFile(const QString &data)
{
    const QString filename = parseNameInfo(data);
    Q_EMIT renameFileDone(filename);
    deleteLater();
}

void HubicJob::parseRenameFolder(const QString &data)
{
    const QString filename = parseNameInfo(data);
    Q_EMIT renameFolderDone(filename);
    deleteLater();
}

void HubicJob::parseCopyFolder(const QString &data)
{
    const QString filename = parseNameInfo(data);
    Q_EMIT copyFolderDone(filename);
    deleteLater();
}

void HubicJob::parseMoveFolder(const QString &data)
{
    const QString filename = parseNameInfo(data);
    Q_EMIT moveFolderDone(filename);
    deleteLater();
}

void HubicJob::parseMoveFile(const QString &data)
{
    const QString filename = parseNameInfo(data);
    Q_EMIT moveFileDone(filename);
    deleteLater();
}

QString HubicJob::parseNameInfo(const QString &data)
{
    QJson::Parser parser;
    bool ok;

    QString filename;
    const QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    if (info.contains(QLatin1String("name"))) {
        filename = info.value(QLatin1String("name")).toString();
    }
    return filename;
}

void HubicJob::parseShareLink(const QString &data)
{
    QJson::Parser parser;
    bool ok;

    QString url;
    const QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    if (info.contains(QLatin1String("shared_link"))) {
        const QVariantMap map = info.value(QLatin1String("shared_link")).toMap();
        if (map.contains(QLatin1String("url"))) {
            url = map.value(QLatin1String("url")).toString();
        }
    }
    Q_EMIT shareLinkDone(url);
    deleteLater();
}

void HubicJob::parseDownloadFile(const QString &data)
{
    qDebug()<<" Data "<<data;
    Q_EMIT downLoadFileDone(QString());
}

QNetworkReply * HubicJob::downloadFile(const QString &name, const QString &fileId, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::DownLoadFile;
    mError = false;
#if 0
    mActionType = PimCommon::StorageServiceAbstract::DownLoadFile;
    mError = false;
    const QString defaultDestination = (destination.isEmpty() ? PimCommon::StorageServiceJobConfig::self()->defaultUploadFolder() : destination);
    delete mDownloadFile;
    mDownloadFile = new QFile(defaultDestination+ QLatin1Char('/') + name);
    if (mDownloadFile->open(QIODevice::WriteOnly)) {
        QUrl url;
        qDebug()<<" fileId "<<fileId<<" name "<<name;
        url.setUrl(mApiUrl + mFileInfoPath + fileId + QLatin1String("/content"));
        qDebug()<<"url!"<<url;
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
        request.setRawHeader("Authorization", "Bearer "+ mToken.toLatin1());
        QNetworkReply *reply = mNetworkAccessManager->get(request);
        mDownloadFile->setParent(reply);
        connect(reply, SIGNAL(readyRead()), this, SLOT(slotDownloadReadyRead()));
        connect(reply, SIGNAL(downloadProgress(qint64,qint64)), SLOT(slotuploadDownloadFileProgress(qint64,qint64)));
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
        return reply;
    } else {
        delete mDownloadFile;
    }
#endif
    return 0;
}

void HubicJob::parseAccessToken(const QString &data)
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
    qint64 expireInTime = 0;
    if (info.contains(QLatin1String("expires_in"))) {
        expireInTime = info.value(QLatin1String("expires_in")).toLongLong();
    }
    qDebug()<<" parseAccessToken";
    Q_EMIT authorizationDone(mRefreshToken, mToken, expireInTime);
    deleteLater();
}
