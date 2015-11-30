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

#ifndef STORAGESERVICEPLUGIN_H
#define STORAGESERVICEPLUGIN_H

#include <QObject>
#include "pimcommon_export.h"
namespace PimCommon
{
class StorageServiceInterface;
class PIMCOMMON_EXPORT StorageServicePlugin : public QObject
{
    Q_OBJECT
public:
    explicit StorageServicePlugin(QObject *parent = Q_NULLPTR);
    ~StorageServicePlugin();
    enum Capability {
        NoCapability = 0,
        //Account
        AccountInfoCapability = 1,
        //File
        UploadFileCapability = 2,
        DeleteFileCapability = 4,
        DownloadFileCapability = 8,
        RenameFileCapabilitity = 16,
        MoveFileCapability = 32,
        CopyFileCapability = 64,
        //Folder
        CreateFolderCapability = 128,
        DeleteFolderCapability = 256,
        ListFolderCapability = 512,
        RenameFolderCapability = 1024,
        MoveFolderCapability = 2048,
        CopyFolderCapability = 4096,
        //Share
        ShareLinkCapability = 8192
    };

    virtual QString storageServiceName() const = 0;

    Q_ENUMS(Capability)
    Q_DECLARE_FLAGS(Capabilities, Capability)

    virtual StorageServicePlugin::Capabilities capabilities() const = 0;
    virtual QIcon icon() const;
    virtual QString description() const = 0;
    virtual QUrl serviceUrl() const = 0;

    virtual QString disallowedSymbols() const;
    virtual QString disallowedSymbolsStr() const;
    virtual qlonglong maximumUploadFileSize() const;

    virtual PimCommon::StorageServiceInterface *createStorageService(const QString &identifier) = 0;
};
}

#endif // STORAGESERVICEPLUGIN_H
