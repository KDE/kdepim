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

#include "yousenditjob.h"
#include "pimcommon/storageservice/logindialog.h"
#include "pimcommon/storageservice/storageserviceabstract.h"

#include <KLocalizedString>

#include <qjson/parser.h>

#include <QNetworkAccessManager>
#include <QDebug>
#include <QNetworkReply>
#include <QPointer>

using namespace PimCommon;

YouSendItJob::YouSendItJob(QObject *parent)
    : PimCommon::StorageServiceAbstractJob(parent)
{
    mApiKey = QLatin1String("fnab8fkgwrka7v6zs2ycd34a");
    mDefaultUrl = QLatin1String("https://test2-api.yousendit.com");
    connect(mNetworkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotSendDataFinished(QNetworkReply*)));
}

YouSendItJob::~YouSendItJob()
{

}

void YouSendItJob::initializeToken(const QString &password, const QString &userName, const QString &token)
{
    mError = false;
    mPassword = password;
    mUsername = userName;
    mToken = token;
}

void YouSendItJob::createServiceFolder()
{
    qDebug()<<" not implemented";
    deleteLater();
}

void YouSendItJob::downloadFile(const QString &filename)
{
    qDebug()<<" not implemented";
    deleteLater();
}

void YouSendItJob::deleteFile(const QString &filename)
{
    qDebug()<<" not implemented";
    deleteLater();
}

void YouSendItJob::deleteFolder(const QString &foldername)
{
    qDebug()<<" not implemented";
    deleteLater();
}

