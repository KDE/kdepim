/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

  webdav access based on QWebDav Copyright (C) 2009-2010 Corentin Chary <corentin.chary@gmail.com>

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

#include "webdavjob.h"
#include "webdavjob_p.cpp"
#include "webdavsettingsdialog.h"
#include "pimcommon/storageservice/authdialog/logindialog.h"
#include "pimcommon/storageservice/storageservicejobconfig.h"

#include <KLocalizedString>

#include <QAuthenticator>
#include <QNetworkAccessManager>
#include <QDebug>
#include <QNetworkReply>
#include <QPointer>
#include <QBuffer>
#include <QFile>
#include <QDomDocument>
#include <QDomNodeList>

using namespace PimCommon;

WebDavJob::WebDavJob(QObject *parent)
    : PimCommon::StorageServiceAbstractJob(parent),
      mNbAuthCheck(0)
{
    mShareApi = QLatin1String("/ocs/v1.php/apps/files_sharing/api/v1/shares");
    connect(mNetworkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotSendDataFinished(QNetworkReply*)));
    connect(mNetworkAccessManager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), SLOT(slotAuthenticationRequired(QNetworkReply*,QAuthenticator*)));
}

WebDavJob::~WebDavJob()
{

}

void WebDavJob::initializeToken(const QString &publicLocation, const QString &serviceLocation, const QString &username, const QString &password)
{
    mError = false;
    mCacheValue.clear();
    mUserName = username;
    mPassword = password;
    mPublicLocation = publicLocation;
    mServiceLocation = serviceLocation;
}

void WebDavJob::slotAuthenticationRequired(QNetworkReply *,QAuthenticator *auth)
{
    if ((mNbAuthCheck > 2) || (mUserName.isEmpty() || mPassword.isEmpty())) {
        QPointer<LoginDialog> dlg = new LoginDialog;
        dlg->setCaption(i18n("WebDav"));
        dlg->setUserName(auth->user());
        dlg->setPassword(auth->password());
        if (dlg->exec()) {
            mUserName = dlg->username();
            mPassword = dlg->password();
            auth->setUser(mUserName);
            auth->setPassword(mPassword);
            Q_EMIT authorizationDone(mPublicLocation, mServiceLocation, mUserName, mPassword);
            mNbAuthCheck = -1;
        } else {
            Q_EMIT authorizationFailed(i18n("Authentication Canceled."));
            deleteLater();
        }
        delete dlg;
    } else {
        auth->setUser(mUserName);
        auth->setPassword(mPassword);
    }
    ++mNbAuthCheck;
}

void WebDavJob::requestTokenAccess()
{
    mError = false;
    mActionType = PimCommon::StorageServiceAbstract::AccessTokenAction;
    QPointer<WebDavSettingsDialog> dlg = new WebDavSettingsDialog;
    if (dlg->exec()) {
        mServiceLocation = dlg->serviceLocation();
        mPublicLocation = dlg->publicLocation();
    } else {
        Q_EMIT authorizationFailed(i18n("Authentication Canceled."));
        delete dlg;
        deleteLater();
        return;
    }
    delete dlg;    
    QUrl url(mServiceLocation);
    QNetworkReply *reply = accountInfo(url.toString());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void WebDavJob::copyFile(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::CopyFileAction;
    mError = false;
    QString filename;
    if (!source.isEmpty()) {
        QStringList parts = source.split(QLatin1String("/"), QString::SkipEmptyParts);
        filename = parts.takeLast();
    }

    const QString destinationFolder = destination + QLatin1Char('/') + filename;

    QUrl sourceFile(mServiceLocation);
    sourceFile.setPath(source);
    QUrl destinationFile(mServiceLocation);
    destinationFile.setPath(destinationFolder);
    copy(sourceFile.toString(), destinationFile.toString(),false);
}

void WebDavJob::copyFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::CopyFolderAction;
    mError = false;
    QString filename;
    if (!source.isEmpty()) {
        QStringList parts = source.split(QLatin1String("/"), QString::SkipEmptyParts);
        filename = parts.takeLast();
    }

    const QString destinationPath = destination + QLatin1Char('/') + filename + QLatin1Char('/');
    QUrl sourceFile(mServiceLocation);
    sourceFile.setPath(source);
    QUrl destinationFile(mServiceLocation);
    destinationFile.setPath(destinationPath);
    copy(sourceFile.toString(), destinationFile.toString(), false);
}

void WebDavJob::deleteFile(const QString &filename)
{
    mActionType = PimCommon::StorageServiceAbstract::DeleteFileAction;
    mError = false;
    QUrl url(mServiceLocation);
    url.setPath(filename);
    remove(url);
}

