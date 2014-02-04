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

#include "storageservice/dialog/storageservicedownloaddialog.h"
#include "storageservice/dialog/storageservicechecknamedialog.h"

#include "dropbox/dropboxstorageservice.h"
#include "hubic/hubicstorageservice.h"
#include "ubuntuone/ubuntuonestorageservice.h"
#include "yousendit/yousenditstorageservice.h"
#include "webdav/webdavstorageservice.h"
#include "box/boxstorageservice.h"
#ifdef KDEPIM_STORAGESERVICE_GDRIVE
#include "gdrive/gdrivestorageservice.h"
#endif
#include <KLocalizedString>
#include <KFileDialog>
#include <KInputDialog>
#include <KActionMenu>
#include <KMessageBox>

#include <QMenu>


using namespace PimCommon;

StorageServiceManager::StorageServiceManager(QObject *parent)
    : QObject(parent)
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
    writeConfig();
    Q_EMIT servicesChanged();
}

void StorageServiceManager::setDefaultUploadFolder(const QString &folder)
{
    mDefaultUploadFolder = folder;
}

QString StorageServiceManager::defaultUploadFolder() const
{
    return mDefaultUploadFolder;
}

KActionMenu *StorageServiceManager::menuUploadServices(QWidget *parent) const
{
    return menuWithCapability(PimCommon::StorageServiceAbstract::ShareLinkCapability, parent);
}

KActionMenu *StorageServiceManager::menuDownloadServices(QWidget *parent) const
{
    return menuWithCapability(PimCommon::StorageServiceAbstract::DownloadFileCapability, parent);
}
//TODO add capabilities
KActionMenu *StorageServiceManager::menuWithCapability(PimCommon::StorageServiceAbstract::Capability capability, QWidget *parent) const
{
    KActionMenu *menuService = new KActionMenu(i18n("Storage service"), parent);
    if (mListService.isEmpty()) {
        QAction *act = new QAction(i18n("No Storage service configured"), menuService);
        act->setEnabled(false);
        menuService->addAction(act);
    } else {
        QMapIterator<QString, StorageServiceAbstract *> i(mListService);
        while (i.hasNext()) {
            i.next();
            //FIXME
            if (i.value()->capabilities() & capability) {
                QAction *act = new QAction(/*serviceToI18n(*/i.key(), menuService);
                act->setData(i.key());
                const KIcon icon = i.value()->icon();
                if (!icon.isNull())
                    act->setIcon(icon);
                switch(capability) {
                case PimCommon::StorageServiceAbstract::NoCapability:
                case PimCommon::StorageServiceAbstract::CreateFolderCapability:
                case PimCommon::StorageServiceAbstract::DeleteFolderCapability:
                case PimCommon::StorageServiceAbstract::ListFolderCapability:
                case PimCommon::StorageServiceAbstract::RenameFolderCapability:
                case PimCommon::StorageServiceAbstract::RenameFileCapabilitity:
                case PimCommon::StorageServiceAbstract::MoveFolderCapability:
                case PimCommon::StorageServiceAbstract::MoveFileCapability:
                case PimCommon::StorageServiceAbstract::CopyFileCapability:
                case PimCommon::StorageServiceAbstract::CopyFolderCapability:
                    qDebug()<<" not implemented ";
                    break;
                case PimCommon::StorageServiceAbstract::DownloadFileCapability:
                    menuService->setText(i18n("Download File..."));
                    connect(act, SIGNAL(triggered()), this, SLOT(slotDownloadFile()));
                    break;
                case PimCommon::StorageServiceAbstract::UploadFileCapability:
                    menuService->setText(i18n("Upload File..."));
                    connect(act, SIGNAL(triggered()), this, SLOT(slotShareFile()));
                    break;
                case PimCommon::StorageServiceAbstract::ShareLinkCapability:
                    menuService->setText(i18n("Share File..."));
                    connect(act, SIGNAL(triggered()), this, SLOT(slotShareFile()));
                    break;
                case PimCommon::StorageServiceAbstract::DeleteFileCapability:
                    menuService->setText(i18n("Delete File..."));
                    connect(act, SIGNAL(triggered()), this, SLOT(slotDeleteFile()));
                    break;
                case PimCommon::StorageServiceAbstract::AccountInfoCapability:
                    menuService->setText(i18n("Account Info..."));
                    connect(act, SIGNAL(triggered()), this, SLOT(slotAccountInfo()));
                    break;
                }
                menuService->addAction(act);
            }
        }
    }
    return menuService;
}