void YouSendItJob::requestTokenAccess()
{
    QPointer<LoginDialog> dlg = new LoginDialog;
    dlg->setUsernameLabel(i18n("Email:"));
    if (dlg->exec()) {
        mPassword = dlg->password();
        mUsername = dlg->username();
    } else {
        Q_EMIT authorizationFailed(i18n("Authentication Canceled."));
        deleteLater();
        delete dlg;
        return;
    }
    delete dlg;

    mActionType = PimCommon::StorageServiceAbstract::RequestToken;
    mError = false;
    QUrl url(mDefaultUrl + QLatin1String("/dpi/v1/auth"));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    request.setRawHeader("X-Api-Key", mApiKey.toLatin1());
    request.setRawHeader("Accept", "application/json");

    QUrl postData;

    postData.addQueryItem(QLatin1String("email"), mUsername);
    postData.addQueryItem(QLatin1String("password"), mPassword);
    qDebug()<<" postData"<<postData;
    QNetworkReply *reply = mNetworkAccessManager->post(request, postData.encodedQuery());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void YouSendItJob::uploadFile(const QString &filename)
{
    //FIXME filename
    mActionType = PimCommon::StorageServiceAbstract::UploadFile;
    mError = false;
    QUrl url(mDefaultUrl + QLatin1String("/dpi/v1/folder/file/initUpload"));
    QNetworkRequest request(url);
    request.setRawHeader("X-Api-Key", mApiKey.toLatin1());
    request.setRawHeader("Accept", "application/json");
    request.setRawHeader("X-Auth-Token", mToken.toLatin1());
    QUrl postData;

    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    QNetworkReply *reply = mNetworkAccessManager->post(request, postData.encodedQuery());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void YouSendItJob::listFolder(const QString &folder)
{
    mActionType = PimCommon::StorageServiceAbstract::ListFolder;
    mError = false;
    //Show root folder => 0
    QUrl url;
    if (folder.isEmpty()) {
        url.setUrl(mDefaultUrl + QLatin1String("/dpi/v1/folder/0"));
    } else {
        url.setUrl(mDefaultUrl + QString::fromLatin1("/dpi/v1/folder/").arg(folder));
    }
    url.addQueryItem(QLatin1String("email"),mUsername);
    url.addQueryItem(QLatin1String("X-Auth-Token"), mToken);
    QNetworkRequest request(url);
    request.setRawHeader("X-Api-Key", mApiKey.toLatin1());
    request.setRawHeader("X-Auth-Token", mToken.toLatin1());
    request.setRawHeader("Accept", "application/json");
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void YouSendItJob::accountInfo()
{
    mActionType = PimCommon::StorageServiceAbstract::AccountInfo;
    mError = false;
    QUrl url(mDefaultUrl + QLatin1String("/dpi/v2/user"));
    url.addQueryItem(QLatin1String("email"),mUsername);
    url.addQueryItem(QLatin1String("X-Auth-Token"), mToken);
    QNetworkRequest request(url);
    request.setRawHeader("X-Api-Key", mApiKey.toLatin1());
    request.setRawHeader("X-Auth-Token", mToken.toLatin1());
    request.setRawHeader("Accept", "application/json");
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void YouSendItJob::createFolder(const QString &foldername)
{
    mActionType = PimCommon::StorageServiceAbstract::CreateFolder;
    mError = false;
    QUrl url(mDefaultUrl + QLatin1String("/dpi/v1/folder"));
    url.addQueryItem(QLatin1String("name"),foldername);
    QNetworkRequest request(url);
    request.setRawHeader("X-Api-Key", mApiKey.toLatin1());
    request.setRawHeader("Accept", "application/json");
    request.setRawHeader("X-Auth-Token", mToken.toLatin1());
    QUrl postData;

    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    QNetworkReply *reply = mNetworkAccessManager->post(request, postData.encodedQuery());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}


void YouSendItJob::slotSendDataFinished(QNetworkReply *reply)
{
    const QString data = QString::fromUtf8(reply->readAll());
    reply->deleteLater();
    if (mError) {
        qDebug()<<" error type "<<data;
        QJson::Parser parser;
        bool ok;
        QString errorStr;
        QMap<QString, QVariant> error = parser.parse(data.toUtf8(), &ok).toMap();
        if (error.contains(QLatin1String("errorStatus"))) {
            const QVariantMap storageMap = error.value(QLatin1String("errorStatus")).toMap();
            if (storageMap.contains(QLatin1String("message"))) {
                errorStr = storageMap.value(QLatin1String("message")).toString();
            }
        } else {
            errorStr = i18n("Unknown Error \"%1\"", data);
        }
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
        case PimCommon::StorageServiceAbstract::CreateServiceFolder:
        case PimCommon::StorageServiceAbstract::DeleteFile:
        case PimCommon::StorageServiceAbstract::DeleteFolder:
        case PimCommon::StorageServiceAbstract::DownLoadFile:
            errorMessage(mActionType, errorStr);
            deleteLater();
            break;
        default:
            qDebug()<<" Action Type unknown:"<<mActionType;
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
        deleteLater();
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
        //TODO
        deleteLater();
        break;
    default:
        qDebug()<<" Action Type unknown:"<<mActionType;
        deleteLater();
        break;
    }
}

void YouSendItJob::parseCreateServiceFolder(const QString &data)
{
    qDebug()<<" create service folder not implmented";
    deleteLater();
}

void YouSendItJob::parseListFolder(const QString &data)
{
    QJson::Parser parser;
    bool ok;
    QStringList listFolder;
    const QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    if (info.contains(QLatin1String("folders"))) {
        QVariantMap mapFolder = info.value(QLatin1String("folders")).toMap();
        QVariantList folders = mapFolder.value(QLatin1String("folder")).toList();
        Q_FOREACH (const QVariant &v, folders) {
            QVariantMap map = v.toMap();
            if (map.contains(QLatin1String("name"))) {
                listFolder.append(map.value(QLatin1String("name")).toString());
            }
        }
    }
    Q_EMIT listFolderDone(listFolder);
    deleteLater();
}


void YouSendItJob::parseRequestToken(const QString &data)
{
    QJson::Parser parser;
    bool ok;

    const QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    qDebug()<<" info"<<info;
    if (info.contains(QLatin1String("authToken"))) {
        const QString authToken = info.value(QLatin1String("authToken")).toString();
        Q_EMIT authorizationDone(mPassword, mUsername, authToken);
    } else {
        QString error;
        if (info.contains(QLatin1String("errorStatus"))) {
            QVariantMap map = info.value(QLatin1String("errorStatus")).toMap();
            if (map.contains(QLatin1String("message"))) {
                error = i18n("Authentication failed. Server returns this error:\n%1",map.value(QLatin1String("message")).toString());
            }
        }
        Q_EMIT authorizationFailed(error);
    }
    deleteLater();
}

void YouSendItJob::parseAccountInfo(const QString &data)
{
    QJson::Parser parser;
    bool ok;
    const QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    qDebug()<<" info"<<info;
    if (info.contains(QLatin1String("storage"))) {
        PimCommon::AccountInfo accountInfo;
        const QVariantMap storageMap = info.value(QLatin1String("storage")).toMap();
        if (storageMap.contains(QLatin1String("currentUsage"))) {
            accountInfo.shared = storageMap.value(QLatin1String("currentUsage")).toLongLong();
        }
        if (storageMap.contains(QLatin1String("storageQuota"))) {
            accountInfo.quota = storageMap.value(QLatin1String("storageQuota")).toLongLong();
        }
        Q_EMIT accountInfoDone(accountInfo);
    }
    deleteLater();
}

void YouSendItJob::parseCreateFolder(const QString &data)
{
    QJson::Parser parser;
    bool ok;
    const QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    qDebug()<<" info"<<info;
    QString newFolderName;
    if (info.contains(QLatin1String("name"))) {
        newFolderName = info.value(QLatin1String("name")).toString();
    }
    Q_EMIT createFolderDone(newFolderName);
    deleteLater();
}


void YouSendItJob::parseUploadFile(const QString &data)
{
    QJson::Parser parser;
    bool ok;
    qDebug()<<" data "<<data;
    const QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    qDebug()<<" info"<<info;
    QString fileId;
    if (info.contains(QLatin1String("fileId"))) {
        qDebug()<<" fileId "<<info.value(QLatin1String("fileId")).toString();
        fileId = info.value(QLatin1String("fileId")).toString();
    }
    startUploadFile(fileId);
}

void YouSendItJob::startUploadFile(const QString &fileId)
{
    mActionType = PimCommon::StorageServiceAbstract::UploadFile;
    mError = false;
    QUrl url(mDefaultUrl + QLatin1String("/dpi/v1/folder/file/initUpload"));
    QNetworkRequest request(url);
    request.setRawHeader("bid", fileId.toLatin1());
    request.setRawHeader("filename", "test.txt");
    request.setRawHeader("Accept", "application/json");
    request.setRawHeader("X-Auth-Token", mToken.toLatin1());
    QUrl postData;
    postData.addQueryItem(QLatin1String("fileId"), fileId);

    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    QNetworkReply *reply = mNetworkAccessManager->post(request, postData.encodedQuery());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
//Multipart
    //TODO
    //deleteLater();
}

void YouSendItJob::shareLink(const QString &root, const QString &path)
{
    mError = false;
    mActionType = PimCommon::StorageServiceAbstract::ShareLink;
    qDebug()<<" not implemented";
    deleteLater();
}
