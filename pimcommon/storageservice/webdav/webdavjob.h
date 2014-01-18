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
#include "storageservice/storageserviceabstractjob.h"
class QNetworkReply;
namespace PimCommon {
class WebDavJob : public PimCommon::StorageServiceAbstractJob
{
    Q_OBJECT
public:
    explicit WebDavJob(QObject *parent=0);
    ~WebDavJob();

    void requestTokenAccess();
    void uploadFile(const QString &filename, const QString &destination);
    void listFolder(const QString &folder = QString());
    void accountInfo();
    void createFolder(const QString &foldername, const QString &destination);
    void shareLink(const QString &root, const QString &path);
    void createServiceFolder();
    void downloadFile(const QString &filename, const QString &destination);
    void deleteFile(const QString &filename);
    void deleteFolder(const QString &foldername);
    void renameFolder(const QString &source, const QString &destination);
    void renameFile(const QString &oldName, const QString &newName);
    void moveFolder(const QString &source, const QString &destination);
    void moveFile(const QString &source, const QString &destination);    
    void copyFile(const QString &source, const QString &destination);
    void copyFolder(const QString &source, const QString &destination);

private slots:
    void slotSendDataFinished(QNetworkReply *reply);

private:
    void parseUploadFile(const QString &data);
    void parseCreateFolder(const QString &data);
    void parseAccountInfo(const QString &data);
    void parseListFolder(const QString &data);
    QString mPublicLocation;
    QString mServiceLocation;
};
}

#endif // WEBDAVJOB_H
