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


#include <qjson/parser.h>

#include <QDebug>
#include <QFile>

using namespace PimCommon;

BoxJob::BoxJob(QObject *parent)
    : PimCommon::OAuth2Job(parent)
{
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

void BoxJob::parseAccountInfo(const QString &data)
{
    QJson::Parser parser;
    bool ok;

    const QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    //qDebug()<<" info"<<info;
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
    mActionType = PimCommon::StorageServiceAbstract::AccessToken;
    QNetworkRequest request(QUrl(QLatin1String("https://www.box.com/api/oauth2/token")));
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


void BoxJob::deleteFile(const QString &filename)
{
    mActionType = PimCommon::StorageServiceAbstract::DeleteFile;
    mError = false;
    QUrl url;
    url.setUrl(mApiUrl + mFileInfoPath + filename);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    request.setRawHeader("Authorization", "Bearer "+ mToken.toLatin1());
    QNetworkReply *reply = mNetworkAccessManager->deleteResource(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void BoxJob::deleteFolder(const QString &foldername)
{
    mActionType = PimCommon::StorageServiceAbstract::DeleteFolder;
    mError = false;
    QUrl url;
    url.setUrl(mApiUrl + mFolderInfoPath + foldername);
    url.addQueryItem(QLatin1String("recursive"), QLatin1String("true"));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    request.setRawHeader("Authorization", "Bearer "+ mToken.toLatin1());
    QNetworkReply *reply = mNetworkAccessManager->deleteResource(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void BoxJob::renameFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::RenameFolder;
    mError = false;
    QUrl url;
    url.setUrl(mApiUrl + mFolderInfoPath + source);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    request.setRawHeader("Authorization", "Bearer "+ mToken.toLatin1());
    const QString data = QString::fromLatin1("{\"name\":\"%1\"}").arg(destination);

    QNetworkReply *reply = mNetworkAccessManager->put(request, data.toLatin1());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void BoxJob::renameFile(const QString &oldName, const QString &newName)
{
    mActionType = PimCommon::StorageServiceAbstract::RenameFile;
    mError = false;

    QUrl url;
    url.setUrl(mApiUrl + mFileInfoPath + oldName);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    request.setRawHeader("Authorization", "Bearer "+ mToken.toLatin1());
    const QString data = QString::fromLatin1("{\"name\":\"%1\"}").arg(newName);

    QNetworkReply *reply = mNetworkAccessManager->put(request, data.toLatin1());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void BoxJob::moveFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::MoveFolder;
    mError = false;
    QUrl url;
    url.setUrl(mApiUrl + mFolderInfoPath + source);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    request.setRawHeader("Authorization", "Bearer "+ mToken.toLatin1());
    const QString data = QString::fromLatin1("{\"parent\":\"%1\"}").arg(destination);

    QNetworkReply *reply = mNetworkAccessManager->put(request, data.toLatin1());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void BoxJob::moveFile(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::MoveFile;
    mError = false;
    QUrl url;
    url.setUrl(mApiUrl + mFileInfoPath + source);
    qDebug()<<" url "<<url;
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    request.setRawHeader("Authorization", "Bearer "+ mToken.toLatin1());
    const QString data = QString::fromLatin1("{\"parent\":\"%1\"}").arg(destination);

    QNetworkReply *reply = mNetworkAccessManager->put(request, data.toLatin1());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void BoxJob::copyFile(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::CopyFile;
    mError = false;
    QUrl url;
    url.setUrl(mApiUrl + mFileInfoPath + QString::fromLatin1("%1/copy").arg(source));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    request.setRawHeader("Authorization", "Bearer "+ mToken.toLatin1());
    const QString data = QString::fromLatin1("{\"parent\": {\"id\": \"%1\"}}").arg(destination);

    QNetworkReply *reply = mNetworkAccessManager->post(request, data.toLatin1());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void BoxJob::copyFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::CopyFolder;
    mError = false;
    QUrl url;
    url.setUrl(mApiUrl + mFolderInfoPath + QString::fromLatin1("%1/copy").arg(source));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    request.setRawHeader("Authorization", "Bearer "+ mToken.toLatin1());
    const QString data = QString::fromLatin1("{\"parent\": {\"id\": \"%1\"}}").arg(destination);

    QNetworkReply *reply = mNetworkAccessManager->post(request, data.toLatin1());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

QNetworkReply *BoxJob::uploadFile(const QString &filename, const QString &uploadAsName, const QString &destination)
{
    QFile *file = new QFile(filename);
    if (file->exists()) {
        mActionType = PimCommon::StorageServiceAbstract::UploadFile;
        mError = false;
        if (file->open(QIODevice::ReadOnly)) {
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
        } else {
            delete file;
        }
    }
    return 0;
}

void BoxJob::listFolder(const QString &folder)
{
    mActionType = PimCommon::StorageServiceAbstract::ListFolder;
    mError = false;
    QUrl url;
    url.setUrl(mApiUrl + mFolderInfoPath + (folder.isEmpty() ? QLatin1String("0") : folder) + QLatin1String("/items?fields=name,created_at,size,modified_at,id"));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    request.setRawHeader("Authorization", "Bearer "+ mToken.toLatin1());
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void BoxJob::accountInfo()
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

void BoxJob::createFolder(const QString &foldername, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::CreateFolder;
    mError = false;
    QUrl url;
    url.setUrl(mApiUrl + mFolderInfoPath);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    request.setRawHeader("Authorization", "Bearer "+ mToken.toLatin1());
    const QString data = QString::fromLatin1("{\"name\":\"%1\", \"parent\": {\"id\": \"%2\"}}").arg(foldername).arg((destination.isEmpty() ? QLatin1String("0") : destination));
    QNetworkReply *reply = mNetworkAccessManager->post(request, data.toLatin1());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}


void BoxJob::shareLink(const QString &root, const QString &fileId)
{
    Q_UNUSED(root);
    mActionType = PimCommon::StorageServiceAbstract::ShareLink;
    mError = false;
    QUrl url;
    url.setUrl(mApiUrl + mFileInfoPath + fileId);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    request.setRawHeader("Authorization", "Bearer "+ mToken.toLatin1());

    const QByteArray data("{\"shared_link\": {\"access\": \"open\"}}");

    QNetworkReply *reply = mNetworkAccessManager->put(request,data);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
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
    qDebug()<<" data"<<data;
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
    QJson::Parser parser;
    bool ok;

    QString filename;
    const QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    if (info.contains(QLatin1String("name"))) {
        filename = info.value(QLatin1String("name")).toString();
    }
    return filename;
}

void BoxJob::parseShareLink(const QString &data)
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

void BoxJob::parseDownloadFile(const QString &data)
{
    qDebug()<<" Data "<<data;
    Q_EMIT downLoadFileDone(QString());
}

QNetworkReply * BoxJob::downloadFile(const QString &name, const QString &fileId, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::DownLoadFile;
    mError = false;

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
    return 0;
}
