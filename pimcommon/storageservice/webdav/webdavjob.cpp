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

#include "webdavjob.h"
#include "webdavsettingsdialog.h"

#include <KLocalizedString>

#include <qjson/parser.h>

#include <QNetworkAccessManager>
#include <QDebug>
#include <QNetworkReply>
#include <QPointer>

using namespace PimCommon;

WebDavJob::WebDavJob(QObject *parent)
    : PimCommon::StorageServiceAbstractJob(parent)
{
    connect(mNetworkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotSendDataFinished(QNetworkReply*)));
}

WebDavJob::~WebDavJob()
{

}

void WebDavJob::requestTokenAccess()
{
    mError = false;
    QPointer<WebDavSettingsDialog> dlg = new WebDavSettingsDialog;
    if (dlg->exec()) {
        WebDavJob *job = new WebDavJob(this);
        mServiceLocation = dlg->serviceLocation();
        mPublicLocation = dlg->publicLocation();
        job->requestTokenAccess();
    } else {
        Q_EMIT authorizationFailed(i18n("Authentication Canceled."));
        deleteLater();
    }
    delete dlg;
}

void WebDavJob::uploadFile(const QString &filename)
{
    mActionType = UploadFiles;
    mError = false;
    qDebug()<<" not implemented";
    deleteLater();
}

void WebDavJob::listFolder(const QString &folder)
{
    mActionType = ListFolder;
    mError = false;
    qDebug()<<" not implemented";
    deleteLater();
}

void WebDavJob::accountInfo()
{
    mActionType = AccountInfo;
    mError = false;
    qDebug()<<" not implemented";
    deleteLater();
}

void WebDavJob::createFolder(const QString &filename)
{
    mActionType = CreateFolder;
    mError = false;
    qDebug()<<" not implemented";
    deleteLater();
}

void WebDavJob::slotSendDataFinished(QNetworkReply *reply)
{
    const QString data = QString::fromUtf8(reply->readAll());
    reply->deleteLater();
    if (mError) {
        qDebug()<<" error type "<<data;
        QJson::Parser parser;
        bool ok;

        QMap<QString, QVariant> error = parser.parse(data.toUtf8(), &ok).toMap();
        if (error.contains(QLatin1String("error"))) {
            const QString errorStr = error.value(QLatin1String("error")).toString();
            switch(mActionType) {
            case NoneAction:
                deleteLater();
                break;
            case RequestToken:
                Q_EMIT authorizationFailed(errorStr);
                deleteLater();
                break;
            case AccessToken:
                Q_EMIT authorizationFailed(errorStr);
                deleteLater();
                break;
            case UploadFiles:
            case CreateFolder:
            case AccountInfo:
            case ListFolder:
            case DownLoadFile:
            case CreateServiceFolder:
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
    case NoneAction:
        deleteLater();
        break;
    case RequestToken:
        deleteLater();
        break;
    case AccessToken:
        deleteLater();
        break;
    case UploadFiles:
        parseUploadFiles(data);
        break;
    case CreateFolder:
        parseCreateFolder(data);
        break;
    case AccountInfo:
        parseAccountInfo(data);
        break;
    case ListFolder:
        parseListFolder(data);
        break;
    case DownLoadFile:
        deleteLater();
        break;
    default:
        qDebug()<<" Action Type unknown:"<<mActionType;
        deleteLater();
    }
}

void WebDavJob::parseUploadFiles(const QString &data)
{
    qDebug()<<" data "<<data;
    deleteLater();
}

void WebDavJob::parseCreateFolder(const QString &data)
{
    qDebug()<<" data "<<data;
    deleteLater();
}

void WebDavJob::parseAccountInfo(const QString &data)
{
    qDebug()<<" data "<<data;
    deleteLater();
}

void WebDavJob::parseListFolder(const QString &data)
{
    qDebug()<<" data "<<data;
    deleteLater();
}


void WebDavJob::shareLink(const QString &root, const QString &path)
{
    mActionType = ShareLink;
    mError = false;
    qDebug()<<" not implemented";
    deleteLater();
}

void WebDavJob::createServiceFolder()
{

}

void WebDavJob::downloadFile(const QString &filename)
{

}