void WebDavJob::deleteFolder(const QString &foldername)
{
    mActionType = PimCommon::StorageServiceAbstract::DeleteFolderAction;
    mError = false;
    QUrl url(mServiceLocation);
    url.setPath(foldername);
    rmdir(url);
}

void WebDavJob::renameFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::RenameFolderAction;
    mError = false;

    QString destinationFolder;
    if (!source.isEmpty()) {
        QStringList parts = source.split(QLatin1String("/"), QString::SkipEmptyParts);
        parts.removeLast();
        destinationFolder = parts.join(QLatin1String("/"));
        if (destinationFolder.isEmpty()) {
            destinationFolder = QLatin1String("/");
        }
        if (!destinationFolder.endsWith(QLatin1Char('/'))) {
            destinationFolder += QLatin1String("/");
        }
        if (!destinationFolder.startsWith(QLatin1Char('/'))) {
            destinationFolder.prepend(QLatin1String("/"));
        }
        destinationFolder += destination;
        if (!destinationFolder.startsWith(QLatin1Char('/'))) {
            destinationFolder.prepend(QLatin1String("/"));
        }
    }

    QUrl sourceFile(mServiceLocation);
    sourceFile.setPath(source);
    QUrl destinationFile(mServiceLocation);
    destinationFile.setPath(destinationFolder);

    rename(sourceFile.toString(), destinationFile.toString(), false);
}

void WebDavJob::renameFile(const QString &oldName, const QString &newName)
{
    mActionType = PimCommon::StorageServiceAbstract::RenameFileAction;
    mError = false;

    QString destination;
    if (!oldName.isEmpty()) {
        QStringList parts = oldName.split(QLatin1String("/"), QString::SkipEmptyParts);
        parts.removeLast();
        destination = parts.join(QLatin1String("/"));
        if (destination.isEmpty()) {
            destination = QLatin1String("/");
        }
        if (!destination.endsWith(QLatin1Char('/'))) {
            destination += QLatin1String("/");
        }
        if (!destination.startsWith(QLatin1Char('/'))) {
            destination.prepend(QLatin1String("/"));
        }
        destination += newName;
    }
    QUrl sourceFile(mServiceLocation);
    sourceFile.setPath(oldName);
    QUrl destinationFile(mServiceLocation);
    destinationFile.setPath(destination);

    rename(sourceFile.toString(), destinationFile.toString(),false);
}

QNetworkReply *WebDavJob::uploadFile(const QString &filename, const QString &uploadAsName, const QString &destination)
{
    QFile *file = new QFile(filename);
    if (file->exists()) {
        mActionType = PimCommon::StorageServiceAbstract::UploadFileAction;
        mError = false;
        if (file->open(QIODevice::ReadOnly)) {
            QUrl destinationFile(mServiceLocation);
            const QString defaultDestination = (destination.isEmpty() ? destinationFile.path() + QLatin1Char('/') + PimCommon::StorageServiceJobConfig::self()->defaultUploadFolder() : destination);

            QString destinationToString;
            if (defaultDestination.isEmpty()) {
                destinationToString = destinationFile.toString() + QLatin1Char('/') + uploadAsName;
            } else {
                destinationFile.setPath(defaultDestination + QLatin1Char('/') + uploadAsName);
                destinationToString = destinationFile.toString();
            }
            mCacheValue = destinationToString;
            QNetworkReply *reply = put(destinationToString,file);
            file->setParent(reply);
            connect(reply, SIGNAL(uploadProgress(qint64,qint64)), SLOT(slotuploadDownloadFileProgress(qint64,qint64)));
            connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
            return reply;
        }
    }
    delete file;
    return 0;
}

void WebDavJob::listFolder(const QString &folder)
{
    mActionType = PimCommon::StorageServiceAbstract::ListFolderAction;
    mError = false;
    QUrl url(mServiceLocation);
    if (!folder.isEmpty())
        url.setPath(folder);
    list(url.toString());
}

void WebDavJob::moveFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::MoveFolderAction;
    mError = false;
    QString destinationPath;
    if (!source.isEmpty()) {
        QStringList parts = source.split(QLatin1String("/"), QString::SkipEmptyParts);
        const QString folderName = parts.takeLast();

        destinationPath = destination;
        if (!destinationPath.endsWith(QLatin1Char('/'))) {
            destinationPath += QLatin1Char('/');
        }
        destinationPath += folderName;
        if (!destinationPath.endsWith(QLatin1Char('/'))) {
            destinationPath += QLatin1Char('/');
        }
    }
    QUrl sourceFile(mServiceLocation);
    sourceFile.setPath(source);
    QUrl destinationFile(mServiceLocation);
    destinationFile.setPath(destinationPath);

    move(sourceFile.toString(), destinationFile.toString(), false);
}

