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
    : PimCommon::StorageServiceAbstractJob(parent)
{
    connect(mNetworkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotSendDataFinished(QNetworkReply*)));
    connect(mNetworkAccessManager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), SLOT(slotAuthenticationRequired(QNetworkReply*,QAuthenticator*)));
}

WebDavJob::~WebDavJob()
{

}

void WebDavJob::initializeToken(const QString &publicLocation, const QString &serviceLocation, const QString &username, const QString &password)
{
    mError = false;
    mUserName = username;
    mPassword = password;
    mPublicLocation = publicLocation;
    mServiceLocation = serviceLocation;
}

void WebDavJob::slotAuthenticationRequired(QNetworkReply *,QAuthenticator *auth)
{
    if (mUserName.isEmpty() || mPassword.isEmpty()) {
        QPointer<LoginDialog> dlg = new LoginDialog;
        dlg->setCaption(i18n("WebDav"));
        if (dlg->exec()) {
            mUserName = dlg->username();
            mPassword = dlg->password();
            auth->setUser(mUserName);
            auth->setPassword(mPassword);
        } else {
            Q_EMIT authorizationFailed(i18n("Authentication Canceled."));
            deleteLater();
        }
        delete dlg;
    } else {
        auth->setUser(mUserName);
        auth->setPassword(mPassword);
    }
}

void WebDavJob::requestTokenAccess()
{
    mError = false;
    mActionType = PimCommon::StorageServiceAbstract::AccessToken;
    QPointer<WebDavSettingsDialog> dlg = new WebDavSettingsDialog;
    if (dlg->exec()) {
        mServiceLocation = dlg->serviceLocation();
        mPublicLocation = dlg->publicLocation();
    } else {
        Q_EMIT authorizationFailed(i18n("Authentication Canceled."));
        deleteLater();
    }
    delete dlg;
    QUrl url(mServiceLocation);
    QNetworkRequest request(url);
    QNetworkReply *reply = mNetworkAccessManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void WebDavJob::copyFile(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::CopyFile;
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
    mActionType = PimCommon::StorageServiceAbstract::CopyFolder;
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
    mActionType = PimCommon::StorageServiceAbstract::DeleteFile;
    mError = false;
    QUrl url(mServiceLocation);
    url.setPath(filename);
    remove(url.toString());
}

void WebDavJob::deleteFolder(const QString &foldername)
{
    mActionType = PimCommon::StorageServiceAbstract::DeleteFolder;
    mError = false;
    QUrl url(mServiceLocation);
    url.setPath(foldername);
    rmdir(url.toString());
}

void WebDavJob::renameFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::RenameFolder;
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
    mActionType = PimCommon::StorageServiceAbstract::RenameFile;
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
    mActionType = PimCommon::StorageServiceAbstract::UploadFile;
    mError = false;
    QFile *file = new QFile(filename);
    if (file->exists()) {
        mActionType = PimCommon::StorageServiceAbstract::UploadFile;
        mError = false;
        if (file->open(QIODevice::ReadOnly)) {
            //TODO fix default value
            const QString defaultDestination = (destination.isEmpty() ? PimCommon::StorageServiceJobConfig::self()->defaultUploadFolder() : destination);
            QUrl destinationFile(mServiceLocation);
            destinationFile.setPath(defaultDestination + QLatin1Char('/') + uploadAsName);

            QNetworkReply *reply = put(destinationFile.toString(),file);
            file->setParent(reply);
            connect(reply, SIGNAL(uploadProgress(qint64,qint64)), SLOT(slotuploadDownloadFileProgress(qint64,qint64)));
            connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
            return reply;
        } else {
            delete file;
        }
    }
    return 0;
}

void WebDavJob::listFolder(const QString &folder)
{
    mActionType = PimCommon::StorageServiceAbstract::ListFolder;
    mError = false;
    qDebug()<<" folder"<<folder;
    QUrl url(mServiceLocation);
    if (!folder.isEmpty())
        url.setPath(folder);
    qDebug()<<" url.toString()"<<url.toString();
    list(url.toString());
}

