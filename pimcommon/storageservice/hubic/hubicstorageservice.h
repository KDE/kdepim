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

#ifndef HUBICSTORAGESERVICE_H
#define HUBICSTORAGESERVICE_H

#include "pimcommon/storageservice/storageserviceabstract.h"
#include "pimcommon_export.h"

#include <QDateTime>

namespace PimCommon {
class PIMCOMMON_EXPORT HubicStorageService : public PimCommon::StorageServiceAbstract
{
    Q_OBJECT
public:
    explicit HubicStorageService(QObject *parent=0);
    ~HubicStorageService();

    static QString name();
    static QString description();
    static QUrl serviceUrl();
    static QString serviceName();
    static QString iconName();

    void uploadFile(const QString &filename);
    void accountInfo();
    void createFolder(const QString &folder);
    void listFolder();
    void removeConfig();
    void authentication();
    void shareLink(const QString &root, const QString &path);
    QString storageServiceName() const;
    void downloadFile();
    void createServiceFolder();


    KIcon icon() const;

private slots:
    void slotAuthorizationDone(const QString &refreshToken, const QString &token, qint64 expireTime);
    void slotAuthorizationFailed(const QString &errorMessage);

private:
    void readConfig();

    QString mRefreshToken;
    QString mToken;
    QDateTime mExpireDateTime;
};
}

#endif // HUBICSTORAGESERVICE_H
