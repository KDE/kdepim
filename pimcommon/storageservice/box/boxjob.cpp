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

#include "boxjob.h"
#include "pimcommon/storageservice/storageserviceabstract.h"
#include "pimcommon/storageservice/storageservicejobconfig.h"
#include "storageservice/authdialog/storageauthviewdialog.h"

#include <QJsonDocument>

#include <KLocalizedString>

#include <QDebug>
#include <QFile>


using namespace PimCommon;

BoxJob::BoxJob(QObject *parent)
    : PimCommon::StorageServiceAbstractJob(parent)
{
    connect(mNetworkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotSendDataFinished(QNetworkReply*)));

    mRedirectUri = PimCommon::StorageServiceJobConfig::self()->oauth2RedirectUrl();
    mClientId = PimCommon::StorageServiceJobConfig::self()->boxClientId();
    mClientSecret = PimCommon::StorageServiceJobConfig::self()->boxClientSecret();
    mRedirectUri = PimCommon::StorageServiceJobConfig::self()->oauth2RedirectUrl();

    mServiceUrl = QLatin1String("https://app.box.com");
    mApiUrl = QLatin1String("https://api.box.com");
    mAuthorizePath = QLatin1String("/api/oauth2/authorize/");
    mPathToken = QLatin1String("/api/oauth2/token/");
    mFolderInfoPath = QLatin1String("/2.0/folders/");
    mFileInfoPath = QLatin1String("/2.0/files/");
    mCurrentAccountInfoPath = QLatin1String("/2.0/users/me");
}

BoxJob::~BoxJob()
{

}

void BoxJob::initializeToken(const QString &refreshToken, const QString &token)
{
    mError = false;
    mRefreshToken = refreshToken;
    mToken = token;
}

void BoxJob::createServiceFolder()
{
    mActionType = PimCommon::StorageServiceAbstract::CreateServiceFolderAction;
    mError = false;    
    createFolderJob(PimCommon::StorageServiceJobConfig::self()->defaultUploadFolder(), QLatin1String("0"));
}

void BoxJob::requestTokenAccess()
{
    mError = false;
    mActionType = PimCommon::StorageServiceAbstract::RequestTokenAction;
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
    connect(mAuthDialog.data(), &PimCommon::StorageAuthViewDialog::urlChanged, this, &BoxJob::slotRedirect);
    mAuthDialog->setUrl(url);
    if (mAuthDialog->exec()) {
        delete mAuthDialog;
    } else {
        Q_EMIT authorizationFailed(i18n("Authorization canceled."));
        delete mAuthDialog;
        deleteLater();
    }
}

void BoxJob::slotRedirect(const QUrl &url)
{
    if (url != mAuthUrl) {
        //qDebug()<<" Redirect !"<<url;
        mAuthDialog->accept();
        parseRedirectUrl(url);
    }
}