void WebDavJob::moveFile(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::MoveFileAction;
    mError = false;
    QString destinationPath;
    if (!source.isEmpty()) {
        QStringList parts = source.split(QLatin1String("/"), QString::SkipEmptyParts);
        const QString folderName = parts.takeLast();

        destinationPath = destination;
        if (!destinationPath.endsWith(QLatin1Char('/'))) {
            destinationPath += QLatin1Char('/');
        }
        destinationPath += folderName;
    }
    QUrl sourceFile(mServiceLocation);
    sourceFile.setPath(source);
    QUrl destinationFile(mServiceLocation);
    destinationFile.setPath(destinationPath);

    move(sourceFile.toString(), destinationFile.toString(), false);
}

void WebDavJob::createFolderJob(const QString &foldername, const QString &destination)
{
    mCacheValue = foldername;
    QUrl url(mServiceLocation);
    if (destination.isEmpty())
        url.setPath(url.path() + QLatin1Char('/') + foldername);
    else
        url.setPath(destination + QLatin1Char('/') + foldername);
    //qDebug()<<" url"<<url;
    mkdir(url);
}

void WebDavJob::createFolder(const QString &foldername, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::CreateFolderAction;
    mError = false;
    createFolderJob(foldername, destination);
}

void WebDavJob::slotListInfo(const QString &data)
{
    Q_EMIT listFolderDone(data);
    deleteLater();
}

void WebDavJob::accountInfo()
{
    mActionType = PimCommon::StorageServiceAbstract::AccountInfoAction;
    mError = false;
    QUrl url(mServiceLocation);
    accountInfo(url.toString());
}

void WebDavJob::slotSendDataFinished(QNetworkReply *reply)
{
    if (mError || reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        errorMessage(mActionType, reply->errorString());
        deleteLater();
    } else {
        const QString data = QString::fromUtf8(reply->readAll());
        //qDebug()<<" data "<<data;
        reply->deleteLater();
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
        case PimCommon::StorageServiceAbstract::DeleteFileAction:
            parseDeleteFile(data);
            break;
        case PimCommon::StorageServiceAbstract::DeleteFolderAction:
            parseDeleteFolder(data);
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
        case PimCommon::StorageServiceAbstract::DownLoadFileAction:
            parseDownloadFile(data);
            break;
        case PimCommon::StorageServiceAbstract::ShareLinkAction:
            parseShareLink(data);
            break;
        case PimCommon::StorageServiceAbstract::CreateServiceFolderAction:
            parseCreateServiceFolder(data);
            break;
        }
    }
}

void WebDavJob::parseCreateServiceFolder(const QString &/*data*/)
{
    Q_EMIT createFolderDone(QString());
    deleteLater();
}

void WebDavJob::parseShareLink(const QString &data)
{
    qDebug()<<" data"<<data;
    Q_EMIT shareLinkDone(data);
    deleteLater();
}

void WebDavJob::parseDownloadFile(const QString &data)
{
    Q_UNUSED(data);
    Q_EMIT downLoadFileDone(QString());
    deleteLater();
}

void WebDavJob::parseMoveFolder(const QString &data)
{
    Q_UNUSED(data);
    Q_EMIT moveFolderDone(QString());
    deleteLater();
}

void WebDavJob::parseMoveFile(const QString &data)
{
    Q_UNUSED(data);
    Q_EMIT moveFileDone(QString());
    deleteLater();
}

void WebDavJob::parseCopyFolder(const QString &data)
{
    Q_UNUSED(data);
    Q_EMIT copyFolderDone(QString());
    deleteLater();
}

void WebDavJob::parseCopyFile(const QString &data)
{
    Q_UNUSED(data);
    Q_EMIT copyFileDone(QString());
    deleteLater();
}

void WebDavJob::parseRenameFolder(const QString &data)
{
    Q_UNUSED(data);
    Q_EMIT renameFolderDone(QString());
    deleteLater();
}

void WebDavJob::parseRenameFile(const QString &data)
{
    Q_UNUSED(data);
    Q_EMIT renameFileDone(QString());
    deleteLater();
}

void WebDavJob::parseDeleteFile(const QString &data)
{
    Q_UNUSED(data);
    Q_EMIT deleteFileDone(QString());
    deleteLater();
}

void WebDavJob::parseDeleteFolder(const QString &data)
{
    Q_UNUSED(data);
    Q_EMIT deleteFolderDone(QString());
    deleteLater();
}

