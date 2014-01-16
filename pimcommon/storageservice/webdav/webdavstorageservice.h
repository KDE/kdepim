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

#ifndef WEBDAVSTORAGESERVICE_H
#define WEBDAVSTORAGESERVICE_H

#include "pimcommon/storageservice/storageserviceabstract.h"
#include "pimcommon_export.h"

namespace PimCommon {
class PIMCOMMON_EXPORT WebDavStorageService : public PimCommon::StorageServiceAbstract
{
    Q_OBJECT
public:
    explicit WebDavStorageService(QObject *parent=0);
    ~WebDavStorageService();

    static QString name();
    static QString description();
    static QUrl serviceUrl();
    static QString serviceName();
    static QString iconName();
    static StorageServiceAbstract::Capabilities serviceCapabilities();

    void storageServiceuploadFile(const QString &filename);
    void storageServiceaccountInfo();
    void storageServicecreateFolder(const QString &folder);
    void storageServicelistFolder(const QString &folder);
    void removeConfig();
    void storageServiceauthentication();
    void storageServiceShareLink(const QString &root, const QString &path);
    void storageServicedownloadFile(const QString &filename);
    void storageServicecreateServiceFolder();
    void storageServicedeleteFile(const QString &filename);
    void storageServicedeleteFolder(const QString &foldername);
    void storageServiceRenameFolder(const QString &source, const QString &destination);
    void storageServiceRenameFile(const QString &source, const QString &destination);
    void storageServiceMoveFolder(const QString &source, const QString &destination);
    void storageServiceMoveFile(const QString &source, const QString &destination);
    void storageServiceCopyFile(const QString &source, const QString &destination);

    StorageServiceAbstract::Capabilities capabilities() const;
    void fillListWidget(StorageServiceTreeWidget *listWidget, const QString &data);


    QString storageServiceName() const;
    KIcon icon() const;

private:
    void readConfig();
    QString mPublicLocation;
    QString mServiceLocation;
};
}

#endif // WEBDAVSTORAGESERVICE_H
