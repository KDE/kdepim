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

#include "storageservicemanager.h"

#include "settings/pimcommonsettings.h"

#include "dropbox/dropboxstorageservice.h"
#include "hubic/hubicstorageservice.h"
#include "ubuntuone/ubuntuonestorageservice.h"
#include "yousendit/yousenditstorageservice.h"
#include "webdav/webdavstorageservice.h"
#include "box/boxstorageservice.h"

#include <KLocalizedString>
#include <KFileDialog>

#include <QMenu>

using namespace PimCommon;

StorageServiceManager::StorageServiceManager(QObject *parent)
    : QObject(parent),
      mMenuService(0)
{
    readConfig();
}

StorageServiceManager::~StorageServiceManager()
{
    qDeleteAll(mListService);
}

QMap<QString, StorageServiceAbstract *> StorageServiceManager::listService() const
{
    return mListService;
}

void StorageServiceManager::setListService(const QMap<QString, StorageServiceAbstract *> &lst)
{
    mListService = lst;
    delete mMenuService;
    mMenuService = 0;
    writeConfig();
    Q_EMIT servicesChanged();
}

QMenu *StorageServiceManager::menuUploadServices()
{
    if (!mMenuService) {
        mMenuService = new QMenu(i18n("Storage service"));
        if (mListService.isEmpty()) {
            QAction *act = new QAction(i18n("No Storage service configured"), mMenuService);
            act->setEnabled(false);
            mMenuService->addAction(act);
        } else {
            QMapIterator<QString, StorageServiceAbstract *> i(mListService);
            while (i.hasNext()) {
                i.next();
                //FIXME
                if (i.value()->capabilities() & PimCommon::StorageServiceAbstract::ShareLinkCapability) {
                    QAction *act = new QAction(/*serviceToI18n(*/i.key(), mMenuService);
                    act->setData(i.key());
                    connect(act, SIGNAL(triggered()), this, SLOT(slotShareFile()));
                    mMenuService->addAction(act);
                }
            }
        }
    }
    return mMenuService;
}


void StorageServiceManager::slotShareFile()
{
    QAction *act = qobject_cast< QAction* >( sender() );
    if ( act ) {
        const QString type = act->data().toString();
        if (mListService.contains(type)) {
            StorageServiceAbstract *service = mListService.value(type);
            const QString fileName = KFileDialog::getOpenFileName( QString(), QString(), 0, i18n("File to upload") );
            if (!fileName.isEmpty()) {
                service->uploadFile(fileName);
                connect(service,SIGNAL(actionFailed(QString,QString)), this, SIGNAL(actionFailed(QString,QString)), Qt::UniqueConnection);
                connect(service,SIGNAL(authenticationDone(QString)), this, SIGNAL(authenticationDone(QString)), Qt::UniqueConnection);
                connect(service,SIGNAL(authenticationFailed(QString,QString)), this, SIGNAL(authenticationFailed(QString,QString)), Qt::UniqueConnection);
                connect(service,SIGNAL(uploadFileProgress(QString,qint64,qint64)), this, SIGNAL(uploadFileProgress(QString,qint64,qint64)), Qt::UniqueConnection);
                connect(service,SIGNAL(uploadFileDone(QString,QString)), this, SIGNAL(uploadFileDone(QString,QString)), Qt::UniqueConnection);
                connect(service,SIGNAL(shareLinkDone(QString,QString)), this, SIGNAL(shareLinkDone(QString,QString)), Qt::UniqueConnection);
            }
        }
    }
}

