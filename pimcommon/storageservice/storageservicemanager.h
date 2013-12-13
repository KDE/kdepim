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

#ifndef STORAGESERVICEMANAGER_H
#define STORAGESERVICEMANAGER_H

#include <QObject>
#include <QMap>
#include "pimcommon_export.h"
#include "storageserviceabstract.h"

class QMenu;

namespace PimCommon {
class PIMCOMMON_EXPORT StorageServiceManager : public QObject
{
    Q_OBJECT
public:
    enum ServiceType {
        Unknown = -1,
        DropBox = 0,
        Hubic,
        UbuntuOne,
        YouSendIt,
        WebDav,
        Box,

        //Last element
        EndListService
    };

    explicit StorageServiceManager(QObject *parent=0);
    ~StorageServiceManager();
    QMenu *menuUploadServices();
    QMap<QString, StorageServiceAbstract *> listService() const;
    void setListService(const QMap<QString, StorageServiceAbstract *> &lst);

    static QString serviceToI18n(ServiceType type);
    static QString serviceName(ServiceType type);
    static QString description(ServiceType type);
    static QUrl serviceUrl(ServiceType type);    
    static QString icon(ServiceType type);

Q_SIGNALS:
    void servicesChanged();
    void uploadFileDone(const QString &serviceName, const QString &filename);
    void uploadFileProgress(const QString &serviceName, qint64 done, qint64 total);
    void shareLinkDone(const QString &serviceName, const QString &link);

private Q_SLOTS:
    void slotShareFile();

private:
    void readConfig();
    void writeConfig();
    QMap<QString, StorageServiceAbstract *> mListService;
    QMenu *mMenuService;
};
}

#endif // STORAGESERVICEMANAGER_H