void WebDavJob::moveFolder(const QString &source, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::MoveFolder;
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
    mActionType = PimCommon::StorageServiceAbstract::MoveFile;
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


void WebDavJob::createFolder(const QString &foldername, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::CreateFolder;
    mError = false;
    QUrl url(mServiceLocation);
    if (!destination.isEmpty())
        url.setPath(destination + QLatin1Char('/') + foldername);
    else
        url.setPath(url.path() + QLatin1Char('/') + foldername);
    //qDebug()<<" url"<<url;
    mkdir(url.toString());
}

void WebDavJob::slotListInfo(const QString &data)
{
    Q_EMIT listFolderDone(data);
    deleteLater();
}

void WebDavJob::accountInfo()
{
    mActionType = PimCommon::StorageServiceAbstract::AccountInfo;
    mError = false;
    QUrl url(mServiceLocation);
    accountInfo(url.toString());
}

void WebDavJob::slotSendDataFinished(QNetworkReply *reply)
{
    if (mError || reply->error() != QNetworkReply::NoError) {
        qDebug()<<" ERROR "<<reply->error();
        reply->deleteLater();
        errorMessage(mActionType, reply->errorString());
        //ADD more parsing
        deleteLater();
    } else {
        const QString data = QString::fromUtf8(reply->readAll());
        qDebug()<<" data "<<data;
        reply->deleteLater();
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
        case PimCommon::StorageServiceAbstract::DeleteFile:
            parseDeleteFile(data);
            break;
        case PimCommon::StorageServiceAbstract::DeleteFolder:
            parseDeleteFolder(data);
            break;
        case PimCommon::StorageServiceAbstract::RenameFolder:
            parseRenameFolder(data);
            break;
        case PimCommon::StorageServiceAbstract::RenameFile:
            parseRenameFile(data);
            break;
        case PimCommon::StorageServiceAbstract::MoveFolder:
            parseMoveFolder(data);
            break;
        case PimCommon::StorageServiceAbstract::MoveFile:
            parseMoveFile(data);
            break;
        case PimCommon::StorageServiceAbstract::CopyFile:
            parseCopyFile(data);
            break;
        case PimCommon::StorageServiceAbstract::CopyFolder:
            parseCopyFolder(data);
            break;
        case PimCommon::StorageServiceAbstract::DownLoadFile:
            parseDownloadFile(data);
            break;
        case PimCommon::StorageServiceAbstract::ShareLink:
        case PimCommon::StorageServiceAbstract::CreateServiceFolder:

            deleteLater();
            break;
        }
    }
}

void WebDavJob::parseDownloadFile(const QString &data)
{
    Q_EMIT downLoadFileDone(QString());
    deleteLater();
}

void WebDavJob::parseMoveFolder(const QString &data)
{
    Q_EMIT moveFolderDone(QString());
    deleteLater();
}

void WebDavJob::parseMoveFile(const QString &data)
{
    Q_EMIT moveFileDone(QString());
    deleteLater();
}

void WebDavJob::parseCopyFolder(const QString &data)
{
    Q_EMIT copyFolderDone(QString());
    deleteLater();
}

void WebDavJob::parseCopyFile(const QString &data)
{
    Q_EMIT copyFileDone(QString());
    deleteLater();
}

void WebDavJob::parseRenameFolder(const QString &data)
{
    Q_EMIT renameFolderDone(QString());
    deleteLater();
}

void WebDavJob::parseRenameFile(const QString &data)
{
    Q_EMIT renameFileDone(QString());
    deleteLater();
}

void WebDavJob::parseDeleteFile(const QString &data)
{
    Q_EMIT deleteFileDone(QString());
    deleteLater();
}

void WebDavJob::parseDeleteFolder(const QString &data)
{
    Q_EMIT deleteFolderDone(QString());
    deleteLater();
}

void WebDavJob::parseAccessToken(const QString &data)
{
    qDebug()<<" void WebDavJob::parseAccessToken(const QString &data)"<<data;
    Q_EMIT authorizationDone(mPublicLocation, mServiceLocation, mUserName, mPassword);
    deleteLater();
}

void WebDavJob::parseUploadFile(const QString &data)
{
    qDebug()<<" data "<<data;
    Q_EMIT uploadFileDone(QString());
    deleteLater();
}

void WebDavJob::parseCreateFolder(const QString &data)
{
    qDebug()<<" data "<<data;
    Q_EMIT createFolderDone(QString());
    deleteLater();
}

void WebDavJob::parseAccountInfo(const QString &data)
{
    PimCommon::AccountInfo accountInfo;
    QDomDocument dom;
    dom.setContent(data.toLatin1(), true);
    for ( QDomNode n = dom.documentElement().firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement thisResponse = n.toElement();
        qDebug()<<"thisResponse "<<thisResponse.tagName();
        if (thisResponse.isNull())
            continue;

        QDomElement href = n.namedItem( QLatin1String("href") ).toElement();

        if ( !href.isNull() ) {

            QDomNodeList propstats = thisResponse.elementsByTagName( QLatin1String("propstat") );
            for (int i=0; i<propstats.count(); ++i) {
                QDomNodeList propstat = propstats.item(i).childNodes();
                for (int j=0; j<propstat.count();++j) {
                    QDomElement element = propstat.item(j).toElement();
                    qDebug()<<" element.tag"<<element.tagName();
                    const QString tagName = element.tagName();
                    if (tagName == QLatin1String("quota-available-bytes")) {
                        accountInfo.accountSize = element.text().toLongLong();
                    } else if (tagName == QLatin1String("quota-used-bytes")) {
                        accountInfo.quota = element.text().toLongLong();
                    }
                }
                qDebug()<<" propstat.tagName() :"<<propstat.count();
            }
        }
    }

    Q_EMIT accountInfoDone(accountInfo);
    deleteLater();
}

