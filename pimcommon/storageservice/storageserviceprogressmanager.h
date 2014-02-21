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

#ifndef STORAGESERVICEPROGRESSMANAGER_H
#define STORAGESERVICEPROGRESSMANAGER_H

#include <QObject>

#include "pimcommon_export.h"

#include <QHash>
#include <QPointer>

namespace KPIM {
class ProgressItem;
}

namespace PimCommon {
class StorageServiceAbstract;
class ProgressJob;
class PIMCOMMON_EXPORT StorageServiceProgressManager : public QObject
{
    Q_OBJECT
public:
    enum ProgressType {
        DownLoad = 0,
        Upload = 1
    };

    ~StorageServiceProgressManager();

    static unsigned int progressTypeValue();

    static StorageServiceProgressManager *self();

    void addProgress(PimCommon::StorageServiceAbstract *storageService, ProgressType type);

private slots:
    void slotProgressItemCanceled(KPIM::ProgressItem *item);

    void slotUploadFileDone(const QString &serviceName, const QString &);
    void slotUploadFileFailed(const QString &serviceName, const QString &);
    void slotDownloadFileDone(const QString &serviceName, const QString &);
    void slotDownloadFileFailed(const QString &serviceName, const QString &);
    void slotDownloadFileProgress(const QString &serviceName, qint64 done, qint64 total);
    void slotActionFailed(const QString &serviceName, const QString &);
private:
    explicit StorageServiceProgressManager(QObject *parent = 0);
    friend class StorageServiceProgressManagerPrivate;
    QHash<QString, ProgressJob *> mHashList;
};

class ProgressJob
{
public:
    ProgressJob(KPIM::ProgressItem *item, StorageServiceProgressManager::ProgressType type);
    ~ProgressJob();

    void setStorageService(StorageServiceAbstract *storage);
    StorageServiceAbstract *storageService() const;

    StorageServiceProgressManager::ProgressType type() const;
    KPIM::ProgressItem *item() const;

private:
    StorageServiceProgressManager::ProgressType mType;
    StorageServiceAbstract *mStorageService;
    QPointer<KPIM::ProgressItem> mProgressItem;
};

}

#endif // STORAGESERVICEPROGRESSMANAGER_H
