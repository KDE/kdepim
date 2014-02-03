/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#include "gdrivejob.h"
#include "storageservice/authdialog/storageauthviewdialog.h"
#include "pimcommon/storageservice/storageservicejobconfig.h"


#include <libkgapi2/authjob.h>
#include <libkgapi2/account.h>
#include <libkgapi2/drive/aboutfetchjob.h>
#include <libkgapi2/drive/about.h>
#include <libkgapi2/drive/filecreatejob.h>
#include <libkgapi2/drive/filedeletejob.h>
#include <libkgapi2/drive/filefetchjob.h>

#include <KLocalizedString>

#include <qjson/parser.h>

#include <QNetworkAccessManager>
#include <QDebug>
#include <QNetworkReply>
#include <QPointer>

#include <QDebug>
#include <QFile>
#include <QPointer>
#include <QDateTime>

using namespace PimCommon;

GDriveJob::GDriveJob(QObject *parent)
    : PimCommon::StorageServiceAbstractJob(parent)
{
    mClientId = PimCommon::StorageServiceJobConfig::self()->gdriveClientId();
    mClientSecret = PimCommon::StorageServiceJobConfig::self()->gdriveClientSecret();
}

GDriveJob::~GDriveJob()
{

}

void GDriveJob::listFolder(const QString &folder)
{
    mActionType = PimCommon::StorageServiceAbstract::ListFolder;
    mError = false;
    KGAPI2::Drive::FileFetchJob *fileFetchJob = new KGAPI2::Drive::FileFetchJob(mAccount, this);
    connect(fileFetchJob, SIGNAL(finished(KGAPI2::Job*)), this, SLOT(slotFileFetchJobFinished(KGAPI2::Job*)));
}

void GDriveJob::slotFileFetchJobFinished(KGAPI2::Job* job)
{
    KGAPI2::Drive::FileFetchJob *fileFetchJob = qobject_cast<KGAPI2::Drive::FileFetchJob*>(job);
    Q_ASSERT(fileFetchJob);
    qDebug()<<"void GDriveJob::slotFileFetchJobFinished(KGAPI2::Job* job)";
    qDebug()<<" fileFetchJob" <<fileFetchJob->items().count();
    //TODO
    Q_EMIT listFolderDone(QString());

    deleteLater();
}

void GDriveJob::requestTokenAccess()
{
    mError = false;
    mActionType = PimCommon::StorageServiceAbstract::RequestToken;

    KGAPI2::AuthJob *authJob = new KGAPI2::AuthJob(
                mAccount,
                mClientId,
                mClientSecret);
    connect(authJob, SIGNAL(finished(KGAPI2::Job*)),
            this, SLOT(slotAuthJobFinished(KGAPI2::Job*)));
}

void GDriveJob::slotAuthJobFinished(KGAPI2::Job *job)
{
    KGAPI2::AuthJob *authJob = qobject_cast<KGAPI2::AuthJob*>(job);
    Q_ASSERT(authJob);

    if (authJob->error() != KGAPI2::NoError) {
        Q_EMIT authorizationFailed(authJob->errorString());
        deleteLater();
        return;
    }
    KGAPI2::AccountPtr account = authJob->account();
    Q_EMIT authorizationDone(account->refreshToken(),account->accessToken());
    /* Always remember to delete the jobs, otherwise your application will
     * leak memory. */
    authJob->deleteLater();
    deleteLater();
}

void GDriveJob::initializeToken(KGAPI2::AccountPtr account)
{
    mError = false;
    mAccount = account;
}

void GDriveJob::accountInfo()
{
    mActionType = PimCommon::StorageServiceAbstract::AccountInfo;
    mError = false;
    KGAPI2::Drive::AboutFetchJob *aboutFetchJob = new KGAPI2::Drive::AboutFetchJob(mAccount, this);
    connect(aboutFetchJob, SIGNAL(finished(KGAPI2::Job*)), this, SLOT(slotAboutFetchJobFinished(KGAPI2::Job*)));
}

void GDriveJob::slotAboutFetchJobFinished(KGAPI2::Job *job)
{
    KGAPI2::Drive::AboutFetchJob *aboutFetchJob = qobject_cast<KGAPI2::Drive::AboutFetchJob*>(job);
    Q_ASSERT(aboutFetchJob);

    if (aboutFetchJob->error() != KGAPI2::NoError) {
        qDebug()<<" ERRRRR"<<aboutFetchJob->errorString();
        Q_EMIT actionFailed(aboutFetchJob->errorString());
        deleteLater();
        return;
    }
    KGAPI2::Drive::AboutPtr about = aboutFetchJob->aboutData();
    PimCommon::AccountInfo accountInfo;
    accountInfo.shared = about->quotaBytesUsed();
    accountInfo.quota = about->quotaBytesTotal();
    Q_EMIT accountInfoDone(accountInfo);
    deleteLater();
}

QNetworkReply *GDriveJob::uploadFile(const QString &filename, const QString &uploadAsName, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::UploadFile;
    mError = false;
    //TODO
    return 0;
}

