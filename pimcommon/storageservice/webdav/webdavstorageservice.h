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

#ifndef WEBDAVSTORAGESERVICE_H
#define WEBDAVSTORAGESERVICE_H

#include "pimcommon/storageservice/storageserviceabstract.h"

namespace PimCommon {
class WebDavStorageService : public PimCommon::StorageServiceAbstract
{
    Q_OBJECT
public:
    explicit WebDavStorageService(QObject *parent=0);
    ~WebDavStorageService();

    static QString name();
    static QString description();
    static QUrl serviceUrl();
    static QString serviceName();

    void uploadFile(const QString &filename);
    void accountInfo();
    void createFolder(const QString &folder);
    void listFolder();
    void removeConfig();
    void authentification();

private:
    void readConfig();

};
}

#endif // WEBDAVSTORAGESERVICE_H