void BoxJob::parseRedirectUrl(const QUrl &url)
{
    const QList<QPair<QString, QString> > listQuery = url.queryItems();
    //qDebug()<< "listQuery "<<listQuery;

    QString authorizeCode;
    QString errorStr;
    QString errorDescription;
    const int listSize(listQuery.size());
    for (int i = 0; i < listSize; ++i) {
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

void BoxJob::getTokenAccess(const QString &authorizeCode)
{
    mActionType = PimCommon::StorageServiceAbstract::AccessTokenAction;
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
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &BoxJob::slotError);
}


void BoxJob::slotSendDataFinished(QNetworkReply *reply)
{
    const QString data = QString::fromUtf8(reply->readAll());
    reply->deleteLater();
    if (mError) {
        QJsonParseError parsingError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(data.toUtf8(), &parsingError);
        if (parsingError.error != QJsonParseError::NoError || jsonDoc.isNull()) {
            errorMessage(mActionType, i18n("Unknown Error \"%1\"", data));
            deleteLater();
            return;
        }
        const QMap<QString, QVariant> error = jsonDoc.toVariant().toMap();
        qDebug()<<" error "<<error;
        if (error.contains(QLatin1String("message")) || error.contains(QLatin1String("error_description"))) {
            QString errorStr;
            if (error.contains(QLatin1String("message")))
                errorStr = error.value(QLatin1String("message")).toString();
            else
                errorStr = error.value(QLatin1String("error_description")).toString();
            switch(mActionType) {
            case PimCommon::StorageServiceAbstract::NoneAction:
                deleteLater();
                break;
            case PimCommon::StorageServiceAbstract::RequestTokenAction:
                Q_EMIT authorizationFailed(errorStr);
                deleteLater();
                break;
            case PimCommon::StorageServiceAbstract::AccessTokenAction:
                Q_EMIT authorizationFailed(errorStr);
                deleteLater();
                break;
            case PimCommon::StorageServiceAbstract::UploadFileAction:
                Q_EMIT uploadFileFailed(errorStr);
                errorMessage(mActionType, errorStr);
                deleteLater();
                break;
            case PimCommon::StorageServiceAbstract::DownLoadFileAction:
                Q_EMIT downLoadFileFailed(errorStr);
                errorMessage(mActionType, errorStr);
                deleteLater();
                break;
            case PimCommon::StorageServiceAbstract::DeleteFileAction:
            case PimCommon::StorageServiceAbstract::CreateFolderAction:
            case PimCommon::StorageServiceAbstract::AccountInfoAction:
            case PimCommon::StorageServiceAbstract::ListFolderAction:
            case PimCommon::StorageServiceAbstract::CreateServiceFolderAction:
            case PimCommon::StorageServiceAbstract::DeleteFolderAction:
            case PimCommon::StorageServiceAbstract::RenameFolderAction:
            case PimCommon::StorageServiceAbstract::RenameFileAction:
            case PimCommon::StorageServiceAbstract::MoveFolderAction:
            case PimCommon::StorageServiceAbstract::MoveFileAction:
            case PimCommon::StorageServiceAbstract::CopyFileAction:
            case PimCommon::StorageServiceAbstract::CopyFolderAction:
            case PimCommon::StorageServiceAbstract::ShareLinkAction:
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
    //qDebug()<<" data: "<<data;
    switch(mActionType) {
    case PimCommon::StorageServiceAbstract::NoneAction:
        deleteLater();
        break;
    case PimCommon::StorageServiceAbstract::RequestTokenAction:
        deleteLater();
        break;
    case PimCommon::StorageServiceAbstract::AccessTokenAction:
        parseAccessToken(data);
        break;
    case PimCommon::StorageServiceAbstract::UploadFileAction:
        parseUploadFile(data);
        break;
    case PimCommon::StorageServiceAbstract::CreateFolderAction:
        parseCreateFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::AccountInfoAction:
        parseAccountInfo(data);
        break;
    case PimCommon::StorageServiceAbstract::ListFolderAction:
        parseListFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::CreateServiceFolderAction:
        parseCreateServiceFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::DeleteFileAction:
        parseDeleteFile(data);
        break;
    case PimCommon::StorageServiceAbstract::DeleteFolderAction:
        parseDeleteFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::CopyFileAction:
        parseCopyFile(data);
        break;
    case PimCommon::StorageServiceAbstract::CopyFolderAction:
        parseCopyFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::RenameFileAction:
        parseRenameFile(data);
        break;
    case PimCommon::StorageServiceAbstract::RenameFolderAction:
        parseRenameFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::MoveFolderAction:
        parseMoveFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::MoveFileAction:
        parseMoveFile(data);
        break;
    case PimCommon::StorageServiceAbstract::ShareLinkAction:
        parseShareLink(data);
        break;
    case PimCommon::StorageServiceAbstract::DownLoadFileAction:
        parseDownloadFile(data);
        break;
    }
}


void BoxJob::parseAccountInfo(const QString &data)
{
    //qDebug()<<" info"<<info;
    QJsonParseError parsingError;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(data.toUtf8(), &parsingError);
    if (parsingError.error != QJsonParseError::NoError || jsonDoc.isNull()) {
        qDebug()<<"parseAccountInfo parse error "<<data;
        return;
    }
    const QMap<QString, QVariant> info = jsonDoc.toVariant().toMap();

    PimCommon::AccountInfo accountInfo;
    if (info.contains(QLatin1String("space_used"))) {
        accountInfo.shared = info.value(QLatin1String("space_used")).toLongLong();
    }
    if (info.contains(QLatin1String("space_amount"))) {
        accountInfo.quota = info.value(QLatin1String("space_amount")).toLongLong();
    }
    Q_EMIT accountInfoDone(accountInfo);
    deleteLater();
}

void BoxJob::refreshToken()
{
    mActionType = PimCommon::StorageServiceAbstract::AccessTokenAction;
    QNetworkRequest request(QUrl(QLatin1String("https://www.box.com/api/oauth2/token")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    QUrl postData;
    postData.addQueryItem(QLatin1String("refresh_token"), mRefreshToken);
    postData.addQueryItem(QLatin1String("grant_type"), QLatin1String("refresh_token"));
    postData.addQueryItem(QLatin1String("client_id"), mClientId);
    postData.addQueryItem(QLatin1String("client_secret"), mClientSecret);
    //qDebug()<<"refreshToken postData: "<<postData;

    QNetworkReply *reply = mNetworkAccessManager->post(request, postData.encodedQuery());
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &BoxJob::slotError);
}


void BoxJob::deleteFile(const QString &filename)
{
    mActionType = PimCommon::StorageServiceAbstract::DeleteFileAction;
    mError = false;
    QUrl url;
    url.setUrl(mApiUrl + mFileInfoPath + filename);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    request.setRawHeader("Authorization", "Bearer "+ mToken.toLatin1());
    QNetworkReply *reply = mNetworkAccessManager->deleteResource(request);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &BoxJob::slotError);
}

void BoxJob::deleteFolder(const QString &foldername)
{
    mActionType = PimCommon::StorageServiceAbstract::DeleteFolderAction;
    mError = false;
    QUrl url;
    url.setUrl(mApiUrl + mFolderInfoPath + foldername);
    url.addQueryItem(QLatin1String("recursive"), QLatin1String("true"));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    request.setRawHeader("Authorization", "Bearer "+ mToken.toLatin1());
    QNetworkReply *reply = mNetworkAccessManager->deleteResource(request);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &BoxJob::slotError);
}

void BoxJob::renameFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::RenameFolderAction;
    mError = false;
    QUrl url;
    url.setUrl(mApiUrl + mFolderInfoPath + source);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    request.setRawHeader("Authorization", "Bearer "+ mToken.toLatin1());
    const QString data = QString::fromLatin1("{\"name\":\"%1\"}").arg(destination);

    QNetworkReply *reply = mNetworkAccessManager->put(request, data.toLatin1());
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &BoxJob::slotError);
}

void BoxJob::renameFile(const QString &oldName, const QString &newName)
{
    mActionType = PimCommon::StorageServiceAbstract::RenameFileAction;
    mError = false;

    QUrl url;
    url.setUrl(mApiUrl + mFileInfoPath + oldName);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    request.setRawHeader("Authorization", "Bearer "+ mToken.toLatin1());
    const QString data = QString::fromLatin1("{\"name\":\"%1\"}").arg(newName);

    QNetworkReply *reply = mNetworkAccessManager->put(request, data.toLatin1());
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &BoxJob::slotError);
}