void GDriveJob::deleteFile(const QString &filename)
{
    mActionType = PimCommon::StorageServiceAbstract::DeleteFile;
    mError = false;
    KGAPI2::Drive::FileDeleteJob *fileDeleteJob = new KGAPI2::Drive::FileDeleteJob(filename, mAccount, this);
    connect(fileDeleteJob, SIGNAL(finished(KGAPI2::Job*)), this, SLOT(slotDeleteFileFinished(KGAPI2::Job*)));
}

void GDriveJob::slotDeleteFileFinished(KGAPI2::Job*job)
{
    //TODO
    Q_EMIT deleteFileDone(QString());
    deleteLater();
}


/*old **********************/


void GDriveJob::createServiceFolder()
{
    mActionType = PimCommon::StorageServiceAbstract::CreateServiceFolder;
    mError = false;
    //TODO
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    qDebug()<<" not implemented";
    deleteLater();
}

QNetworkReply * GDriveJob::downloadFile(const QString &name, const QString &fileId, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::DownLoadFile;
    mError = false;
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    qDebug()<<" not implemented";
    deleteLater();
    return 0;
}

void GDriveJob::deleteFolder(const QString &foldername)
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

void GDriveJob::renameFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::RenameFolder;
    mError = false;
    qDebug()<<" not implemented";
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    //TODO
    deleteLater();
}

void GDriveJob::renameFile(const QString &oldName, const QString &newName)
{
    mActionType = PimCommon::StorageServiceAbstract::RenameFile;
    mError = false;
    qDebug()<<" not implemented";
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    //TODO
    deleteLater();
}

void GDriveJob::moveFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::MoveFolder;
    mError = false;
    qDebug()<<" not implemented";
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    //TODO
    deleteLater();
}

void GDriveJob::moveFile(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::MoveFile;
    mError = false;
    qDebug()<<" not implemented";
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    //TODO
    deleteLater();
}

void GDriveJob::copyFile(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::CopyFile;
    mError = false;
    qDebug()<<" not implemented";
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    //TODO
    deleteLater();
}

void GDriveJob::copyFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::CopyFolder;
    mError = false;
    qDebug()<<" not implemented";
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    //TODO
    deleteLater();
}

void GDriveJob::parseRedirectUrl(const QUrl &url)
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

void GDriveJob::getTokenAccess(const QString &authorizeCode)
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

void GDriveJob::createFolder(const QString &foldername, const QString &destination)
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
    //postData.addQueryItem(QLatin1String("id"), QLatin1String("0"));
    postData.addQueryItem(QLatin1String("parent"), QLatin1String("{\'id\': \'0\'}"));
    QNetworkReply *reply = mNetworkAccessManager->post(request, postData.encodedQuery());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void GDriveJob::shareLink(const QString &fileId)
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

void GDriveJob::shareLink(const QString &root, const QString &path)
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


void GDriveJob::slotSendDataFinished(QNetworkReply *reply)
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
        Q_EMIT actionFailed(QLatin1String("Not Implemented"));
        deleteLater();
        break;
    }
}

void GDriveJob::parseRenameFile(const QString &data)
{
    Q_EMIT renameFileDone(QString());
}

void GDriveJob::parseRenameFolder(const QString &data)
{
    Q_EMIT renameFolderDone(QString());
}

void GDriveJob::parseCopyFile(const QString &data)
{
    Q_EMIT copyFileDone(QString());
}

void GDriveJob::parseCopyFolder(const QString &data)
{
    Q_EMIT copyFolderDone(QString());
}

void GDriveJob::parseDeleteFolder(const QString &data)
{
    QJson::Parser parser;
    bool ok;

    const QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    qDebug()<<" info"<<info;
    Q_EMIT deleteFolderDone(QString());
}

void GDriveJob::parseDeleteFile(const QString &data)
{
    QJson::Parser parser;
    bool ok;

    const QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    qDebug()<<" info"<<info;
    Q_EMIT deleteFileDone(QString());
}

void GDriveJob::parseCreateServiceFolder(const QString &data)
{
    QJson::Parser parser;
    bool ok;

    const QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    qDebug()<<" info"<<info;
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    deleteLater();
}

void GDriveJob::parseListFolder(const QString &data)
{
    Q_EMIT listFolderDone(data);
    deleteLater();
}

void GDriveJob::parseMoveFolder(const QString &data)
{
    Q_EMIT moveFolderDone(data);
    deleteLater();
}

void GDriveJob::parseMoveFile(const QString &data)
{
    Q_EMIT moveFileDone(data);
    deleteLater();
}

void GDriveJob::parseShareLink(const QString &data)
{
    //TODO reimplement in derivated function
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    deleteLater();
}

void GDriveJob::parseAccountInfo(const QString &)
{
    //TODO reimplement in derivated function
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    deleteLater();
}

void GDriveJob::parseCreateFolder(const QString &data)
{
    //TODO
    qDebug()<<" data "<<data;
    Q_EMIT createFolderDone(QString());
    deleteLater();
}

void GDriveJob::parseUploadFile(const QString &data)
{
    //TODO
    qDebug()<<" data "<<data;
    Q_EMIT uploadFileDone(QString());
    deleteLater();
}


void GDriveJob::parseAccessToken(const QString &data)
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
    qDebug()<<" parseAccessToken";
    //Q_EMIT authorizationDone(mRefreshToken, mToken, mExpireInTime);
    //TODO save it.
    deleteLater();
}


void GDriveJob::refreshToken()
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
