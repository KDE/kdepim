/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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
#include "storageservice/authdialog/logindialog.h"
#include "storageservice/storageserviceabstract.h"
#include "storageservice/storageservicejobconfig.h"

#include <KLocalizedString>

#include "pimcommon_debug.h"
#include <QJsonParseError>
#include <QNetworkReply>
#include <QPointer>

using namespace PimCommon;

YouSendItJob::YouSendItJob(QObject *parent)
    : PimCommon::StorageServiceAbstractJob(parent)
{
    mApiKey = PimCommon::StorageServiceJobConfig::self()->youSendItApiKey();
    mDefaultUrl = QStringLiteral("https://test2-api.yousendit.com");
    connect(mNetworkAccessManager, &QNetworkAccessManager::finished, this, &YouSendItJob::slotSendDataFinished);
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

void YouSendItJob::copyFile(const QString &/*source*/, const QString &/*destination*/)
{
    mActionType = PimCommon::StorageServiceAbstract::CopyFileAction;
    mError = false;
    qCDebug(PIMCOMMON_LOG) << " not implemented";
    Q_EMIT actionFailed(QStringLiteral("Not Implemented"));
    deleteLater();
}

void YouSendItJob::copyFolder(const QString &/*source*/, const QString &/*destination*/)
{
    mActionType = PimCommon::StorageServiceAbstract::CopyFolderAction;
    mError = false;
    qCDebug(PIMCOMMON_LOG) << " not implemented";
    Q_EMIT actionFailed(QStringLiteral("Not Implemented"));
    deleteLater();
}

void YouSendItJob::createServiceFolder()
{
    mActionType = PimCommon::StorageServiceAbstract::CreateServiceFolderAction;
    mError = false;
    createFolderJob(PimCommon::StorageServiceJobConfig::self()->defaultUploadFolder(), QString());
}

QNetworkReply *YouSendItJob::downloadFile(const QString &name, const QString &fileId, const QString &destination)
{
    qCDebug(PIMCOMMON_LOG) << " not implemented";
    Q_EMIT actionFailed(QStringLiteral("Not Implemented"));
    deleteLater();
    return Q_NULLPTR;
}

QNetworkRequest YouSendItJob::setDefaultHeader(const QUrl &url)
{
    QNetworkRequest request(url);
    request.setRawHeader("X-Api-Key", mApiKey.toLatin1());
    request.setRawHeader("Accept", "application/json");
    request.setRawHeader("X-Auth-Token", mToken.toLatin1());
    return request;
}

void YouSendItJob::deleteFile(const QString &filename)
{
    mActionType = PimCommon::StorageServiceAbstract::DeleteFileAction;
    mError = false;
    QUrl url(mDefaultUrl + QStringLiteral("/dpi/v1/folder/file/%1").arg(filename));
    qCDebug(PIMCOMMON_LOG) << " url" << url;
    QNetworkRequest request = setDefaultHeader(url);
    QNetworkReply *reply = mNetworkAccessManager->deleteResource(request);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &YouSendItJob::slotError);
}

void YouSendItJob::deleteFolder(const QString &foldername)
{
    mActionType = PimCommon::StorageServiceAbstract::DeleteFolderAction;
    mError = false;
    QUrl url(mDefaultUrl + QStringLiteral("/dpi/v1/folder/%1").arg(foldername));
    QNetworkRequest request = setDefaultHeader(url);
    QNetworkReply *reply = mNetworkAccessManager->deleteResource(request);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &YouSendItJob::slotError);
}

void YouSendItJob::renameFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::RenameFolderAction;
    mError = false;
    QUrl url(mDefaultUrl + QStringLiteral("/dpi/v1/folder/%1/rename?name=%2").arg(source).arg(destination));
    QNetworkRequest request = setDefaultHeader(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));
    QUrl postData;
    postData.addQueryItem(QStringLiteral("name"), destination);

    QNetworkReply *reply = mNetworkAccessManager->put(request, postData.encodedQuery());
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &YouSendItJob::slotError);
}

