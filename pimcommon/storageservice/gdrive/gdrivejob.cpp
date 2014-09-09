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


#include <kgapi/authjob.h>
#include <kgapi/account.h>
#include <kgapi/drive/aboutfetchjob.h>
#include <kgapi/drive/about.h>
#include <kgapi/drive/filecreatejob.h>
#include <kgapi/drive/filedeletejob.h>
#include <kgapi/drive/filefetchjob.h>
#include <kgapi/drive/filefetchcontentjob.h>
#include <kgapi/types.h>
#include <kgapi/drive/childreferencefetchjob.h>
#include <kgapi/drive/childreference.h>
#include <kgapi/drive/file.h>
#include <kgapi/drive/filecopyjob.h>
#include <KGAPI/Drive/ParentReference>



#include <QDebug>
#include <QNetworkReply>
#include <QPointer>

#include <QDebug>
#include <QFile>
#include <QPointer>

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

bool GDriveJob::handleError( KGAPI2::Job *job )
{
    qDebug() << job->error() << job->errorString();

    switch ( job->error() ) {
    case KGAPI2::OK:
    case KGAPI2::NoError:
        return false;
    case KGAPI2::AuthCancelled:
    case KGAPI2::AuthError:
    case KGAPI2::Unauthorized:
    case KGAPI2::Forbidden:
    case KGAPI2::NotFound:
    case KGAPI2::NoContent:
    case KGAPI2::QuotaExceeded:
    default:
        return true;
    }

    return true;
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
    mActionType = PimCommon::StorageServiceAbstract::ListFolderAction;
    mError = false;
    const QString folderId = folder.isEmpty() ? QLatin1String("root") : folder;
    KGAPI2::Drive::ChildReferenceFetchJob *fetchJob = new KGAPI2::Drive::ChildReferenceFetchJob( folderId, mAccount );
    connect(fetchJob, &KGAPI2::Drive::ChildReferenceFetchJob::finished, this, &GDriveJob::slotChildReferenceFetchJobFinished);
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
        connect(fileFetchJob, &KGAPI2::Drive::FileFetchJob::finished, this, &GDriveJob::slotFileFetchFinished);
        childRef->deleteLater();
    } else {
        //TODO emit error
        deleteLater();
    }
}

void GDriveJob::slotFileFetchFinished(KGAPI2::Job* job)
{
    KGAPI2::Drive::FileFetchJob *fileFetchJob = qobject_cast<KGAPI2::Drive::FileFetchJob*>(job);
    Q_ASSERT(fileFetchJob);
    QStringList listFolder;
    KGAPI2::ObjectsList objects = fileFetchJob->items();
    Q_FOREACH ( const KGAPI2::ObjectPtr &object, objects ) {
        const KGAPI2::Drive::FilePtr file = object.dynamicCast<KGAPI2::Drive::File>();
        qDebug()<<" file "<<file;
	
        if ( !file->labels() || file->labels()->trashed() ) {
            continue;
        }
        const QString value = QString::fromLatin1(KGAPI2::Drive::File::toJSON(file));
        listFolder<<value;
    }
    fileFetchJob->deleteLater();
    Q_EMIT listFolderDone(QVariant(listFolder));
    deleteLater();
}

void GDriveJob::requestTokenAccess()
{
    mError = false;
    mActionType = PimCommon::StorageServiceAbstract::RequestTokenAction;

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

    if (handleError(job)) {
        Q_EMIT authorizationFailed(authJob->errorString());
        deleteLater();
        return;
    }
    KGAPI2::AccountPtr account = authJob->account();
    //qDebug()<<" account->expireDateTime()"<<account->expireDateTime();
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
    mActionType = PimCommon::StorageServiceAbstract::AccountInfoAction;
    mError = false;
    KGAPI2::Drive::AboutFetchJob *aboutFetchJob = new KGAPI2::Drive::AboutFetchJob(mAccount, this);
    connect(aboutFetchJob, &KGAPI2::Drive::AboutFetchJob::finished, this, &GDriveJob::slotAboutFetchJobFinished);
}

void GDriveJob::slotAboutFetchJobFinished(KGAPI2::Job *job)
{
    KGAPI2::Drive::AboutFetchJob *aboutFetchJob = qobject_cast<KGAPI2::Drive::AboutFetchJob*>(job);
    Q_ASSERT(aboutFetchJob);
    if (handleError(job)) {
        Q_EMIT actionFailed(aboutFetchJob->errorString());
        aboutFetchJob->deleteLater();
        deleteLater();
        return;
    }
    KGAPI2::Drive::AboutPtr about = aboutFetchJob->aboutData();
    PimCommon::AccountInfo accountInfo;
    accountInfo.shared = about->quotaBytesUsed();
    accountInfo.quota = about->quotaBytesTotal();
    Q_EMIT accountInfoDone(accountInfo);
    aboutFetchJob->deleteLater();
    deleteLater();
}

