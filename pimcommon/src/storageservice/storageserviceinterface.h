/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#ifndef STORAGESERVICEINTERFACE_H
#define STORAGESERVICEINTERFACE_H

#include <QObject>

#include "pimcommon_export.h"
namespace PimCommon
{
class PIMCOMMON_EXPORT StorageServiceInterface : public QObject
{
    Q_OBJECT
public:
    explicit StorageServiceInterface(QObject *parent = Q_NULLPTR);
    ~StorageServiceInterface();

    virtual void downloadFile(const QString &name, const QString &fileId, const QString &destination);
    virtual void uploadFile(const QString &filename, const QString &uploadAsName, const QString &destination);
    virtual void accountInfo();
    virtual void createFolder(const QString &foldername, const QString &destination);
    virtual void listFolder(const QString &folder = QString());
    virtual void authentication();
    virtual void shareLink(const QString &root, const QString &path);
    virtual void createServiceFolder();
    virtual void deleteFile(const QString &filename);
    virtual void deleteFolder(const QString &foldername);
    virtual void renameFolder(const QString &source, const QString &destination);
    virtual void renameFile(const QString &source, const QString &destination);
    virtual void moveFile(const QString &source, const QString &destination);
    virtual void moveFolder(const QString &source, const QString &destination);
    virtual void copyFile(const QString &source, const QString &destination);
    virtual void copyFolder(const QString &source, const QString &destination);
    virtual void shutdownService() = 0;

    virtual bool isConfigurated() const = 0;
    virtual void removeConfig();
};
}
#endif // STORAGESERVICEINTERFACE_H
