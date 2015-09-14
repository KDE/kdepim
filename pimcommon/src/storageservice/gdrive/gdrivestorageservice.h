/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#ifndef GDriveStorageService_H
#define GDriveStorageService_H

#include "storageservice/storageserviceabstract.h"
#include "pimcommon_export.h"

#include <QDateTime>
#include <kgapi/account.h>

namespace PimCommon
{
class PIMCOMMON_EXPORT GDriveStorageService : public PimCommon::StorageServiceAbstract
{
    Q_OBJECT
public:
    explicit GDriveStorageService(QObject *parent = Q_NULLPTR);
    ~GDriveStorageService();

    static QString name();
    static QString description();
    static QUrl serviceUrl();
    static QString serviceName();
    static QString iconName();
    static StorageServiceAbstract::Capabilities serviceCapabilities();

    void storageServiceuploadFile(const QString &filename, const QString &uploadAsName, const QString &destination = QString()) Q_DECL_OVERRIDE;
    void storageServiceaccountInfo() Q_DECL_OVERRIDE;
    void storageServicecreateFolder(const QString &folder, const QString &destination = QString()) Q_DECL_OVERRIDE;
    void storageServicelistFolder(const QString &folder) Q_DECL_OVERRIDE;
    void removeConfig() Q_DECL_OVERRIDE;
    void storageServiceauthentication() Q_DECL_OVERRIDE;
    void storageServiceShareLink(const QString &root, const QString &path) Q_DECL_OVERRIDE;
    void storageServicedownloadFile(const QString &name, const QString &fileId, const QString &destination) Q_DECL_OVERRIDE;
    void storageServicedeleteFile(const QString &filename) Q_DECL_OVERRIDE;
    void storageServicedeleteFolder(const QString &foldername) Q_DECL_OVERRIDE;
    void storageServiceRenameFolder(const QString &source, const QString &destination) Q_DECL_OVERRIDE;
    void storageServiceRenameFile(const QString &source, const QString &destination) Q_DECL_OVERRIDE;
    void storageServiceMoveFolder(const QString &source, const QString &destination) Q_DECL_OVERRIDE;
    void storageServiceMoveFile(const QString &source, const QString &destination) Q_DECL_OVERRIDE;
    void storageServiceCopyFile(const QString &source, const QString &destination) Q_DECL_OVERRIDE;
    void storageServiceCopyFolder(const QString &source, const QString &destination) Q_DECL_OVERRIDE;
    QMap<QString, QString> itemInformation(const QVariantMap &variantMap) Q_DECL_OVERRIDE;
    QString storageServiceName() const Q_DECL_OVERRIDE;
    QString fileIdentifier(const QVariantMap &variantMap) Q_DECL_OVERRIDE;
    QString fileShareRoot(const QVariantMap &variantMap) Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    StorageServiceAbstract::Capabilities capabilities() const Q_DECL_OVERRIDE;
    void storageServicecreateServiceFolder() Q_DECL_OVERRIDE;
    QString fillListWidget(StorageServiceTreeWidget *listWidget, const QVariant &data, const QString &currentFolder) Q_DECL_OVERRIDE;

    void shutdownService() Q_DECL_OVERRIDE;
    bool hasValidSettings() const Q_DECL_OVERRIDE;
    bool hasCancelSupport() const Q_DECL_OVERRIDE;
private Q_SLOTS:
    void slotAuthorizationDone(const QString &refreshToken, const QString &token, const QDateTime &expireTime, const QString &accountName);
    void slotAuthorizationFailed(const QString &errorMessage);

private:
    bool needToRefreshToken() const;
    void refreshToken();
    void readConfig();
    KGAPI2::AccountPtr mAccount;
    QDateTime mExpireDateTime;
};
}

#endif // GDriveStorageService_H