void WebDavJob::parseListFolder(const QString &data)
{
    //qDebug()<<" data "<<data;
    Q_EMIT listFolderDone(data);
    deleteLater();
}


void WebDavJob::shareLink(const QString &root, const QString &path)
{
    mActionType = PimCommon::StorageServiceAbstract::ShareLink;
    mError = false;
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    qDebug()<<" not implemented";
    deleteLater();
}

void WebDavJob::createServiceFolder()
{
    mActionType = PimCommon::StorageServiceAbstract::CreateServiceFolder;
    mError = false;
    Q_EMIT actionFailed(QLatin1String("Not Implemented"));
    qDebug()<<" not implemented";
    deleteLater();
}

QNetworkReply *WebDavJob::downloadFile(const QString &name, const QString &fileId, const QString &destination)
{
    mActionType = PimCommon::StorageServiceAbstract::DownLoadFile;
    mError = false;

#if 0
    const QString defaultDestination = (destination.isEmpty() ? PimCommon::StorageServiceJobConfig::self()->defaultUploadFolder() : destination);
    delete mDownloadFile;
    mDownloadFile = new QFile(defaultDestination+ QLatin1Char('/') + name);
    if (mDownloadFile->open(QIODevice::WriteOnly)) {
        QUrl url;
        QNetworkReply *reply = getenv();
        mDownloadFile->setParent(reply);
        connect(reply, SIGNAL(readyRead()), this, SLOT(slotDownloadReadyRead()));
        connect(reply, SIGNAL(downloadProgress(qint64,qint64)), SLOT(slotuploadDownloadFileProgress(qint64,qint64)));
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
        return reply;
    } else {
        delete mDownloadFile;
    }
#endif
    return 0;
}

QNetworkReply *WebDavJob::accountInfo(const QString &dir)
{
    WebDavJob::PropNames query;
    QStringList props;

    props << QLatin1String("quota-available-bytes");
    props << QLatin1String("quota-used-bytes");
    query[QLatin1String("DAV:")] = props;

    return propfind(dir, query, 0);
}

QNetworkReply *WebDavJob::list ( const QString & dir)
{
    WebDavJob::PropNames query;
    QStringList props;

    props << QLatin1String("creationdate");
    props << QLatin1String("getcontentlength");
    props << QLatin1String("displayname");
    props << QLatin1String("source");
    props << QLatin1String("getcontentlanguage");
    props << QLatin1String("getcontenttype");
    props << QLatin1String("executable");
    props << QLatin1String("getlastmodified");
    props << QLatin1String("getetag");
    props << QLatin1String("resourcetype");
    query[QLatin1String("DAV:")] = props;

    return propfind(dir, query, 1);
}

QNetworkReply *WebDavJob::search ( const QString & path, const QString & q )
{
    QByteArray query = "<?xml version=\"1.0\"?>\r\n";

    query.append( "<D:searchrequest xmlns:D=\"DAV:\">\r\n" );
    query.append( q.toUtf8() );
    query.append( "</D:searchrequest>\r\n" );

    QNetworkRequest req;
    req.setUrl(path);

    return davRequest(QLatin1String("SEARCH"), req);
}

QNetworkReply *WebDavJob::put ( const QString & path, QIODevice * data )
{
    QNetworkRequest req;
    req.setUrl(QUrl(path));

    return davRequest(QLatin1String("PUT"), req, data);
}

QNetworkReply *WebDavJob::put ( const QString & path, QByteArray & data )
{
    QBuffer buffer(&data);

    return put(path, &buffer);
}

QNetworkReply *WebDavJob::propfind( const QString &path, const WebDavJob::PropNames & props, int depth)
{
    QByteArray query;

    query = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>";
    query += "<D:propfind xmlns:D=\"DAV:\" >";
    query += "<D:prop>";
    foreach (const QString &ns, props.keys())
    {
        foreach (const QString &key, props[ns])
            if (ns == QLatin1String("DAV:"))
                query += "<D:" + key.toLatin1() + "/>";
            else
                query += "<" + key.toLatin1() + " xmlns=\"" + ns.toLatin1() + "\"/>";
    }
    query += "</D:prop>";
    query += "</D:propfind>";
    return propfind(path, query, depth);
}