void YouSendItJob::renameFile(const QString &oldName, const QString &newName)
{
    mActionType = PimCommon::StorageServiceAbstract::RenameFileAction;
    mError = false;
    QUrl url(mDefaultUrl + QStringLiteral("/dpi/v1/folder/file/%1/rename?name=%2").arg(oldName).arg(newName));
    QNetworkRequest request = setDefaultHeader(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));

    QUrl postData;
    postData.addQueryItem(QStringLiteral("name"), newName);
    QNetworkReply *reply = mNetworkAccessManager->put(request, postData.encodedQuery());
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &YouSendItJob::slotError);
}

void YouSendItJob::moveFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::MoveFolderAction;
    mError = false;
    QUrl url(mDefaultUrl + QStringLiteral("/dpi/v1/folder/%1/move?parentId=%2").arg(source).arg(destination));
    QNetworkRequest request = setDefaultHeader(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));

    QUrl postData;
    postData.addQueryItem(QStringLiteral("parentId"), destination);
    QNetworkReply *reply = mNetworkAccessManager->put(request, postData.encodedQuery());
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &YouSendItJob::slotError);
}

void YouSendItJob::moveFile(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::MoveFileAction;
    mError = false;
    QUrl url(mDefaultUrl + QStringLiteral("/dpi/v1/folder/file/%1/move?parentId=%2").arg(source).arg(destination));
    QNetworkRequest request = setDefaultHeader(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));

    QUrl postData;
    postData.addQueryItem(QStringLiteral("parentId"), destination);
    QNetworkReply *reply = mNetworkAccessManager->put(request, postData.encodedQuery());
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &YouSendItJob::slotError);
}

void YouSendItJob::requestTokenAccess()
{
    QPointer<LoginDialog> dlg = new LoginDialog;
    dlg->setWindowTitle(i18n("YouSendIt"));
    dlg->setUsernameLabel(i18n("Email:"));
    if (dlg->exec()) {
        mPassword = dlg->password();
        mUsername = dlg->username();
    } else {
        Q_EMIT authorizationCancelled();
        deleteLater();
        delete dlg;
        return;
    }
    delete dlg;

    mActionType = PimCommon::StorageServiceAbstract::RequestTokenAction;
    mError = false;
    QUrl url(mDefaultUrl + QLatin1String("/dpi/v1/auth"));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));
    request.setRawHeader("X-Api-Key", mApiKey.toLatin1());
    request.setRawHeader("Accept", "application/json");

    QUrl postData;

    postData.addQueryItem(QStringLiteral("email"), mUsername);
    postData.addQueryItem(QStringLiteral("password"), mPassword);
    //qCDebug(PIMCOMMON_LOG)<<" postData"<<postData;
    QNetworkReply *reply = mNetworkAccessManager->post(request, postData.encodedQuery());
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &YouSendItJob::slotError);
}

QNetworkReply *YouSendItJob::uploadFile(const QString &filename, const QString &uploadAsName, const QString &destination)
{
    //FIXME filename
    mActionType = PimCommon::StorageServiceAbstract::UploadFileAction;
    mError = false;
    QUrl url(mDefaultUrl + QLatin1String("/dpi/v1/folder/file/initUpload"));
    QNetworkRequest request = setDefaultHeader(url);
    QUrl postData;

    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));
    QNetworkReply *reply = mNetworkAccessManager->post(request, postData.encodedQuery());
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &YouSendItJob::slotError);
    return reply;
}

void YouSendItJob::listFolder(const QString &folder)
{
    mActionType = PimCommon::StorageServiceAbstract::ListFolderAction;
    mError = false;
    //Show root folder => 0
    QUrl url;
    if (folder.isEmpty()) {
        url.setUrl(mDefaultUrl + QLatin1String("/dpi/v1/folder/0"));
    } else {
        url.setUrl(mDefaultUrl + QStringLiteral("/dpi/v1/folder/%1").arg(folder));
    }
    url.addQueryItem(QStringLiteral("email"), mUsername);
    url.addQueryItem(QStringLiteral("X-Auth-Token"), mToken);
    QNetworkRequest request = setDefaultHeader(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &YouSendItJob::slotError);
}

void YouSendItJob::accountInfo()
{
    mActionType = PimCommon::StorageServiceAbstract::AccountInfoAction;
    mError = false;
    QUrl url(mDefaultUrl + QLatin1String("/dpi/v2/user"));
    url.addQueryItem(QStringLiteral("email"), mUsername);
    url.addQueryItem(QStringLiteral("X-Auth-Token"), mToken);
    QNetworkRequest request = setDefaultHeader(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &YouSendItJob::slotError);
}

