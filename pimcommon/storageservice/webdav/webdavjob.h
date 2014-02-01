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

#ifndef WEBDAVJOB_H
#define WEBDAVJOB_H

#include <QObject>
#include "storageservice/job/storageserviceabstractjob.h"
class QNetworkReply;
namespace PimCommon {
class QWebdav;
class WebDavJob : public PimCommon::StorageServiceAbstractJob
{
    Q_OBJECT
public:
    explicit WebDavJob(QObject *parent=0);
    ~WebDavJob();

    void requestTokenAccess();
    QNetworkReply *uploadFile(const QString &filename, const QString &uploadAsName, const QString &destination);
    void listFolder(const QString &folder = QString());
    void accountInfo();
    void createFolder(const QString &foldername, const QString &destination);
    void shareLink(const QString &root, const QString &path);
    void createServiceFolder();
    QNetworkReply *downloadFile(const QString &name, const QString &fileId, const QString &destination);
    void deleteFile(const QString &filename);
    void deleteFolder(const QString &foldername);
    void renameFolder(const QString &source, const QString &destination);
    void renameFile(const QString &oldName, const QString &newName);
    void moveFolder(const QString &source, const QString &destination);
    void moveFile(const QString &source, const QString &destination);    
    void copyFile(const QString &source, const QString &destination);
    void copyFolder(const QString &source, const QString &destination);    
    void initializeToken(const QString &publicLocation, const QString &serviceLocation, const QString &username, const QString &password);

Q_SIGNALS:
    void authorizationDone(const QString &publicLocation, const QString &serviceLocation, const QString &username, const QString &password);

private slots:
    void slotSendDataFinished(QNetworkReply *reply);    
    void slotAuthenticationRequired(QNetworkReply *, QAuthenticator *);
    void slotListInfo(const QString &data);
    void slotRequired(const QString &hostname, quint16 port, QAuthenticator *authenticator);
    void slotRequestFinished(int, bool);
private:
    QUrl configureWebDav(QWebdav *webdav);
    void parseUploadFile(const QString &data);
    void parseCreateFolder(const QString &data);
    void parseAccountInfo(const QString &data);
    void parseListFolder(const QString &data);
    void parseAccessToken(const QString &data);
    QString mPublicLocation;
    QString mServiceLocation;
    QString mUserName;
    QString mPassword;
    int mReqId;
};
}

#endif // WEBDAVJOB_H
