/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#ifndef GDriveJob_H
#define GDriveJob_H

#include <QObject>
#include "storageservice/oauth2job.h"
namespace PimCommon {
class GDriveJob : public PimCommon::OAuth2Job
{
    Q_OBJECT
public:
    explicit GDriveJob(QObject *parent=0);
    ~GDriveJob();

    void deleteFile(const QString &filename);
    void deleteFolder(const QString &foldername);

    void renameFolder(const QString &source, const QString &destination);
    void renameFile(const QString &oldName, const QString &newName);
    void moveFolder(const QString &source, const QString &destination);
    void moveFile(const QString &source, const QString &destination);
    QNetworkReply *uploadFile(const QString &filename, const QString &destination);
    void listFolder(const QString &folder);
    void accountInfo();
    void createFolder(const QString &foldername, const QString &destination);
    void shareLink(const QString &fileId);
    void shareLink(const QString &root, const QString &path);
    void copyFile(const QString &source, const QString &destination);
    void copyFolder(const QString &source, const QString &destination);

private:
    void parseDeleteFolder(const QString &data);
    void parseAccountInfo(const QString &data);
    void parseDeleteFile(const QString &data);
    void parseCreateServiceFolder(const QString &data);
    void parseListFolder(const QString &data);
    void parseCreateFolder(const QString &data);
    void parseUploadFile(const QString &data);
};
}

#endif // GDriveJob_H