void StorageServiceManager::slotShareFile()
{
    QAction *act = qobject_cast< QAction* >( sender() );
    if ( act ) {
        const QString type = act->data().toString();
        if (mListService.contains(type)) {
            StorageServiceAbstract *service = mListService.value(type);
            if (service->hasUploadOrDownloadInProgress()) {
                KMessageBox::information(0, i18n("There is still an upload in progress."));
            } else {
                const QString fileName = KFileDialog::getOpenFileName( QString(), QString(), 0, i18n("File to upload") );
                if (!fileName.isEmpty()) {
                    QFileInfo info(fileName);
                    const QRegExp disallowedSymbols = service->disallowedSymbols();
                    QString newName = info.fileName();
                    if (!disallowedSymbols.isEmpty()) {
                        if (newName.contains(disallowedSymbols)) {
                            QPointer<PimCommon::StorageServiceCheckNameDialog> dlg = new PimCommon::StorageServiceCheckNameDialog;
                            dlg->setOldName(newName);
                            dlg->setDisallowedSymbols(disallowedSymbols);
                            dlg->setDisallowedSymbolsStr(service->disallowedSymbolsStr());
                            if (dlg->exec()) {
                                newName = dlg->newName();
                                delete dlg;
                            } else {
                                delete dlg;
                                return;
                            }
                        }
                    }

                    defaultConnect(service);
                    connect(service,SIGNAL(uploadDownloadFileProgress(QString,qint64,qint64)), this, SIGNAL(uploadDownloadFileProgress(QString,qint64,qint64)), Qt::UniqueConnection);
                    connect(service,SIGNAL(uploadFileDone(QString,QString)), this, SIGNAL(uploadFileDone(QString,QString)), Qt::UniqueConnection);
                    connect(service,SIGNAL(uploadFileFailed(QString,QString)), this, SIGNAL(uploadFileFailed(QString,QString)), Qt::UniqueConnection);
                    connect(service,SIGNAL(shareLinkDone(QString,QString)), this, SIGNAL(shareLinkDone(QString,QString)), Qt::UniqueConnection);
                    Q_EMIT uploadFileStart(service);
                    service->uploadFile(fileName, newName, mDefaultUploadFolder);
                }
            }
        }
    }
}

void StorageServiceManager::slotDownloadFile()
{
    QAction *act = qobject_cast< QAction* >( sender() );
    if ( act ) {
        const QString type = act->data().toString();
        if (mListService.contains(type)) {
            StorageServiceAbstract *service = mListService.value(type);
            QPointer<PimCommon::StorageServiceDownloadDialog> dlg = new PimCommon::StorageServiceDownloadDialog(service, 0);
            if (dlg->exec()) {
                //TODO ?
            }
            delete dlg;
        }
    }
}

void StorageServiceManager::defaultConnect(StorageServiceAbstract *service)
{
    connect(service,SIGNAL(actionFailed(QString,QString)), this, SIGNAL(actionFailed(QString,QString)), Qt::UniqueConnection);
    connect(service,SIGNAL(authenticationDone(QString)), this, SIGNAL(authenticationDone(QString)), Qt::UniqueConnection);
    connect(service,SIGNAL(authenticationFailed(QString,QString)), this, SIGNAL(authenticationFailed(QString,QString)), Qt::UniqueConnection);
}