void WebDavJob::parseAccessToken(const QString &data)
{
    Q_UNUSED(data);
    Q_EMIT authorizationDone(mPublicLocation, mServiceLocation, mUserName, mPassword);
    deleteLater();
}

void WebDavJob::parseUploadFile(const QString &data)
{
    Q_UNUSED(data);
    Q_EMIT uploadFileDone(QString());
    shareLink(QString(), mCacheValue);
}

void WebDavJob::parseCreateFolder(const QString &data)
{
    Q_UNUSED(data);
    Q_EMIT createFolderDone(mCacheValue);
    deleteLater();
}

void WebDavJob::parseAccountInfo(const QString &data)
{
    //qDebug()<<" parseAccountInfo "<<data;
    PimCommon::AccountInfo accountInfo;
    QDomDocument dom;
    dom.setContent(data.toLatin1(), true);
    for ( QDomNode n = dom.documentElement().firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement thisResponse = n.toElement();
        if (thisResponse.isNull())
            continue;

        QDomElement href = n.namedItem( QLatin1String("href") ).toElement();

        if ( !href.isNull() ) {

            QDomNodeList propstats = thisResponse.elementsByTagName( QLatin1String("propstat") );
            for (int i=0; i<propstats.count(); ++i) {
                QDomNodeList propstat = propstats.item(i).childNodes();
                for (int j=0; j<propstat.count();++j) {
                    QDomElement element = propstat.item(j).toElement();
                    QString tagName = element.tagName();
                    if (tagName == QLatin1String("prop")) {
                        QDomNodeList prop = element.childNodes();
                        for (int t=0; t<prop.count();++t) {
                            const QDomElement propElement = prop.item(t).toElement();
                            tagName = propElement.tagName();
                            if (tagName == QLatin1String("quota-available-bytes")) {
                                bool ok;
                                qlonglong val = propElement.text().toLongLong(&ok);
                                if (ok)
                                    accountInfo.accountSize = val;
                            } else if (tagName == QLatin1String("quota-used-bytes")) {
                                bool ok;
                                qlonglong val = propElement.text().toLongLong(&ok);
                                if (ok)
                                    accountInfo.shared = val;
                            }
                        }
                    }
                }
            }
        }
    }

    Q_EMIT accountInfoDone(accountInfo);
    deleteLater();
}

void WebDavJob::parseListFolder(const QString &data)
{
    Q_UNUSED(data);

    //qDebug()<<" data "<<data;
    Q_EMIT listFolderDone(data);
    deleteLater();
}

void WebDavJob::shareLink(const QString &/*root*/, const QString &path)
{
    mActionType = PimCommon::StorageServiceAbstract::ShareLinkAction;
    mError = false;
    QString filePath(path);
    if (!path.startsWith(mServiceLocation)) {
        QUrl sourceFile(mServiceLocation);
        sourceFile.setPath(path);
        filePath = sourceFile.toString();
    }
    QUrl path2(mServiceLocation);
    QNetworkRequest request(QUrl(path2.scheme() + QLatin1String("://") + path2.host() + mShareApi));
    qDebug()<<" filePath"<<filePath;
    qDebug()<<" url"<<(path2.scheme() + QLatin1String("://") + path2.host() + mShareApi);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    QUrl postData;
    postData.addQueryItem(QLatin1String("path"), filePath);
    postData.addQueryItem(QLatin1String("shareType"), QString::number(3)); //public link
    QNetworkReply *reply = mNetworkAccessManager->post(request, postData.encodedQuery());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void WebDavJob::createServiceFolder()
{
    mActionType = PimCommon::StorageServiceAbstract::CreateServiceFolderAction;
    mError = false;
    createFolderJob(PimCommon::StorageServiceJobConfig::self()->defaultUploadFolder(), QString());
}

QNetworkReply *WebDavJob::downloadFile(const QString &name, const QString &fileId, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::DownLoadFileAction;
    mError = false;
    const QString defaultDestination = (destination.isEmpty() ? PimCommon::StorageServiceJobConfig::self()->defaultUploadFolder() : destination);
    delete mDownloadFile;
    mDownloadFile = new QFile(defaultDestination+ QLatin1Char('/') + name);
    if (mDownloadFile->open(QIODevice::WriteOnly)) {
        QNetworkRequest req;
        QUrl sourceFile(mServiceLocation);
        sourceFile.setPath(fileId);
        req.setUrl(sourceFile);
        QNetworkReply *reply = mNetworkAccessManager->get(req);
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

#include "moc_webdavjob.cpp"