void YouSendItJob::createFolder(const QString &foldername, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::CreateFolderAction;
    mError = false;
    createFolderJob(foldername, destination);
}

void YouSendItJob::createFolderJob(const QString &foldername, const QString &destination)
{
    QUrl url(mDefaultUrl + QLatin1String("/dpi/v1/folder"));
    QNetworkRequest request = setDefaultHeader(url);
    QUrl postData;
    postData.addQueryItem(QStringLiteral("name"), foldername);
    if (!destination.isEmpty()) {
        postData.addQueryItem(QStringLiteral("parentId"), destination);
    }
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));
    QNetworkReply *reply = mNetworkAccessManager->post(request, postData.encodedQuery());
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &YouSendItJob::slotError);
}

void YouSendItJob::slotSendDataFinished(QNetworkReply *reply)
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
        QString errorStr;
        if (error.contains(QStringLiteral("errorStatus"))) {
            const QVariantMap storageMap = error.value(QStringLiteral("errorStatus")).toMap();
            if (storageMap.contains(QStringLiteral("message"))) {
                errorStr = storageMap.value(QStringLiteral("message")).toString();
            }
        } else {
            errorStr = i18n("Unknown Error \"%1\"", data);
        }
        switch (mActionType) {
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
        case PimCommon::StorageServiceAbstract::DeleteFolderAction:
        case PimCommon::StorageServiceAbstract::CreateFolderAction:
        case PimCommon::StorageServiceAbstract::AccountInfoAction:
        case PimCommon::StorageServiceAbstract::ListFolderAction:
        case PimCommon::StorageServiceAbstract::CreateServiceFolderAction:
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
        return;
    }
    switch (mActionType) {
    case PimCommon::StorageServiceAbstract::NoneAction:
        deleteLater();
        break;
    case PimCommon::StorageServiceAbstract::RequestTokenAction:
        parseRequestToken(data);
        break;
    case PimCommon::StorageServiceAbstract::AccessTokenAction:
        deleteLater();
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
    case PimCommon::StorageServiceAbstract::DeleteFolderAction:
        parseDeleteFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::DeleteFileAction:
        parseDeleteFile(data);
        break;
    case PimCommon::StorageServiceAbstract::DownLoadFileAction:
        parseDownloadFile(data);
        break;
    case PimCommon::StorageServiceAbstract::RenameFolderAction:
        parseRenameFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::RenameFileAction:
        parseRenameFile(data);
        break;
    case PimCommon::StorageServiceAbstract::MoveFolderAction:
        parseMoveFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::MoveFileAction:
        parseMoveFile(data);
        break;
    case PimCommon::StorageServiceAbstract::CopyFileAction:
        parseCopyFile(data);
        break;
    case PimCommon::StorageServiceAbstract::CopyFolderAction:
        parseCopyFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::ShareLinkAction:
        parseShareLink(data);
        break;
    }
}

void YouSendItJob::parseShareLink(const QString &data)
{
    QJsonParseError parsingError;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(data.toUtf8(), &parsingError);
    if (parsingError.error != QJsonParseError::NoError || jsonDoc.isNull()) {
        return;
    }
    const QMap<QString, QVariant> info = jsonDoc.toVariant().toMap();

    if (!parseError(info)) {
        //TODO
        Q_EMIT shareLinkDone(QString());
    }
    deleteLater();
}

void YouSendItJob::parseCopyFolder(const QString &data)
{
    QJsonParseError parsingError;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(data.toUtf8(), &parsingError);
    if (parsingError.error != QJsonParseError::NoError || jsonDoc.isNull()) {
        qCDebug(PIMCOMMON_LOG) << " parseCopyFile error " << data;
        return;
    }
    const QMap<QString, QVariant> info = jsonDoc.toVariant().toMap();
    if (!parseError(info)) {
        Q_EMIT copyFolderDone(QString());
    }
    deleteLater();
}

