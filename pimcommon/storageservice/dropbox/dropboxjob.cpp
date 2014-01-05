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
#include "storageservice/storageauthviewdialog.h"
#include "storageservice/storageserviceabstract.h"
#include "storageservice/storageserviceutils.h"
#include <KLocalizedString>

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

DropBoxJob::DropBoxJob(QObject *parent)
    : PimCommon::StorageServiceAbstractJob(parent)
{
    mApiPath = QLatin1String("https://api.dropbox.com/1/");
    mOauthconsumerKey = QLatin1String("e40dvomckrm48ci");
    mOauthSignature = QLatin1String("0icikya464lny9g&");
    mOauthVersion = QLatin1String("1.0");
    mOauthSignatureMethod = QLatin1String("PLAINTEXT");
    mRootPath = QLatin1String("dropbox");
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
    mActionType = PimCommon::StorageServiceAbstract::RequestToken;
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
    mActionType = PimCommon::StorageServiceAbstract::AccessToken;
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
            case PimCommon::StorageServiceAbstract::ShareLink:
            case PimCommon::StorageServiceAbstract::DownLoadFile:
            case PimCommon::StorageServiceAbstract::CreateServiceFolder:
            case PimCommon::StorageServiceAbstract::DeleteFile:
            case PimCommon::StorageServiceAbstract::DeleteFolder:
                errorMessage(mActionType, errorStr);
                deleteLater();
                break;
            default:
                qDebug()<<" Action Type unknown:"<<mActionType;
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
    case PimCommon::StorageServiceAbstract::RequestToken:
        parseRequestToken(data);
        break;
    case PimCommon::StorageServiceAbstract::AccessToken:
        parseResponseAccessToken(data);
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
    case PimCommon::StorageServiceAbstract::ShareLink:
        parseShareLink(data);
        break;
    case PimCommon::StorageServiceAbstract::CreateServiceFolder:
        deleteLater();
        break;
    case PimCommon::StorageServiceAbstract::DeleteFile:
        parseDeleteFile(data);
        break;
    case PimCommon::StorageServiceAbstract::DeleteFolder:
        parseDeleteFolder(data);
        break;
    case PimCommon::StorageServiceAbstract::DownLoadFile:
        parseDownLoadFile(data);
        break;
    default:
        qDebug()<<" Action Type unknown:"<<mActionType;
    }
}

void DropBoxJob::parseDeleteFolder(const QString &data)
{
    QJson::Parser parser;
    bool ok;
    QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    QString foldername;
    if (info.contains(QLatin1String("path"))) {
        foldername = info.value(QLatin1String("path")).toString();
    }
    Q_EMIT deleteFolderDone(foldername);

    deleteLater();
}

