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

#ifndef STORAGESERVICEMANAGER_H
#define STORAGESERVICEMANAGER_H

#include <QObject>
#include <QMap>
#include "pimcommon_export.h"
#include "storageserviceabstract.h"

class KActionMenu;
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
#ifdef KDEPIM_STORAGESERVICE_GDRIVE
        GDrive,
#endif

        //Last element
        EndListService
    };

    explicit StorageServiceManager(QObject *parent=0);
    ~StorageServiceManager();

    KActionMenu *menuDownloadServices(QWidget *parent) const;
    KActionMenu *menuUploadServices(QWidget *parent) const;
    KActionMenu *menuWithCapability(PimCommon::StorageServiceAbstract::Capability capability, QWidget *parent) const;


    QMap<QString, StorageServiceAbstract *> listService() const;
    void setListService(const QMap<QString, StorageServiceAbstract *> &lst);
    void setDefaultUploadFolder(const QString &folder);
    QString defaultUploadFolder() const;

    static QString serviceToI18n(ServiceType type);
    static QString serviceName(ServiceType type);
    static QString description(ServiceType type);
    static QUrl serviceUrl(ServiceType type);
    static QString icon(ServiceType type);
    static StorageServiceAbstract::Capabilities capabilities(ServiceType type);
    static QString kconfigName();

Q_SIGNALS:
    void servicesChanged();
    void uploadFileDone(const QString &serviceName, const QString &filename);
    void uploadDownloadFileProgress(const QString &serviceName, qint64 done, qint64 total);
    void uploadFileFailed(const QString &serviceName, const QString &filename);
    void shareLinkDone(const QString &serviceName, const QString &link);
    void authenticationDone(const QString &serviceName);
    void authenticationFailed(const QString &serviceName, const QString &error);
    void actionFailed(const QString &serviceName, const QString &error);
    void deleteFileDone(const QString &serviceName, const QString &filename);
    void accountInfoDone(const QString &serviceName, const PimCommon::AccountInfo &accountInfo);

private Q_SLOTS:
    void slotAccountInfo();
    void slotShareFile();
    void slotDeleteFile();
    void slotDownloadFile();
private:
    void defaultConnect(StorageServiceAbstract *service);
    void readConfig();
    void writeConfig();
    QMap<QString, StorageServiceAbstract *> mListService;
    QString mDefaultUploadFolder;
};
}

#endif // STORAGESERVICEMANAGER_H
