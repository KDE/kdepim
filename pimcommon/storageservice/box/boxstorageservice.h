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

#ifndef BoxStorageService_H
#define BoxStorageService_H

#include "pimcommon/storageservice/storageserviceabstract.h"
#include "pimcommon_export.h"

#include <QDateTime>

namespace PimCommon {
class PIMCOMMON_EXPORT BoxStorageService : public PimCommon::StorageServiceAbstract
{
    Q_OBJECT
public:
    explicit BoxStorageService(QObject *parent=0);
    ~BoxStorageService();

    static QString name();
    static QString description();
    static QUrl serviceUrl();
    static QString serviceName();
    static QString iconName();

    void storageServiceuploadFile(const QString &filename);
    void storageServiceaccountInfo();
    void storageServicecreateFolder(const QString &folder);
    void storageServicelistFolder();
    void removeConfig();
    void storageServiceauthentication();
    void storageServiceShareLink(const QString &root, const QString &path);
    void storageServicedownloadFile(const QString &filename);
    void storageServicedeleteFile(const QString &filename);
    void storageServicedeleteFolder(const QString &foldername);

    QString storageServiceName() const;
    KIcon icon() const;
    void storageServicecreateServiceFolder();

private slots:
    void slotAuthorizationDone(const QString &refreshToken, const QString &token, qint64 expireTime);
    void slotAuthorizationFailed(const QString &errorMessage);

private:
    void readConfig();
    QString mToken;
    QString mRefreshToken;
    QDateTime mExpireDateTime;
};
}

#endif // BoxStorageService_H