QNetworkReply *GDriveJob::uploadFile(const QString &filename, const QString &uploadAsName, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::UploadFileAction;
    mError = false;
    KGAPI2::Drive::FilePtr file( new KGAPI2::Drive::File() );
    file->setTitle( uploadAsName );
    KGAPI2::Drive::ParentReferencePtr parent( new KGAPI2::Drive::ParentReference( destination ) );
    file->setParents( KGAPI2::Drive::ParentReferencesList() << parent );

    //TODO destination
    KGAPI2::Drive::FileCreateJob *createJob = new KGAPI2::Drive::FileCreateJob( filename/*, file*/, mAccount);
    connect(createJob, &KGAPI2::Drive::FileCreateJob::finished, this, &GDriveJob::slotUploadJobFinished);
    connect(createJob, &KGAPI2::Drive::FileCreateJob::progress, this, &GDriveJob::slotUploadDownLoadProgress);
    return 0;
}

void GDriveJob::slotUploadDownLoadProgress(KGAPI2::Job *job, int progress, int total)
{
    Q_UNUSED(job);
    qDebug()<<" progress "<<progress<<" total"<<total;
    Q_EMIT uploadDownloadFileProgress(progress, total);
}

void GDriveJob::slotUploadJobFinished(KGAPI2::Job* job)
{
    if (handleError(job)) {
        Q_EMIT errorMessage(mActionType, job->errorString());
    } else {
        Q_EMIT uploadFileDone(QString());
    }
    job->deleteLater();
    deleteLater();
}

void GDriveJob::deleteFile(const QString &filename)
{
    mActionType = PimCommon::StorageServiceAbstract::DeleteFileAction;
    mError = false;
    const QString folderId = filename;
    KGAPI2::Drive::FileDeleteJob *fileDeleteJob = new KGAPI2::Drive::FileDeleteJob(folderId, mAccount, this);
    connect(fileDeleteJob, &KGAPI2::Drive::FileDeleteJob::finished, this, &GDriveJob::slotDeleteFileFinished);
}

void GDriveJob::deleteFolder(const QString &foldername)
{
    mActionType = PimCommon::StorageServiceAbstract::DeleteFolderAction;
    mError = false;
    const QString folderId = foldername;
    KGAPI2::Drive::FileDeleteJob *fileDeleteJob = new KGAPI2::Drive::FileDeleteJob(folderId, mAccount, this);
    connect(fileDeleteJob, &KGAPI2::Drive::FileDeleteJob::finished, this, &GDriveJob::slotDeleteFolderFinished);
}

void GDriveJob::slotDeleteFolderFinished(KGAPI2::Job*job)
{
    if (handleError(job)) {
        Q_EMIT errorMessage(mActionType, job->errorString());
    } else {
        Q_EMIT deleteFolderDone(QString());
    }
    job->deleteLater();
    deleteLater();
}


void GDriveJob::slotDeleteFileFinished(KGAPI2::Job*job)
{
    if (handleError(job)) {
        Q_EMIT errorMessage(mActionType, job->errorString());
    } else {
        Q_EMIT deleteFileDone(QString());
    }
    job->deleteLater();
    deleteLater();
}

QNetworkReply * GDriveJob::downloadFile(const QString &name, const QString &fileId, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::DownLoadFileAction;
    mError = false;
    const QString defaultDestination = (destination.isEmpty() ? PimCommon::StorageServiceJobConfig::self()->defaultUploadFolder() : destination);

    delete mDownloadFile;
    mDownloadFile = new QFile(defaultDestination+ QLatin1Char('/') + name);
    if (mDownloadFile->open(QIODevice::WriteOnly)) {
        KGAPI2::Drive::FilePtr file( new KGAPI2::Drive::File() );
        KGAPI2::Drive::FileFetchContentJob *fileFetchContentJob = new KGAPI2::Drive::FileFetchContentJob(file, mAccount, this);
        //TODO
        return 0;
    } else {
        delete mDownloadFile;
    }
    return 0;
}

void GDriveJob::refreshToken()
{
    mActionType = PimCommon::StorageServiceAbstract::AccessTokenAction;
    KGAPI2::AuthJob *authJob = new KGAPI2::AuthJob(
                mAccount,
                mClientId,
                mClientSecret);
    connect(authJob, &KGAPI2::AuthJob::finished, this, &GDriveJob::slotAuthJobFinished);
}

void GDriveJob::createFolder(const QString &foldername, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::CreateFolderAction;
    mError = false;

    KGAPI2::Drive::FilePtr file( new KGAPI2::Drive::File() );
    file->setTitle( foldername );
    file->setMimeType( KGAPI2::Drive::File::folderMimeType() );

    KGAPI2::Drive::ParentReferencePtr parent( new KGAPI2::Drive::ParentReference( destination ) );
    file->setParents( KGAPI2::Drive::ParentReferencesList() << parent );

    KGAPI2::Drive::FileCreateJob *createJob = new KGAPI2::Drive::FileCreateJob( file, mAccount);
    connect(createJob, &KGAPI2::Drive::FileCreateJob::finished, this, &GDriveJob::slotCreateJobFinished);
}