void StorageServiceManager::slotDeleteFile()
{
    QAction *act = qobject_cast< QAction* >( sender() );
    if ( act ) {
        const QString type = act->data().toString();
        if (mListService.contains(type)) {
            StorageServiceAbstract *service = mListService.value(type);
            const QString fileName = KInputDialog::getText(i18n("Delete File"), i18n("Filename:"));
            if (!fileName.isEmpty()) {
                defaultConnect(service);
                connect(service,SIGNAL(deleteFileDone(QString,QString)), this, SIGNAL(deleteFileDone(QString,QString)), Qt::UniqueConnection);
                service->deleteFile(fileName);
            }
        }
    }
}

void StorageServiceManager::slotAccountInfo()
{
    QAction *act = qobject_cast< QAction* >( sender() );
    if ( act ) {
        const QString type = act->data().toString();
        if (mListService.contains(type)) {
            StorageServiceAbstract *service = mListService.value(type);
            defaultConnect(service);
            connect(service,SIGNAL(accountInfoDone(QString,PimCommon::AccountInfo)), this, SIGNAL(accountInfoDone(QString,PimCommon::AccountInfo)), Qt::UniqueConnection);
            service->accountInfo();
        }
    }
}

void StorageServiceManager::readConfig()
{
    KConfig conf(kconfigName());
    KConfigGroup grp(&conf, QLatin1String("General"));

    const QStringList services = grp.readEntry("Services", QStringList());
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
#ifdef KDEPIM_STORAGESERVICE_GDRIVE
        } else if (service == serviceName(GDrive)) {
            if (!mListService.contains(serviceName(GDrive))) {
                storageService = new GDriveStorageService();
            }
#endif
        }
        if (storageService) {
            mListService.insert(service, storageService);
        }
    }
}

void StorageServiceManager::writeConfig()
{
    KConfig conf(kconfigName());
    KConfigGroup grp(&conf, QLatin1String("General"));
    grp.writeEntry("Services", mListService.keys());
    conf.sync();
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
    case YouSendIt:
        return PimCommon::YouSendItStorageService::description();
#ifdef KDEPIM_STORAGESERVICE_GDRIVE
    case GDrive:
        return PimCommon::GDriveStorageService::description();
#endif
    case EndListService:
    case Unknown:
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
#ifdef KDEPIM_STORAGESERVICE_GDRIVE
    case GDrive:
        return PimCommon::GDriveStorageService::serviceUrl();
#endif
    case EndListService:
    case Unknown:
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
#ifdef KDEPIM_STORAGESERVICE_GDRIVE
    case GDrive:
        return PimCommon::GDriveStorageService::serviceName();
#endif
    case EndListService:
    case Unknown:
        return QString();
    }
    return QString();
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
#ifdef KDEPIM_STORAGESERVICE_GDRIVE
    case GDrive:
        return PimCommon::GDriveStorageService::name();
#endif
    case EndListService:
    case Unknown:
        return QString();
    }
    return QString();
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
#ifdef KDEPIM_STORAGESERVICE_GDRIVE
    case GDrive:
        return PimCommon::GDriveStorageService::iconName();
#endif
    case EndListService:
    case Unknown:
        return QString();
    }
    return QString();
}

StorageServiceAbstract::Capabilities StorageServiceManager::capabilities(ServiceType type)
{
    switch(type) {
    case DropBox:
        return PimCommon::DropBoxStorageService::serviceCapabilities();
    case Hubic:
        return PimCommon::HubicStorageService::serviceCapabilities();
    case UbuntuOne:
        return PimCommon::UbuntuoneStorageService::serviceCapabilities();
    case YouSendIt:
        return PimCommon::YouSendItStorageService::serviceCapabilities();
    case WebDav:
        return PimCommon::WebDavStorageService::serviceCapabilities();
    case Box:
        return PimCommon::BoxStorageService::serviceCapabilities();
#ifdef KDEPIM_STORAGESERVICE_GDRIVE
    case GDrive:
        return PimCommon::GDriveStorageService::serviceCapabilities();
#endif
    case EndListService:
    case Unknown:
        return StorageServiceAbstract::NoCapability;
    }
    return StorageServiceAbstract::NoCapability;
}

QString StorageServiceManager::kconfigName()
{
    return QLatin1String("storageservicerc");
}

#include "moc_storageservicemanager.cpp"
