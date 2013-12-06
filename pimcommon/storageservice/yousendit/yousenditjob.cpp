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
#include <qjson/parser.h>

#include <QNetworkAccessManager>
#include <QDebug>
#include <QNetworkReply>
#include <QPointer>

using namespace PimCommon;

YouSendItJob::YouSendItJob(QObject *parent)
    : PimCommon::StorageServiceAbstractJob(parent)
{
    mApiKey = QLatin1String("...");
    //TODO adapt api
    mDefaultUrl = QLatin1String("https://test2-api.yousendit.com/dpi/v1/");
    connect(mNetworkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotSendDataFinished(QNetworkReply*)));
}

YouSendItJob::~YouSendItJob()
{

}

void YouSendItJob::initializeToken(const QString &password, const QString &userName, const QString &token)
{
    mPassword = password;
    mUsername = userName;
    mToken = token;
}

void YouSendItJob::requestTokenAccess()
{
    QPointer<LoginDialog> dlg = new LoginDialog;
    if (dlg->exec()) {
        mPassword = dlg->password();
        mUsername = dlg->username();
    }
    delete dlg;

    QUrl url(QLatin1String("https://dpi.yousendit.com/dpi/v1/auth"));
}

void YouSendItJob::uploadFile(const QString &filename)
{

}

void YouSendItJob::listFolder()
{

}

void YouSendItJob::accountInfo()
{

}

void YouSendItJob::createFolder(const QString &filename)
{

}


void YouSendItJob::slotSendDataFinished(QNetworkReply *reply)
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
                deleteLater();
                break;
            case AccessToken:
                deleteLater();
                break;
            case UploadFiles:
                deleteLater();
                break;
            case CreateFolder:
                deleteLater();
                break;
            case AccountInfo:
                deleteLater();
                break;
            case ListFolder:
                deleteLater();
                break;
            default:
                qDebug()<<" Action Type unknown:"<<mActionType;
                deleteLater();
                break;
            }
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
        deleteLater();
        break;
    case CreateFolder:
        deleteLater();
        break;
    case AccountInfo:
        deleteLater();
        break;
    case ListFolder:
        deleteLater();
        break;
    default:
        qDebug()<<" Action Type unknown:"<<mActionType;
        deleteLater();
    }
}


void PimCommon::YouSendItJob::shareLink(const QString &root, const QString &path)
{

}