void GDriveJob::slotCreateJobFinished(KGAPI2::Job *job)
{
    if (handleError(job)) {
        Q_EMIT errorMessage(mActionType, job->errorString());
    } else {
        Q_EMIT createFolderDone(QString());
    }
    job->deleteLater();
    deleteLater();
}

void GDriveJob::createServiceFolder()
{
    mActionType = PimCommon::StorageServiceAbstract::CreateServiceFolderAction;
    mError = false;
    const QString folderName = lastPathComponent( QString() );

    KGAPI2::Drive::FilePtr file( new KGAPI2::Drive::File() );
    file->setTitle( PimCommon::StorageServiceJobConfig::self()->defaultUploadFolder() );
    file->setMimeType( KGAPI2::Drive::File::folderMimeType() );

    KGAPI2::Drive::ParentReferencePtr parent( new KGAPI2::Drive::ParentReference( folderName ) );
    file->setParents( KGAPI2::Drive::ParentReferencesList() << parent );

    KGAPI2::Drive::FileCreateJob *createJob = new KGAPI2::Drive::FileCreateJob( file, mAccount);
    connect(createJob, &KGAPI2::Drive::FileCreateJob::finished, this, &GDriveJob::slotCreateJobFinished);
}

void GDriveJob::copyFile(const QString &source, const QString &destination)
{
    qDebug()<<"source "<<source<<" destination"<<destination;

    mActionType = PimCommon::StorageServiceAbstract::CopyFileAction;
    mError = false;
    KGAPI2::Drive::FilePtr file( new KGAPI2::Drive::File() );
    file->setTitle( QLatin1String("copy") );
    file->setMimeType( KGAPI2::Drive::File::folderMimeType() );

    KGAPI2::Drive::ParentReferencePtr parent( new KGAPI2::Drive::ParentReference( destination ) );
    file->setParents( KGAPI2::Drive::ParentReferencesList() << parent );

    KGAPI2::Drive::FileCopyJob *copyJob = new KGAPI2::Drive::FileCopyJob( source, file, mAccount);
    connect(copyJob, &KGAPI2::Drive::FileCopyJob::finished, this, &GDriveJob::slotCopyJobFinished);
}

void GDriveJob::slotCopyJobFinished(KGAPI2::Job *job)
{
    if (handleError(job)) {
        Q_EMIT errorMessage(mActionType, job->errorString());
    } else {
        Q_EMIT copyFileDone(QString());
    }
    job->deleteLater();
    deleteLater();
}

void GDriveJob::copyFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::CopyFolderAction;
    mError = false;
    KGAPI2::Drive::FilePtr file( new KGAPI2::Drive::File() );
    file->setTitle( QLatin1String("copy") );
    file->setMimeType( KGAPI2::Drive::File::folderMimeType() );

    KGAPI2::Drive::ParentReferencePtr parent( new KGAPI2::Drive::ParentReference( destination ) );
    file->setParents( KGAPI2::Drive::ParentReferencesList() << parent );

    KGAPI2::Drive::FileCopyJob *copyJob = new KGAPI2::Drive::FileCopyJob( source, file, mAccount);
    connect(copyJob, &KGAPI2::Drive::FileCopyJob::finished, this, &GDriveJob::slotCopyFolderJobFinished);
}

void GDriveJob::slotCopyFolderJobFinished(KGAPI2::Job *job)
{
    if (handleError(job)) {
        Q_EMIT errorMessage(mActionType, job->errorString());
    } else {
        Q_EMIT copyFolderDone(QString());
    }
    job->deleteLater();
    deleteLater();
}

void GDriveJob::renameFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::RenameFolderAction;
    qDebug()<<" source "<<source<<" destination "<<destination;
    mError = false;
    qDebug()<<" not implemented";
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    //TODO
    deleteLater();
}

void GDriveJob::renameFile(const QString &oldName, const QString &newName)
{
    mActionType = PimCommon::StorageServiceAbstract::RenameFileAction;
    qDebug()<<" oldName "<<oldName<<" newName "<<newName;
    mError = false;
    qDebug()<<" not implemented";
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    //TODO
    deleteLater();
}


/*old **********************/




void GDriveJob::moveFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::MoveFolderAction;
    mError = false;
    qDebug()<<" not implemented";
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    //TODO
    deleteLater();
}

void GDriveJob::moveFile(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::MoveFileAction;
    mError = false;
    qDebug()<<" not implemented";
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    //TODO
    deleteLater();
}

void GDriveJob::shareLink(const QString &root, const QString &path)
{
    mActionType = PimCommon::StorageServiceAbstract::ShareLinkAction;
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


