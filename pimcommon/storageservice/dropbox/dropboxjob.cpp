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


#include "dropboxjob.h"
#include "storageservice/authdialog/storageauthviewdialog.h"
#include "storageservice/storageserviceabstract.h"
#include "storageservice/utils/storageserviceutils.h"
#include "pimcommon/storageservice/storageservicejobconfig.h"

#include <KLocalizedString>
#include <KUrl>

#include <QFile>

#include <QNetworkRequest>
#include <QDateTime>
#include <QStringList>
#include <QDebug>
#include <QPointer>
#include <QFile>

using namespace PimCommon;

DropBoxJob::DropBoxJob(QObject *parent)
    : PimCommon::StorageServiceAbstractJob(parent)
{
    mApiPath = QLatin1String("https://api.dropbox.com/1/");
    mOauthconsumerKey = PimCommon::StorageServiceJobConfig::self()->dropboxOauthConsumerKey();
    mOauthSignature = PimCommon::StorageServiceJobConfig::self()->dropboxOauthSignature();
    mRootPath = PimCommon::StorageServiceJobConfig::self()->dropboxRootPath();
    mOauthVersion = QLatin1String("1.0");
    mOauthSignatureMethod = QLatin1String("PLAINTEXT");
    mTimestamp = QString::number(QDateTime::currentMSecsSinceEpoch()/1000);
    mNonce = PimCommon::StorageServiceUtils::generateNonce(8);
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
}

