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
#include <libkgapi2/drive/filefetchcontentjob.h>
#include <libkgapi2/types.h>
#include <libkgapi2/drive/childreferencefetchjob.h>
#include <libkgapi2/drive/childreference.h>
#include <libkgapi2/drive/file.h>
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

QString GDriveJob::lastPathComponent( const QUrl &url ) const
{
    QString path = url.toString( QUrl::StripTrailingSlash );
    if ( path.indexOf( QLatin1Char( '/' ) ) == -1 ) {
        return QLatin1String( "root" );
    } else {
        return path.mid( path.lastIndexOf( QLatin1Char('/') ) + 1 );
    }
}

void GDriveJob::listFolder(const QString &folder)
{
    mActionType = PimCommon::StorageServiceAbstract::ListFolder;
    mError = false;
    const QString folderId = lastPathComponent( folder );
    qDebug()<<"folderId "<<folderId;
    KGAPI2::Drive::ChildReferenceFetchJob *fetchJob = new KGAPI2::Drive::ChildReferenceFetchJob( folderId, mAccount );
    connect(fetchJob, SIGNAL(finished(KGAPI2::Job*)), this, SLOT(slotChildReferenceFetchJobFinished(KGAPI2::Job*)));
}

void GDriveJob::slotChildReferenceFetchJobFinished(KGAPI2::Job *job)
{
    KGAPI2::Drive::ChildReferenceFetchJob *childRef = qobject_cast<KGAPI2::Drive::ChildReferenceFetchJob *>(job);
    if (childRef) {
        KGAPI2::ObjectsList objects = childRef->items();
        QStringList filesIds;
        Q_FOREACH ( const KGAPI2::ObjectPtr &object, objects ) {
            const KGAPI2::Drive::ChildReferencePtr ref = object.dynamicCast<KGAPI2::Drive::ChildReference>();
            filesIds << ref->id();
        }
        KGAPI2::Drive::FileFetchJob *fileFetchJob = new KGAPI2::Drive::FileFetchJob( filesIds, mAccount );
        connect(fileFetchJob, SIGNAL(finished(KGAPI2::Job*)), this, SLOT(slotFileFetchFinished(KGAPI2::Job*)));
    } else {
        //TODO emit error
        deleteLater();
    }
}

void GDriveJob::slotFileFetchFinished(KGAPI2::Job* job)
{
    KGAPI2::Drive::FileFetchJob *fileFetchJob = qobject_cast<KGAPI2::Drive::FileFetchJob*>(job);
    Q_ASSERT(fileFetchJob);
    qDebug()<<" fileFetchJob" <<fileFetchJob->items().count();
    QStringList listFolder;
    KGAPI2::ObjectsList objects = fileFetchJob->items();
    Q_FOREACH ( const KGAPI2::ObjectPtr &object, objects ) {
        const KGAPI2::Drive::FilePtr file = object.dynamicCast<KGAPI2::Drive::File>();
        if ( file->labels()->trashed() ) {
            continue;
        }
        const QString value = QString::fromLatin1(KGAPI2::Drive::File::toJSON(file));
        listFolder<<value;
    }
    Q_EMIT listFolderDone(QVariant(listFolder));
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
    qDebug()<<" account->expireDateTime()"<<account->expireDateTime();
    Q_EMIT authorizationDone(account->refreshToken(),account->accessToken(), account->expireDateTime(), account->accountName());
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

QNetworkReply * GDriveJob::downloadFile(const QString &name, const QString &fileId, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::DownLoadFile;
    mError = false;
    //Add url
    //KGAPI2::Drive::FileFetchContentJob *fileFetchContentJob = new KGAPI2::Drive::FileFetchContentJob(mAccount, this);
    return 0;
}

void GDriveJob::refreshToken()
{
    mActionType = PimCommon::StorageServiceAbstract::AccessToken;
    KGAPI2::AuthJob *authJob = new KGAPI2::AuthJob(
                mAccount,
                mClientId,
                mClientSecret);
    connect(authJob, SIGNAL(finished(KGAPI2::Job*)),
            this, SLOT(slotAuthJobFinished(KGAPI2::Job*)));
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