void DropBoxJob::parseDeleteFile(const QString &data)
{
    QJson::Parser parser;
    bool ok;
    QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    QString filename;
    if (info.contains(QLatin1String("path"))) {
        filename = info.value(QLatin1String("path")).toString();
    }
    Q_EMIT deleteFileDone(filename);
    deleteLater();
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

void DropBoxJob::createFolder(const QString &folder)
{
    mActionType = PimCommon::StorageServiceAbstract::CreateFolder;
    mError = false;
    if (folder.isEmpty()) {
        qDebug()<<" folder empty!";
    }
    QUrl url(mApiPath + QLatin1String("fileops/create_folder"));
    url.addQueryItem(QLatin1String("root"), mRootPath);
    url.addQueryItem(QLatin1String("path"), folder );
    url.addQueryItem(QLatin1String("oauth_consumer_key"),mOauthconsumerKey );
    url.addQueryItem(QLatin1String("oauth_nonce"), mNonce);
    url.addQueryItem(QLatin1String("oauth_signature"), mAccessOauthSignature.replace(QLatin1Char('&'),QLatin1String("%26")));
    url.addQueryItem(QLatin1String("oauth_signature_method"), mOauthSignatureMethod);
    url.addQueryItem(QLatin1String("oauth_timestamp"), mTimestamp);
    url.addQueryItem(QLatin1String("oauth_version"), mOauthVersion);
    url.addQueryItem(QLatin1String("oauth_token"), mOauthToken);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void DropBoxJob::uploadFile(const QString &filename)
{
    QFile *file = new QFile(filename);
    if (file->exists()) {
        mActionType = PimCommon::StorageServiceAbstract::UploadFile;
        mError = false;
        QFileInfo info(filename);
        const QString r = mAccessOauthSignature.replace(QLatin1Char('&'),QLatin1String("%26"));
        const QString str = QString::fromLatin1("https://api-content.dropbox.com/1/files_put/dropbox///test/%1?oauth_consumer_key=%2&oauth_nonce=%3&oauth_signature=%4&oauth_signature_method=PLAINTEXT&oauth_timestamp=%6&oauth_version=1.0&oauth_token=%5&overwrite=false").
                arg(info.fileName()).arg(mOauthconsumerKey).arg(mNonce).arg(r).arg(mOauthToken).arg(mTimestamp);
        KUrl url(str);
        QNetworkRequest request(url);
        if (!file->open(QIODevice::ReadOnly)) {
            delete file;
            return;
        } else {
            QNetworkReply *reply = mNetworkAccessManager->put(request, file);
            connect(reply, SIGNAL(uploadProgress(qint64,qint64)), SLOT(slotUploadFileProgress(qint64, qint64)));
            file->setParent(reply);
            connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
        }
    }
}

void DropBoxJob::accountInfo()
{
    mActionType = PimCommon::StorageServiceAbstract::AccountInfo;
    mError = false;
    QUrl url(mApiPath + QLatin1String("account/info"));
    url.addQueryItem(QLatin1String("oauth_consumer_key"), mOauthconsumerKey);
    url.addQueryItem(QLatin1String("oauth_nonce"), mNonce);
    url.addQueryItem(QLatin1String("oauth_signature"), mAccessOauthSignature.replace(QLatin1Char('&'),QLatin1String("%26")));
    url.addQueryItem(QLatin1String("oauth_signature_method"),mOauthSignatureMethod);
    url.addQueryItem(QLatin1String("oauth_timestamp"), mTimestamp);
    url.addQueryItem(QLatin1String("oauth_version"), mOauthVersion);
    url.addQueryItem(QLatin1String("oauth_token"), mOauthToken);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));

    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void DropBoxJob::listFolder(const QString &folder)
{
    mActionType = PimCommon::StorageServiceAbstract::ListFolder;
    mError = false;
    QUrl url(mApiPath + QLatin1String("metadata/dropbox/") + folder);
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

void DropBoxJob::slotUploadFileProgress(qint64 done, qint64 total)
{
    //qDebug()<<" done "<<done<<" total :"<<total;
    Q_EMIT uploadFileProgress(done, total);
}

void DropBoxJob::parseUploadFile(const QString &data)
{
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
    Q_EMIT uploadFileDone(path);
    shareLink(root, path);
}

void DropBoxJob::shareLink(const QString &root, const QString &path)
{
    mActionType = PimCommon::StorageServiceAbstract::ShareLink;
    mError = false;

    QUrl url = QUrl(mApiPath + QString::fromLatin1("shares/%1/%2").arg(root).arg(path));
    url.addQueryItem(QLatin1String("root"), mRootPath);
    url.addQueryItem(QLatin1String("oauth_consumer_key"),mOauthconsumerKey);
    url.addQueryItem(QLatin1String("oauth_nonce"), nonce);
    url.addQueryItem(QLatin1String("oauth_signature"), mAccessOauthSignature.replace(QLatin1Char('&'),QLatin1String("%26")));
    url.addQueryItem(QLatin1String("oauth_signature_method"),mOauthSignatureMethod);
    url.addQueryItem(QLatin1String("oauth_timestamp"), mTimestamp);
    url.addQueryItem(QLatin1String("oauth_version"),mOauthVersion);
    url.addQueryItem(QLatin1String("oauth_token"),mOauthToken);
    QNetworkRequest request(url);

    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void DropBoxJob::createServiceFolder()
{
    mActionType = PimCommon::StorageServiceAbstract::CreateServiceFolder;
    mError = false;
    qDebug()<<" not implemented";
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    deleteLater();
}

void DropBoxJob::downloadFile(const QString &filename)
{
    mActionType = PimCommon::StorageServiceAbstract::DownLoadFile;
    mError = false;
    qDebug()<<" not implemented";
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    deleteLater();
}

void DropBoxJob::deleteFile(const QString &filename)
{
    mActionType = PimCommon::StorageServiceAbstract::DeleteFile;
    mError = false;
    QUrl url = QUrl(mApiPath + QLatin1String("fileops/delete"));
    url.addQueryItem(QLatin1String("root"), mRootPath);
    url.addQueryItem(QLatin1String("path"), filename);
    url.addQueryItem(QLatin1String("oauth_consumer_key"),mOauthconsumerKey);
    url.addQueryItem(QLatin1String("oauth_nonce"), nonce);
    url.addQueryItem(QLatin1String("oauth_signature"), mAccessOauthSignature.replace(QLatin1Char('&'),QLatin1String("%26")));
    url.addQueryItem(QLatin1String("oauth_signature_method"),mOauthSignatureMethod);
    url.addQueryItem(QLatin1String("oauth_timestamp"), mTimestamp);
    url.addQueryItem(QLatin1String("oauth_version"),mOauthVersion);
    url.addQueryItem(QLatin1String("oauth_token"),mOauthToken);
    QNetworkRequest request(url);
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void DropBoxJob::deleteFolder(const QString &foldername)
{
    mActionType = PimCommon::StorageServiceAbstract::DeleteFolder;
    mError = false;
    QUrl url = QUrl(mApiPath + QLatin1String("fileops/delete"));
    url.addQueryItem(QLatin1String("root"), mRootPath);
    url.addQueryItem(QLatin1String("path"), foldername);
    url.addQueryItem(QLatin1String("oauth_consumer_key"),mOauthconsumerKey);
    url.addQueryItem(QLatin1String("oauth_nonce"), nonce);
    url.addQueryItem(QLatin1String("oauth_signature"), mAccessOauthSignature.replace(QLatin1Char('&'),QLatin1String("%26")));
    url.addQueryItem(QLatin1String("oauth_signature_method"),mOauthSignatureMethod);
    url.addQueryItem(QLatin1String("oauth_timestamp"), mTimestamp);
    url.addQueryItem(QLatin1String("oauth_version"),mOauthVersion);
    url.addQueryItem(QLatin1String("oauth_token"),mOauthToken);
    QNetworkRequest request(url);
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void DropBoxJob::parseShareLink(const QString &data)
{
    QJson::Parser parser;
    bool ok;
    QString url;
    QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    if (info.contains(QLatin1String("url"))) {
        url = info.value(QLatin1String("url")).toString();
    }
    Q_EMIT shareLinkDone(url);
    deleteLater();
}

void DropBoxJob::parseCreateFolder(const QString &data)
{
    QJson::Parser parser;
    bool ok;
    QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    QString foldername;
    if (info.contains(QLatin1String("path"))) {
        foldername = info.value(QLatin1String("path")).toString();
    }
    Q_EMIT createFolderDone(foldername);
    deleteLater();
}

void DropBoxJob::parseListFolder(const QString &data)
{
    QJson::Parser parser;
    bool ok;
    QStringList listFolder;
    QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    if (info.contains(QLatin1String("contents"))) {
        const QVariantList lst = info.value(QLatin1String("contents")).toList();
        Q_FOREACH (const QVariant &variant, lst) {
            const QVariantMap qwer = variant.toMap();
            const QList<QString> b = qwer.uniqueKeys();
            for(int i=0;i<qwer.size();i++) {
                const QString identifier = b.at(i);
                if((identifier == QLatin1String("is_dir")) || (identifier == QLatin1String("path"))) {
                    if(identifier == QLatin1String("path") && i==4) {
                        const QString name = qwer[b[i]].toString().section(QLatin1Char('/'),-2);
                        listFolder.append(name);
                    }
                }
            }
        }
    }
    Q_EMIT listFolderDone(listFolder);
    deleteLater();
}

void DropBoxJob::parseDownLoadFile(const QString &data)
{
    qDebug()<<" data "<<data;
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    deleteLater();
}