void DropBoxJob::requestTokenAccess()
{
    mActionType = PimCommon::StorageServiceAbstract::RequestTokenAction;
    mError = false;
    QNetworkRequest request(QUrl(mApiPath + QLatin1String("oauth/request_token")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));

    QUrl postData;

    postData.addQueryItem(QLatin1String("oauth_consumer_key"), mOauthconsumerKey);
    postData.addQueryItem(QLatin1String("oauth_nonce"), mNonce);
    postData.addQueryItem(QLatin1String("oauth_signature"), mOauthSignature);
    postData.addQueryItem(QLatin1String("oauth_signature_method"),mOauthSignatureMethod);
    postData.addQueryItem(QLatin1String("oauth_timestamp"), mTimestamp);
    postData.addQueryItem(QLatin1String("oauth_version"), mOauthVersion);

    QNetworkReply *reply = mNetworkAccessManager->post(request, postData.encodedQuery());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void DropBoxJob::getTokenAccess()
{
    mActionType = PimCommon::StorageServiceAbstract::AccessTokenAction;
    mError = false;
    QNetworkRequest request(QUrl(mApiPath + QLatin1String("oauth/access_token")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));

    QUrl postData;
    postData.addQueryItem(QLatin1String("oauth_consumer_key"), mOauthconsumerKey);
    postData.addQueryItem(QLatin1String("oauth_nonce"), mNonce);
    postData.addQueryItem(QLatin1String("oauth_signature"), mAccessOauthSignature);
    postData.addQueryItem(QLatin1String("oauth_signature_method"),mOauthSignatureMethod);
    postData.addQueryItem(QLatin1String("oauth_timestamp"), mTimestamp);
    postData.addQueryItem(QLatin1String("oauth_version"), mOauthVersion);
    postData.addQueryItem(QLatin1String("oauth_token"), mOauthToken);

    QNetworkReply *reply = mNetworkAccessManager->post(request, postData.encodedQuery());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void DropBoxJob::slotSendDataFinished(QNetworkReply *reply)
{
#if 0 //QT5
    const QString data = QString::fromUtf8(reply->readAll());
    reply->deleteLater();
    if (mError) {
        QJson::Parser parser;
        bool ok;

        QMap<QString, QVariant> error = parser.parse(data.toUtf8(), &ok).toMap();
        if (error.contains(QLatin1String("error"))) {
            const QString errorStr = error.value(QLatin1String("error")).toString();
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
            case PimCommon::StorageServiceAbstract::CreateFolderAction:
            case PimCommon::StorageServiceAbstract::AccountInfoAction:
            case PimCommon::StorageServiceAbstract::ListFolderAction:
            case PimCommon::StorageServiceAbstract::ShareLinkAction:
            case PimCommon::StorageServiceAbstract::CreateServiceFolderAction:
            case PimCommon::StorageServiceAbstract::DeleteFileAction:
            case PimCommon::StorageServiceAbstract::DeleteFolderAction:
            case PimCommon::StorageServiceAbstract::RenameFolderAction:
            case PimCommon::StorageServiceAbstract::RenameFileAction:
            case PimCommon::StorageServiceAbstract::MoveFolderAction:
            case PimCommon::StorageServiceAbstract::MoveFileAction:
            case PimCommon::StorageServiceAbstract::CopyFileAction:
            case PimCommon::StorageServiceAbstract::CopyFolderAction:
                errorMessage(mActionType, errorStr);
                deleteLater();
                break;
            }
        } else {
            errorMessage(mActionType, i18n("Unknown Error \"%1\"", data));
            deleteLater();
        }
        return;
    }
    switch(mActionType) {
    case PimCommon::StorageServiceAbstract::NoneAction:
        break;
    case PimCommon::StorageServiceAbstract::RequestTokenAction:
        parseRequestToken(data);
        break;
    case PimCommon::StorageServiceAbstract::AccessTokenAction:
        parseResponseAccessToken(data);
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
    case PimCommon::StorageServiceAbstract::ShareLinkAction:
        parseShareLink(data);
        break;
    case PimCommon::StorageServiceAbstract::CreateServiceFolderAction:
        deleteLater();
        break;
    case PimCommon::StorageServiceAbstract::DeleteFileAction:
        parseDeleteFile(data);
        break;
    case PimCommon::StorageServiceAbstract::DeleteFolderAction:
        parseDeleteFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::DownLoadFileAction:
        parseDownLoadFile(data);
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
    }
#endif
}

QString DropBoxJob::extractPathFromData(const QString &data)
{
    QString name;
#if 0 //QT5
    QJson::Parser parser;
    bool ok;
    QString name;
    QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    if (info.contains(QLatin1String("path"))) {
        name = info.value(QLatin1String("path")).toString();
    }
#endif
    return name;
}

void DropBoxJob::parseCopyFile(const QString &data)
{
    //qDebug()<<" data :"<<data;
    const QString name = extractPathFromData(data);
    Q_EMIT copyFileDone(name);
    deleteLater();
}

void DropBoxJob::parseCopyFolder(const QString &data)
{
    //qDebug()<<" data :"<<data;
    const QString name = extractPathFromData(data);
    Q_EMIT copyFolderDone(name);
    deleteLater();
}

void DropBoxJob::parseMoveFolder(const QString &data)
{
    //qDebug()<<" data :"<<data;
    const QString name = extractPathFromData(data);
    Q_EMIT moveFolderDone(name);
    deleteLater();
}

void DropBoxJob::parseMoveFile(const QString &data)
{
    //qDebug()<<" data :"<<data;
    const QString name = extractPathFromData(data);
    Q_EMIT moveFileDone(name);
    deleteLater();
}

void DropBoxJob::parseRenameFile(const QString &data)
{
    //qDebug()<<" data :"<<data;
    const QString name = extractPathFromData(data);
    Q_EMIT renameFileDone(name);
    deleteLater();
}

void DropBoxJob::parseRenameFolder(const QString &data)
{
    //qDebug()<<" data :"<<data;
    const QString name = extractPathFromData(data);
    Q_EMIT renameFolderDone(name);
    deleteLater();
}

void DropBoxJob::parseDeleteFolder(const QString &data)
{
    const QString name = extractPathFromData(data);
    Q_EMIT deleteFolderDone(name);
    deleteLater();
}

void DropBoxJob::parseDeleteFile(const QString &data)
{
    const QString name = extractPathFromData(data);
    Q_EMIT deleteFileDone(name);
    deleteLater();
}

void DropBoxJob::parseAccountInfo(const QString &data)
{
#if 0 //QT5
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
#endif
    deleteLater();
}

void DropBoxJob::parseResponseAccessToken(const QString &data)
{
    if(data.contains(QLatin1String("error"))) {
        Q_EMIT authorizationFailed(data);
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

        //qDebug()<<" mOauthToken" <<mOauthToken<<"mAccessOauthSignature "<<mAccessOauthSignature<<" mOauthSignature"<<mOauthSignature;

    } else {
        qDebug()<<" data is not good: "<<result;
    }
    doAuthentication();
}

void DropBoxJob::doAuthentication()
{
    QUrl url(mApiPath + QLatin1String("oauth/authorize"));
    url.addQueryItem(QLatin1String("oauth_token"), mOauthToken);
    QPointer<StorageAuthViewDialog> dlg = new StorageAuthViewDialog;
    dlg->setUrl(url);
    if (dlg->exec()) {
        getTokenAccess();
        delete dlg;
    } else {
        Q_EMIT authorizationFailed(i18n("Authentication Canceled."));
        delete dlg;
        deleteLater();
    }
}

void DropBoxJob::createFolderJob(const QString &foldername, const QString &destination)
{
    if (foldername.isEmpty()) {
        qDebug()<<" folder empty!";
    }
    QUrl url(mApiPath + QLatin1String("fileops/create_folder"));
    url.addQueryItem(QLatin1String("root"), mRootPath);
    url.addQueryItem(QLatin1String("path"), destination + QLatin1Char('/') + foldername );
    addDefaultUrlItem(url);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void DropBoxJob::createFolder(const QString &foldername, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::CreateFolderAction;
    mError = false;
    createFolderJob(foldername, destination);
}

QNetworkReply *DropBoxJob::uploadFile(const QString &filename, const QString &uploadAsName, const QString &destination)
{
    QFile *file = new QFile(filename);
    if (file->exists()) {
        mActionType = PimCommon::StorageServiceAbstract::UploadFileAction;
        mError = false;
        if (file->open(QIODevice::ReadOnly)) {
            const QString defaultDestination = (destination.isEmpty() ? PimCommon::StorageServiceJobConfig::self()->defaultUploadFolder() : destination);
            const QString r = mAccessOauthSignature.replace(QLatin1Char('&'),QLatin1String("%26"));
            const QString str = QString::fromLatin1("https://api-content.dropbox.com/1/files_put/dropbox///%7/%1?oauth_consumer_key=%2&oauth_nonce=%3&oauth_signature=%4&oauth_signature_method=PLAINTEXT&oauth_timestamp=%6&oauth_version=1.0&oauth_token=%5&overwrite=false").
                    arg(uploadAsName).arg(mOauthconsumerKey).arg(mNonce).arg(r).arg(mOauthToken).arg(mTimestamp).arg(defaultDestination);
            KUrl url(str);
            QNetworkRequest request(url);
            QNetworkReply *reply = mNetworkAccessManager->put(request, file);
            file->setParent(reply);
            connect(reply, SIGNAL(uploadProgress(qint64,qint64)), SLOT(slotuploadDownloadFileProgress(qint64,qint64)));
            connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
            return reply;
        }
    }
    delete file;
    return 0;
}

void DropBoxJob::accountInfo()
{
    mActionType = PimCommon::StorageServiceAbstract::AccountInfoAction;
    mError = false;
    QUrl url(mApiPath + QLatin1String("account/info"));
    addDefaultUrlItem(url);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));

    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void DropBoxJob::listFolder(const QString &folder)
{
    mActionType = PimCommon::StorageServiceAbstract::ListFolderAction;
    mError = false;
    QUrl url(mApiPath + QLatin1String("metadata/dropbox/") + folder);
    addDefaultUrlItem(url);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));

    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void DropBoxJob::parseUploadFile(const QString &data)
{
#if 0 //QT5
    QJson::Parser parser;
    bool ok;

    QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    QString root;
    QString path;
    if (info.contains(QLatin1String("root"))) {
        root = info.value(QLatin1String("root")).toString();
    }
    if (info.contains(QLatin1String("path"))) {
        path = info.value(QLatin1String("path")).toString();
    }
    QString itemName;
    if (!path.isEmpty()) {
        itemName = path.right((path.length() - path.lastIndexOf(QLatin1Char('/'))) - 1);
    }
    Q_EMIT uploadFileDone(itemName);
    shareLink(root, path);
#endif
}

