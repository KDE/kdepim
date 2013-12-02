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

#ifndef DROPBOXSTORAGESERVICE_H
#define DROPBOXSTORAGESERVICE_H

#include "pimcommon/storageservice/storageserviceabstract.h"
#include "pimcommon/pimcommon_export.h"
namespace PimCommon {
class DropBoxToken;
class PIMCOMMON_EXPORT DropBoxStorageService : public PimCommon::StorageServiceAbstract
{
    Q_OBJECT
public:
    explicit DropBoxStorageService(QObject *parent=0);
    ~DropBoxStorageService();

    static QString name();
    static QString description();
    static QUrl serviceUrl();
    static QString serviceName();


    void listFolder();
    void uploadFile(const QString &filename);
    void accountInfo();
    void createFolder(const QString &folder);
    void removeConfig();
    void authentification();
    QUrl sharedUrl() const;

private slots:
    void slotAuthorizationDone(const QString &accessToken, const QString &accessTokenSecret, const QString &accessOauthSignature);
    void slotCreateFolderDone();
    void slotUploadFileDone();
    void slotAccountInfoDone(const PimCommon::AccountInfo &info);
    void slotListFolderDone();
    void slotAuthorizationFailed();

    void slotErrorFound(const QString &error);
private:
    void readConfig();
    QString mAccessToken;
    QString mAccessTokenSecret;
    QString mAccessOauthSignature;
};
}

#endif // DROPBOXSTORAGESERVICE_H