void StorageServiceManager::readConfig()
{
    const QStringList services = PimCommon::PimCommonSettings::self()->services();
    Q_FOREACH(const QString &service, services) {
        PimCommon::StorageServiceAbstract *storageService = 0;
        if (service == serviceName(DropBox)) {
            if (!mListService.contains(serviceName(DropBox))) {
                storageService = new DropBoxStorageService();
            }
        } else if (service == serviceName(Hubic)) {
            if (!mListService.contains(serviceName(Hubic))) {
                storageService = new HubicStorageService();
            }
        } else if (service == serviceName(UbuntuOne)) {
            if (!mListService.contains(serviceName(UbuntuOne))) {
                storageService = new UbuntuoneStorageService();
            }
        } else if (service == serviceName(YouSendIt)) {
            if (!mListService.contains(serviceName(YouSendIt))) {
                storageService = new YouSendItStorageService();
            }
        } else if (service == serviceName(WebDav)) {
            if (!mListService.contains(serviceName(WebDav))) {
                storageService = new WebDavStorageService();
            }
        } else if (service == serviceName(Box)) {
            if (!mListService.contains(serviceName(Box))) {
                storageService = new BoxStorageService();
            }
        }
        if (storageService) {
            mListService.insert(service, storageService);
        }
    }
}

void StorageServiceManager::writeConfig()
{
    PimCommon::PimCommonSettings::self()->setServices(mListService.keys());
    PimCommon::PimCommonSettings::self()->writeConfig();
}

QString StorageServiceManager::description(ServiceType type)
{
    switch(type) {
    case DropBox:
        return PimCommon::DropBoxStorageService::description();
    case Hubic:
        return PimCommon::HubicStorageService::description();
    case UbuntuOne:
        return PimCommon::UbuntuoneStorageService::description();
    case WebDav:
        return PimCommon::WebDavStorageService::description();
    case Box:
        return PimCommon::BoxStorageService::description();
    default:
        return QString();
    }
    return QString();
}

QUrl StorageServiceManager::serviceUrl(ServiceType type)
{
    switch(type) {
    case DropBox:
        return PimCommon::DropBoxStorageService::serviceUrl();
    case Hubic:
        return PimCommon::HubicStorageService::serviceUrl();
    case UbuntuOne:
        return PimCommon::UbuntuoneStorageService::serviceUrl();
    case YouSendIt:
        return PimCommon::YouSendItStorageService::serviceUrl();
    case WebDav:
        return PimCommon::WebDavStorageService::serviceUrl();
    case Box:
        return PimCommon::BoxStorageService::serviceUrl();
    default:
        return QString();
    }
    return QString();
}


QString StorageServiceManager::serviceName(ServiceType type)
{
    switch(type) {
    case DropBox:
        return PimCommon::DropBoxStorageService::serviceName();
    case Hubic:
        return PimCommon::HubicStorageService::serviceName();
    case UbuntuOne:
        return PimCommon::UbuntuoneStorageService::serviceName();
    case YouSendIt:
        return PimCommon::YouSendItStorageService::serviceName();
    case WebDav:
        return PimCommon::WebDavStorageService::serviceName();
    case Box:
        return PimCommon::BoxStorageService::serviceName();
    default:
        return QString();
    }
}

QString StorageServiceManager::serviceToI18n(ServiceType type)
{
    switch(type) {
    case DropBox:
        return PimCommon::DropBoxStorageService::name();
    case Hubic:
        return PimCommon::HubicStorageService::name();
    case UbuntuOne:
        return PimCommon::UbuntuoneStorageService::name();
    case YouSendIt:
        return PimCommon::YouSendItStorageService::name();
    case WebDav:
        return PimCommon::WebDavStorageService::name();
    case Box:
        return PimCommon::BoxStorageService::name();
    default:
        return QString();
    }
}

QString StorageServiceManager::icon(ServiceType type)
{
    switch(type) {
    case DropBox:
        return PimCommon::DropBoxStorageService::iconName();
    case Hubic:
        return PimCommon::HubicStorageService::iconName();
    case UbuntuOne:
        return PimCommon::UbuntuoneStorageService::iconName();
    case YouSendIt:
        return PimCommon::YouSendItStorageService::iconName();
    case WebDav:
        return PimCommon::WebDavStorageService::iconName();
    case Box:
        return PimCommon::BoxStorageService::iconName();
    default:
        return QString();
    }
}