void DropBoxJob::shareLink(const QString &root, const QString &path)
{
    mActionType = PimCommon::StorageServiceAbstract::ShareLinkAction;
    mError = false;

    QUrl url = QUrl(mApiPath + QString::fromLatin1("shares/%1/%2").arg(root).arg(path));
    url.addQueryItem(QLatin1String("root"), mRootPath);
    addDefaultUrlItem(url);
    QNetworkRequest request(url);

    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void DropBoxJob::createServiceFolder()
{
    mActionType = PimCommon::StorageServiceAbstract::CreateServiceFolderAction;
    mError = false;
    createFolderJob(PimCommon::StorageServiceJobConfig::self()->defaultUploadFolder(), QString());
}

QNetworkReply *DropBoxJob::downloadFile(const QString &name, const QString &fileId, const QString &destination)
{
    Q_UNUSED(fileId);
    mActionType = PimCommon::StorageServiceAbstract::DownLoadFileAction;
    mError = false;
    const QString defaultDestination = (destination.isEmpty() ? PimCommon::StorageServiceJobConfig::self()->defaultUploadFolder() : destination);
    delete mDownloadFile;
    mDownloadFile = new QFile(defaultDestination+ QLatin1Char('/') + name);
    if (mDownloadFile->open(QIODevice::WriteOnly)) {
        const QString r = mAccessOauthSignature.replace(QLatin1Char('&'),QLatin1String("%26"));
        const QString str = QString::fromLatin1("https://api-content.dropbox.com/1/files/dropbox///%1?oauth_consumer_key=%2&oauth_nonce=%3&oauth_signature=%4&oauth_signature_method=PLAINTEXT&oauth_timestamp=%6&oauth_version=1.0&oauth_token=%5").
                arg(name).arg(mOauthconsumerKey).arg(mNonce).arg(r).arg(mOauthToken).arg(mTimestamp);
        KUrl url(str);
        QNetworkRequest request(url);
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

void DropBoxJob::deleteFile(const QString &filename)
{
    mActionType = PimCommon::StorageServiceAbstract::DeleteFileAction;
    mError = false;
    QUrl url = QUrl(mApiPath + QLatin1String("fileops/delete"));
    url.addQueryItem(QLatin1String("root"), mRootPath);
    url.addQueryItem(QLatin1String("path"), filename);
    addDefaultUrlItem(url);
    QNetworkRequest request(url);
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void DropBoxJob::deleteFolder(const QString &foldername)
{
    mActionType = PimCommon::StorageServiceAbstract::DeleteFolderAction;
    mError = false;
    QUrl url = QUrl(mApiPath + QLatin1String("fileops/delete"));
    url.addQueryItem(QLatin1String("root"), mRootPath);
    url.addQueryItem(QLatin1String("path"), foldername);
    addDefaultUrlItem(url);
    QNetworkRequest request(url);
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void DropBoxJob::renameFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::RenameFolderAction;
    mError = false;
    QUrl url = QUrl(mApiPath + QLatin1String("fileops/move"));
    url.addQueryItem(QLatin1String("root"), mRootPath);
    addDefaultUrlItem(url);
    url.addQueryItem(QLatin1String("from_path"), source);

    QStringList parts = source.split(QLatin1String("/"), QString::SkipEmptyParts);
    parts.removeLast();
    QString destinationFolder = parts.join(QLatin1String("/"));
    if (destinationFolder.isEmpty()) {
        destinationFolder = QLatin1String("/");
    }
    if (!destinationFolder.endsWith(QLatin1Char('/'))) {
        destinationFolder += QLatin1String("/");
    }
    if (!destinationFolder.startsWith(QLatin1Char('/'))) {
        destinationFolder.prepend(QLatin1String("/"));
    }

    const QString newPath = destinationFolder.append(destination);
    url.addQueryItem(QLatin1String("to_path"), newPath);

    QNetworkRequest request(url);
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void DropBoxJob::renameFile(const QString &oldName, const QString &newName)
{
    mActionType = PimCommon::StorageServiceAbstract::RenameFileAction;
    mError = false;
    QUrl url = QUrl(mApiPath + QLatin1String("fileops/move"));
    url.addQueryItem(QLatin1String("root"), mRootPath);
    addDefaultUrlItem(url);

    //Generate new path.
    QStringList parts = oldName.split(QLatin1String("/"), QString::SkipEmptyParts);
    parts.removeLast();
    QString destinationFolder = parts.join(QLatin1String("/"));
    if (destinationFolder.isEmpty()) {
        destinationFolder = QLatin1String("/");
    }
    if (!destinationFolder.endsWith(QLatin1Char('/'))) {
        destinationFolder += QLatin1String("/");
    }
    if (!destinationFolder.startsWith(QLatin1Char('/'))) {
        destinationFolder.prepend(QLatin1String("/"));
    }

    const QString newPath = destinationFolder.append(newName);

    url.addQueryItem(QLatin1String("from_path"), oldName);
    url.addQueryItem(QLatin1String("to_path"), newPath);

    QNetworkRequest request(url);
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void DropBoxJob::moveFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::MoveFolderAction;
    mError = false;
    QUrl url = QUrl(mApiPath + QLatin1String("fileops/move"));
    url.addQueryItem(QLatin1String("root"), mRootPath);
    addDefaultUrlItem(url);
    url.addQueryItem(QLatin1String("from_path"), source);
    url.addQueryItem(QLatin1String("to_path"), destination + source);

    QNetworkRequest request(url);
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void DropBoxJob::moveFile(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::MoveFileAction;
    mError = false;
    QUrl url = QUrl(mApiPath + QLatin1String("fileops/move"));
    url.addQueryItem(QLatin1String("root"), mRootPath);
    addDefaultUrlItem(url);
    url.addQueryItem(QLatin1String("from_path"), source);
    url.addQueryItem(QLatin1String("to_path"), destination+ source);

    QNetworkRequest request(url);
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void DropBoxJob::copyFile(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::CopyFileAction;
    mError = false;
    QUrl url = QUrl(mApiPath + QLatin1String("fileops/copy"));
    url.addQueryItem(QLatin1String("root"), mRootPath);
    addDefaultUrlItem(url);
    url.addQueryItem(QLatin1String("from_path"), source);
    url.addQueryItem(QLatin1String("to_path"), destination + source);

    QNetworkRequest request(url);
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void DropBoxJob::copyFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::CopyFolderAction;
    mError = false;
    QUrl url = QUrl(mApiPath + QLatin1String("fileops/copy"));
    url.addQueryItem(QLatin1String("root"), mRootPath);
    addDefaultUrlItem(url);
    url.addQueryItem(QLatin1String("from_path"), source);
    url.addQueryItem(QLatin1String("to_path"), destination + source);

    QNetworkRequest request(url);
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void DropBoxJob::addDefaultUrlItem(QUrl &url)
{
    url.addQueryItem(QLatin1String("oauth_consumer_key"),mOauthconsumerKey);
    url.addQueryItem(QLatin1String("oauth_nonce"), mNonce);
    url.addQueryItem(QLatin1String("oauth_signature"), mAccessOauthSignature.replace(QLatin1Char('&'),QLatin1String("%26")));
    url.addQueryItem(QLatin1String("oauth_signature_method"),mOauthSignatureMethod);
    url.addQueryItem(QLatin1String("oauth_timestamp"), mTimestamp);
    url.addQueryItem(QLatin1String("oauth_version"),mOauthVersion);
    url.addQueryItem(QLatin1String("oauth_token"),mOauthToken);
}

void DropBoxJob::parseShareLink(const QString &data)
{
#if 0 //QT5
    QJson::Parser parser;
    bool ok;
    QString url;
    QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    if (info.contains(QLatin1String("url"))) {
        url = info.value(QLatin1String("url")).toString();
    }
    Q_EMIT shareLinkDone(url);
#endif
    deleteLater();
}

void DropBoxJob::parseCreateFolder(const QString &data)
{
    const QString name = extractPathFromData(data);
    Q_EMIT createFolderDone(name);
    deleteLater();
}

void DropBoxJob::parseListFolder(const QString &data)
{
    Q_EMIT listFolderDone(data);
    deleteLater();
}

void DropBoxJob::parseDownLoadFile(const QString &data)
{
    //qDebug()<<" data "<<data;
    Q_EMIT downLoadFileDone(QString());
    deleteLater();
}