void BoxJob::moveFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::MoveFolderAction;
    mError = false;
    QUrl url;
    url.setUrl(mApiUrl + mFolderInfoPath + source);
    qDebug()<<" url "<<url;
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    request.setRawHeader("Authorization", "Bearer "+ mToken.toLatin1());
    const QString data = QString::fromLatin1("{\"parent\": {\"id\" : \"%1\"} }").arg(destination);
    QNetworkReply *reply = mNetworkAccessManager->put(request, data.toLatin1());
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &BoxJob::slotError);
}

void BoxJob::moveFile(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::MoveFileAction;
    mError = false;
    QUrl url;
    url.setUrl(mApiUrl + mFileInfoPath + source);
    //qDebug()<<" url "<<url;
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    request.setRawHeader("Authorization", "Bearer "+ mToken.toLatin1());
    const QString data = QString::fromLatin1("{\"parent\": {\"id\" : \"%1\"} }").arg(destination);

    QNetworkReply *reply = mNetworkAccessManager->put(request, data.toLatin1());
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &BoxJob::slotError);
}

void BoxJob::copyFile(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::CopyFileAction;
    mError = false;
    QUrl url;
    url.setUrl(mApiUrl + mFileInfoPath + QString::fromLatin1("%1/copy").arg(source));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    request.setRawHeader("Authorization", "Bearer "+ mToken.toLatin1());
    const QString data = QString::fromLatin1("{\"parent\": {\"id\": \"%1\"}}").arg(destination);

    QNetworkReply *reply = mNetworkAccessManager->post(request, data.toLatin1());
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &BoxJob::slotError);
}

