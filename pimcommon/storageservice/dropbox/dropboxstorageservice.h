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

#ifndef DROPBOXSTORAGESERVICE_H
#define DROPBOXSTORAGESERVICE_H

#include "pimcommon/storageservice/storageserviceabstract.h"
#include "pimcommon/pimcommon_export.h"
namespace PimCommon {
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
    static QString iconName();
    static StorageServiceAbstract::Capabilities serviceCapabilities();


    void storageServicelistFolder(const QString &folder);
    void storageServiceuploadFile(const QString &filename, const QString &uploadAsName, const QString &destination = QString());
    void storageServiceaccountInfo();
    void storageServicecreateFolder(const QString &folder, const QString &destination = QString());
    void removeConfig();
    void storageServiceauthentication();
    void storageServiceShareLink(const QString &root, const QString &path);
    void storageServicecreateServiceFolder();
    QString storageServiceName() const;
    void storageServicedownloadFile(const QString &name, const QString &fileId, const QString &destination);
    void storageServicedeleteFile(const QString &filename);
    void storageServicedeleteFolder(const QString &foldername);
    void storageServiceRenameFolder(const QString &source, const QString &destination);
    void storageServiceRenameFile(const QString &source, const QString &destination);
    void storageServiceMoveFolder(const QString &source, const QString &destination);
    void storageServiceMoveFile(const QString &source, const QString &destination);
    void storageServiceCopyFile(const QString &source, const QString &destination);
    void storageServiceCopyFolder(const QString &source, const QString &destination);
    QString fileIdentifier(const QVariantMap &variantMap);
    QString fileShareRoot(const QVariantMap &variantMap);
    StorageServiceAbstract::Capabilities capabilities() const;
    QString fillListWidget(StorageServiceTreeWidget *listWidget, const QVariant &data, const QString &currentFolder);
    QString itemInformation(const QVariantMap &variantMap);

    KIcon icon() const;
    QRegExp disallowedSymbols() const;
    QString disallowedSymbolsStr() const;
    qlonglong maximumUploadFileSize() const;

private slots:
    void slotAuthorizationDone(const QString &accessToken, const QString &accessTokenSecret, const QString &accessOauthSignature);
    void slotAuthorizationFailed(const QString &errorMessage);

private:
    void readConfig();
    QString mAccessToken;
    QString mAccessTokenSecret;
    QString mAccessOauthSignature;    
};
}

#endif // DROPBOXSTORAGESERVICE_H