void YouSendItJob::parseCopyFile(const QString &data)
{
    QJsonParseError parsingError;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(data.toUtf8(), &parsingError);
    if (parsingError.error != QJsonParseError::NoError || jsonDoc.isNull()) {
        qCDebug(PIMCOMMON_LOG) << " parseCopyFile error " << data;
        return;
    }
    const QMap<QString, QVariant> info = jsonDoc.toVariant().toMap();
    if (!parseError(info)) {
        Q_EMIT copyFileDone(QString());
    }
    deleteLater();
}

void YouSendItJob::parseMoveFolder(const QString &data)
{
    QJsonParseError parsingError;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(data.toUtf8(), &parsingError);
    if (parsingError.error != QJsonParseError::NoError || jsonDoc.isNull()) {
        return;
    }
    const QMap<QString, QVariant> info = jsonDoc.toVariant().toMap();

    if (!parseError(info)) {
        Q_EMIT moveFolderDone(QString());
    }
    deleteLater();
}

void YouSendItJob::parseMoveFile(const QString &data)
{
    QJsonParseError parsingError;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(data.toUtf8(), &parsingError);
    if (parsingError.error != QJsonParseError::NoError || jsonDoc.isNull()) {
        return;
    }
    const QMap<QString, QVariant> info = jsonDoc.toVariant().toMap();
    if (!parseError(info)) {
        Q_EMIT moveFileDone(QString());
    }
    deleteLater();
}

bool YouSendItJob::parseError(const QMap<QString, QVariant> &info)
{
    qCDebug(PIMCOMMON_LOG) << " info" << info;
    if (info.contains(QStringLiteral("errorStatus"))) {
        const QVariantMap map = info.value(QStringLiteral("errorStatus")).toMap();
        if (map.contains(QStringLiteral("message"))) {
            Q_EMIT actionFailed(map.value(QStringLiteral("message")).toString());
            return true;
        }
    }
    return false;
}

void YouSendItJob::parseRenameFile(const QString &data)
{
    QJsonParseError parsingError;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(data.toUtf8(), &parsingError);
    if (parsingError.error != QJsonParseError::NoError || jsonDoc.isNull()) {
        return;
    }
    const QMap<QString, QVariant> info = jsonDoc.toVariant().toMap();
    if (!parseError(info)) {
        Q_EMIT renameFileDone(QString());
    }
    deleteLater();
}

void YouSendItJob::parseRenameFolder(const QString &data)
{
    QJsonParseError parsingError;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(data.toUtf8(), &parsingError);
    if (parsingError.error != QJsonParseError::NoError || jsonDoc.isNull()) {
        return;
    }
    const QMap<QString, QVariant> info = jsonDoc.toVariant().toMap();
    if (!parseError(info)) {
        Q_EMIT renameFolderDone(QString());
    }
    deleteLater();
}

void YouSendItJob::parseDownloadFile(const QString &data)
{
    qCDebug(PIMCOMMON_LOG) << " data :" << data;
    Q_EMIT actionFailed(QStringLiteral("Not Implemented"));
    //Q_EMIT downLoadFileDone(filename);
    //TODO
    deleteLater();
}

void YouSendItJob::parseDeleteFile(const QString &data)
{
    Q_EMIT deleteFileDone(QString());
    deleteLater();
}

void YouSendItJob::parseDeleteFolder(const QString &data)
{
    Q_UNUSED(data);
    //qCDebug(PIMCOMMON_LOG)<<" data "<<data;
    //Api doesn't return folder name.
    Q_EMIT deleteFolderDone(QString());
    deleteLater();
}

void YouSendItJob::parseCreateServiceFolder(const QString &data)
{
    qCDebug(PIMCOMMON_LOG) << " create service folder not implmented";
    Q_EMIT actionFailed(QStringLiteral("Not Implemented"));
    deleteLater();
}

void YouSendItJob::parseListFolder(const QString &data)
{
    Q_EMIT listFolderDone(data);
    deleteLater();
}