QNetworkReply *WebDavJob::propfind( const QString & path, const QByteArray & query, int depth )
{
    QNetworkRequest req;
    QUrl url(path);
    req.setUrl(url);

    QString value;

    if (depth == 2)
        value = QLatin1String("infinity");
    else
        value = QString::fromLatin1("%1").arg(depth);
    req.setRawHeader(QByteArray("Depth"), value.toUtf8());
    return davRequest(QLatin1String("PROPFIND"), req, query);
}

QNetworkReply *WebDavJob::proppatch( const QString & path, const WebDavJob::PropValues & props)
{
    QByteArray query;

    query = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>";
    query += "<D:proppatch xmlns:D=\"DAV:\" >";
    query += "<D:prop>";
    foreach (const QString &ns, props.keys())
    {
        QMap < QString , QVariant >::const_iterator i;

        for (i = props[ns].constBegin(); i != props[ns].constEnd(); ++i) {
            if (ns == QLatin1String("DAV:")) {
                query += "<D:" + i.key().toLatin1() + ">";
                query += i.value().toString().toLatin1();
                query += "</D:" + i.key().toLatin1() + ">" ;
            } else {
                query += "<" + i.key().toLatin1() + " xmlns=\"" + ns.toLatin1() + "\">";
                query += i.value().toString().toLatin1();
                query += "</" + i.key().toLatin1() + " xmlns=\"" + ns.toLatin1() + "\"/>";
            }
        }
    }
    query += "</D:prop>";
    query += "</D:propfind>";

    return proppatch(path, query);
}

QNetworkReply *WebDavJob::proppatch( const QString & path, const QByteArray & query)
{
    QNetworkRequest req;
    req.setUrl(QUrl(path));

    return davRequest(QLatin1String("PROPPATCH"), req, query);
}

void WebDavJob::setupHeaders(QNetworkRequest & req, quint64 size)
{
    req.setRawHeader(QByteArray("Connection"), QByteArray("Keep-Alive"));
    if (size > 0) {
        req.setHeader(QNetworkRequest::ContentLengthHeader, QVariant(size));
        req.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(QLatin1String("text/xml; charset=utf-8")));
    }
}

QNetworkReply *WebDavJob::davRequest(const QString & reqVerb,  QNetworkRequest & req, const QByteArray & data)
{
    QByteArray dataClone(data);
    QBuffer *buffer = new QBuffer(this);
    buffer->setData(dataClone);
    return davRequest(reqVerb, req, buffer);
}

QNetworkReply *WebDavJob::davRequest(const QString & reqVerb,  QNetworkRequest & req, QIODevice * data)
{
    setupHeaders(req, data->size());
    return mNetworkAccessManager->sendCustomRequest(req, reqVerb.toUtf8(), data);
}

QNetworkReply *WebDavJob::mkdir ( const QString & dir )
{
    QNetworkRequest req;
    req.setUrl(QUrl(dir));
    return davRequest(QLatin1String("MKCOL"), req);
}

QNetworkReply *WebDavJob::copy ( const QString & oldname, const QString & newname, bool overwrite)
{
    QNetworkRequest req;

    req.setUrl(QUrl(oldname));
    req.setRawHeader(QByteArray("Destination"), newname.toUtf8());
    req.setRawHeader(QByteArray("Depth"), QByteArray("infinity"));
    req.setRawHeader(QByteArray("Overwrite"), QByteArray(overwrite ? "T" : "F"));
    return davRequest(QLatin1String("COPY"), req);
}

QNetworkReply *WebDavJob::rename ( const QString & oldname, const QString & newname, bool overwrite)
{
    return move(oldname, newname, overwrite);
}

QNetworkReply *WebDavJob::move ( const QString & oldname, const QString & newname, bool overwrite)
{
    QNetworkRequest req;
    req.setUrl(QUrl(oldname));

    req.setRawHeader(QByteArray("Destination"), newname.toUtf8());
    req.setRawHeader(QByteArray("Depth"), QByteArray("infinity"));
    req.setRawHeader(QByteArray("Overwrite"), QByteArray(overwrite ? "T" : "F"));
    return davRequest(QLatin1String("MOVE"), req);
}

QNetworkReply *WebDavJob::rmdir ( const QString & dir )
{
    return remove(dir);
}

QNetworkReply *WebDavJob::remove ( const QString & path )
{
    QNetworkRequest req;
    req.setUrl(QUrl(path));
    return davRequest(QLatin1String("DELETE"), req);
}

#include "moc_webdavjob.cpp"