void BoxJob::copyFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::CopyFolderAction;
    mError = false;
    QUrl url;
    url.setUrl(mApiUrl + mFolderInfoPath + QString::fromLatin1("%1/copy").arg(source));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    request.setRawHeader("Authorization", "Bearer "+ mToken.toLatin1());
    const QString data = QString::fromLatin1("{\"parent\": {\"id\": \"%1\"}}").arg(destination);

    QNetworkReply *reply = mNetworkAccessManager->post(request, data.toLatin1());
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &BoxJob::slotError);
}

QNetworkReply *BoxJob::uploadFile(const QString &filename, const QString &uploadAsName, const QString &destination)
{
    QFile *file = new QFile(filename);
    if (file->exists()) {
        mActionType = PimCommon::StorageServiceAbstract::UploadFileAction;
        mError = false;
        if (file->open(QIODevice::ReadOnly)) {
            QUrl url;
            //TODO upload multipart
            url.setUrl(QLatin1String("https://upload.box.com/api/2.0/files/content"));
            QNetworkRequest request(url);
            request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
            request.setRawHeader("Authorization", "Bearer "+ mToken.toLatin1());
            QUrl postData;
            postData.addQueryItem(QLatin1String("parent_id"), destination);
            postData.addQueryItem(QLatin1String("filename"), uploadAsName);
            QNetworkReply *reply = mNetworkAccessManager->post(request, postData.encodedQuery());
            file->setParent(reply);
            connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &BoxJob::slotError);
            return reply;
        }
    }
    delete file;
    return 0;
}

void BoxJob::listFolder(const QString &folder)
{
    mActionType = PimCommon::StorageServiceAbstract::ListFolderAction;
    mError = false;
    QUrl url;
    url.setUrl(mApiUrl + mFolderInfoPath + (folder.isEmpty() ? QLatin1String("0") : folder) + QLatin1String("/items?fields=name,created_at,size,modified_at,id"));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    request.setRawHeader("Authorization", "Bearer "+ mToken.toLatin1());
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &BoxJob::slotError);
}

void BoxJob::accountInfo()
{
    mActionType = PimCommon::StorageServiceAbstract::AccountInfoAction;
    mError = false;
    QUrl url;
    url.setUrl(mApiUrl + mCurrentAccountInfoPath);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    request.setRawHeader("Authorization", "Bearer "+ mToken.toLatin1());
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &BoxJob::slotError);
}

void BoxJob::createFolderJob(const QString &foldername, const QString &destination)
{
    QUrl url;
    url.setUrl(mApiUrl + mFolderInfoPath);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    request.setRawHeader("Authorization", "Bearer "+ mToken.toLatin1());
    const QString data = QString::fromLatin1("{\"name\":\"%1\", \"parent\": {\"id\": \"%2\"}}").arg(foldername).arg((destination.isEmpty() ? QLatin1String("0") : destination));
    QNetworkReply *reply = mNetworkAccessManager->post(request, data.toLatin1());
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &BoxJob::slotError);
}

void BoxJob::createFolder(const QString &foldername, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::CreateFolderAction;
    mError = false;
    createFolderJob(foldername, destination);
}


void BoxJob::shareLink(const QString &root, const QString &fileId)
{
    Q_UNUSED(root);
    mActionType = PimCommon::StorageServiceAbstract::ShareLinkAction;
    mError = false;
    QUrl url;
    url.setUrl(mApiUrl + mFileInfoPath + fileId);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    request.setRawHeader("Authorization", "Bearer "+ mToken.toLatin1());

    const QByteArray data("{\"shared_link\": {\"access\": \"open\"}}");

    QNetworkReply *reply = mNetworkAccessManager->put(request,data);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &BoxJob::slotError);
}

void BoxJob::parseDeleteFolder(const QString &data)
{
    /*
    QJson::Parser parser;
    bool ok;

    const QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    qDebug()<<" info"<<info;
    */
    Q_EMIT deleteFolderDone(QString());
}

void BoxJob::parseDeleteFile(const QString &data)
{
    /*
    QJson::Parser parser;
    bool ok;

    const QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    qDebug()<<" info"<<info;
    */
    Q_EMIT deleteFileDone(QString());
}

