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

#ifndef UBUNTUONESTORAGESERVICE_H
#define UBUNTUONESTORAGESERVICE_H

#include "pimcommon/storageservice/storageserviceabstract.h"

namespace PimCommon {
class UbuntuoneStorageService : public PimCommon::StorageServiceAbstract
{
    Q_OBJECT
public:
    explicit UbuntuoneStorageService(QObject *parent=0);
    ~UbuntuoneStorageService();

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
    void shareLink(const QString &root, const QString &path);

private slots:
    void slotAuthorizationDone(const QString &customerSecret, const QString &token, const QString &customerKey, const QString &tokenSecret);
    void slotAccountInfoDone(const PimCommon::AccountInfo &info);
private:
    void readConfig();
    QString mCustomerSecret;
    QString mToken;
    QString mCustomerKey;
    QString mTokenSecret;
};
}

#endif // UBUNTUONESTORAGESERVICE_H
