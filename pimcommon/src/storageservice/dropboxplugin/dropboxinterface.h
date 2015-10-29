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

#ifndef DROPBOXINTERFACE_H
#define DROPBOXINTERFACE_H

#include <storageservice/storageserviceinterface.h>

namespace PimCommon
{
class DropBoxPlugin;
class DropBoxInterface : public PimCommon::StorageServiceInterface
{
    Q_OBJECT
public:
    explicit DropBoxInterface(DropBoxPlugin *plugin, QObject *parent = Q_NULLPTR);
    ~DropBoxInterface();
    void shutdownService() Q_DECL_OVERRIDE;
    bool isConfigurated() const Q_DECL_OVERRIDE;
    void downloadFile(const QString &name, const QString &fileId, const QString &destination) Q_DECL_OVERRIDE;
    void uploadFile(const QString &filename, const QString &uploadAsName, const QString &destination) Q_DECL_OVERRIDE;
    void accountInfo() Q_DECL_OVERRIDE;
    void createFolder(const QString &foldername, const QString &destination) Q_DECL_OVERRIDE;
    void listFolder(const QString &folder) Q_DECL_OVERRIDE;
    void authentication() Q_DECL_OVERRIDE;
    void shareLink(const QString &root, const QString &path) Q_DECL_OVERRIDE;
    void createServiceFolder() Q_DECL_OVERRIDE;
    void deleteFile(const QString &filename) Q_DECL_OVERRIDE;
    void deleteFolder(const QString &foldername) Q_DECL_OVERRIDE;
    void renameFolder(const QString &source, const QString &destination) Q_DECL_OVERRIDE;
    void renameFile(const QString &source, const QString &destination) Q_DECL_OVERRIDE;
    void moveFile(const QString &source, const QString &destination) Q_DECL_OVERRIDE;
    void moveFolder(const QString &source, const QString &destination) Q_DECL_OVERRIDE;
    void copyFile(const QString &source, const QString &destination) Q_DECL_OVERRIDE;
    void copyFolder(const QString &source, const QString &destination) Q_DECL_OVERRIDE;
    void removeConfig() Q_DECL_OVERRIDE;

private:
    DropBoxPlugin *mPlugin;
};
}
#endif // DROPBOXINTERFACE_H