void BoxJob::parseCreateServiceFolder(const QString &data)
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

void BoxJob::parseListFolder(const QString &data)
{
    Q_EMIT listFolderDone(data);
    deleteLater();
}

void BoxJob::parseCreateFolder(const QString &data)
{
    const QString folderName = parseNameInfo(data);
    Q_EMIT createFolderDone(folderName);
    deleteLater();
}

void BoxJob::parseUploadFile(const QString &data)
{
    const QString folderName = parseNameInfo(data);
    Q_EMIT uploadFileDone(folderName);
    //shareLink(QString());
    deleteLater();
}

void BoxJob::parseCopyFile(const QString &data)
{
    const QString filename = parseNameInfo(data);
    Q_EMIT copyFileDone(filename);
    deleteLater();
}

void BoxJob::parseRenameFile(const QString &data)
{
    const QString filename = parseNameInfo(data);
    Q_EMIT renameFileDone(filename);
    deleteLater();
}

void BoxJob::parseRenameFolder(const QString &data)
{
    const QString filename = parseNameInfo(data);
    Q_EMIT renameFolderDone(filename);
    deleteLater();
}

void BoxJob::parseCopyFolder(const QString &data)
{
    const QString filename = parseNameInfo(data);
    Q_EMIT copyFolderDone(filename);
    deleteLater();
}

void BoxJob::parseMoveFolder(const QString &data)
{
    const QString filename = parseNameInfo(data);
    Q_EMIT moveFolderDone(filename);
    deleteLater();
}

void BoxJob::parseMoveFile(const QString &data)
{
    const QString filename = parseNameInfo(data);
    Q_EMIT moveFileDone(filename);
    deleteLater();
}

QString BoxJob::parseNameInfo(const QString &data)
{
    QString filename;
    QJsonParseError parsingError;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(data.toUtf8(), &parsingError);
    if (parsingError.error != QJsonParseError::NoError || jsonDoc.isNull()) {
        return filename;
    }
    const QMap<QString, QVariant> info = jsonDoc.toVariant().toMap();

    if (info.contains(QLatin1String("name"))) {
        filename = info.value(QLatin1String("name")).toString();
    }
    return filename;
}

void BoxJob::parseShareLink(const QString &data)
{
    QString url;
    QJsonParseError parsingError;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(data.toUtf8(), &parsingError);
    if (parsingError.error != QJsonParseError::NoError || jsonDoc.isNull()) {
        qDebug()<<"parseShareLink error "<<data;
        return;
    }
    const QMap<QString, QVariant> info = jsonDoc.toVariant().toMap();
    if (info.contains(QLatin1String("shared_link"))) {
        const QVariantMap map = info.value(QLatin1String("shared_link")).toMap();
        if (map.contains(QLatin1String("url"))) {
            url = map.value(QLatin1String("url")).toString();
        }
    }
    Q_EMIT shareLinkDone(url);
    deleteLater();
}

void BoxJob::parseDownloadFile(const QString &data)
{
    qDebug()<<" Data "<<data;
    Q_EMIT downLoadFileDone(QString());
}

QNetworkReply * BoxJob::downloadFile(const QString &name, const QString &fileId, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::DownLoadFileAction;
    mError = false;

    mActionType = PimCommon::StorageServiceAbstract::DownLoadFileAction;
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
        connect(reply, &QNetworkReply::readyRead, this, &BoxJob::slotDownloadReadyRead);
        connect(reply, &QNetworkReply::downloadProgress, this, &BoxJob::slotuploadDownloadFileProgress);
        connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &BoxJob::slotError);
        return reply;
    } else {
        delete mDownloadFile;
    }
    return 0;
}

void BoxJob::parseAccessToken(const QString &data)
{
    QJsonParseError parsingError;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(data.toUtf8(), &parsingError);
    if (parsingError.error != QJsonParseError::NoError || jsonDoc.isNull()) {
        qDebug()<<"parseAccessToken error "<<data;
        return;
    }
    const QMap<QString, QVariant> info = jsonDoc.toVariant().toMap();

    //qDebug()<<" info"<<info;
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
    //qDebug()<<" parseAccessToken";
    Q_EMIT authorizationDone(mRefreshToken, mToken, expireInTime);
    deleteLater();
}