void YouSendItJob::parseRequestToken(const QString &data)
{
    QJsonParseError parsingError;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(data.toUtf8(), &parsingError);
    if (parsingError.error != QJsonParseError::NoError || jsonDoc.isNull()) {
        qCDebug(PIMCOMMON_LOG) << " parseRequestToken error" << data;
        return;
    }
    const QMap<QString, QVariant> info = jsonDoc.toVariant().toMap();
    qCDebug(PIMCOMMON_LOG) << " info" << info;
    if (info.contains(QStringLiteral("authToken"))) {
        const QString authToken = info.value(QStringLiteral("authToken")).toString();
        Q_EMIT authorizationDone(mPassword, mUsername, authToken);
    } else {
        QString error;
        if (info.contains(QStringLiteral("errorStatus"))) {
            QVariantMap map = info.value(QStringLiteral("errorStatus")).toMap();
            if (map.contains(QStringLiteral("message"))) {
                error = i18n("Authentication failed. Server returns this error:\n%1", map.value(QStringLiteral("message")).toString());
            }
        }
        Q_EMIT authorizationFailed(error);
    }
    deleteLater();
}

void YouSendItJob::parseAccountInfo(const QString &data)
{
    QJsonParseError parsingError;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(data.toUtf8(), &parsingError);
    if (parsingError.error != QJsonParseError::NoError || jsonDoc.isNull()) {
        return;
    }
    const QMap<QString, QVariant> info = jsonDoc.toVariant().toMap();
    qCDebug(PIMCOMMON_LOG) << " info" << info;
    if (info.contains(QStringLiteral("storage"))) {
        PimCommon::AccountInfo accountInfo;
        const QVariantMap storageMap = info.value(QStringLiteral("storage")).toMap();
        if (storageMap.contains(QStringLiteral("currentUsage"))) {
            accountInfo.shared = storageMap.value(QStringLiteral("currentUsage")).toLongLong();
        }
        if (storageMap.contains(QStringLiteral("storageQuota"))) {
            accountInfo.quota = storageMap.value(QStringLiteral("storageQuota")).toLongLong();
        }
        Q_EMIT accountInfoDone(accountInfo);
    }
    deleteLater();
}

void YouSendItJob::parseCreateFolder(const QString &data)
{
    QJsonParseError parsingError;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(data.toUtf8(), &parsingError);
    if (parsingError.error != QJsonParseError::NoError || jsonDoc.isNull()) {
        return;
    }
    const QMap<QString, QVariant> info = jsonDoc.toVariant().toMap();
    QString newFolderName;
    if (info.contains(QStringLiteral("name"))) {
        newFolderName = info.value(QStringLiteral("name")).toString();
    }
    Q_EMIT createFolderDone(newFolderName);
    deleteLater();
}

void YouSendItJob::parseUploadFile(const QString &data)
{
    QJsonParseError parsingError;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(data.toUtf8(), &parsingError);
    if (parsingError.error != QJsonParseError::NoError || jsonDoc.isNull()) {
        return;
    }
    const QMap<QString, QVariant> info = jsonDoc.toVariant().toMap();
    qCDebug(PIMCOMMON_LOG) << " data " << data;
    qCDebug(PIMCOMMON_LOG) << " info" << info;
    QString fileId;
    if (info.contains(QStringLiteral("fileId"))) {
        qCDebug(PIMCOMMON_LOG) << " fileId " << info.value(QStringLiteral("fileId")).toString();
        fileId = info.value(QStringLiteral("fileId")).toString();
    }
    startUploadFile(fileId);
}

void YouSendItJob::startUploadFile(const QString &fileId)
{
    mActionType = PimCommon::StorageServiceAbstract::UploadFileAction;
    mError = false;
    QUrl url(mDefaultUrl + QLatin1String("/dpi/v1/folder/file/initUpload"));
    QNetworkRequest request(url);
    request.setRawHeader("bid", fileId.toLatin1());
    request.setRawHeader("filename", "test.txt");
    request.setRawHeader("Accept", "application/json");
    request.setRawHeader("X-Auth-Token", mToken.toLatin1());
    QUrl postData;
    postData.addQueryItem(QStringLiteral("fileId"), fileId);

    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));
    QNetworkReply *reply = mNetworkAccessManager->post(request, postData.encodedQuery());
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &YouSendItJob::slotError);
    //Multipart
    //TODO
    //deleteLater();
}

void YouSendItJob::shareLink(const QString &root, const QString &path)
{
    mError = false;
    mActionType = PimCommon::StorageServiceAbstract::ShareLinkAction;
    Q_EMIT actionFailed(QStringLiteral("Not Implemented"));
    qCDebug(PIMCOMMON_LOG) << " not implemented";
    deleteLater();
}

