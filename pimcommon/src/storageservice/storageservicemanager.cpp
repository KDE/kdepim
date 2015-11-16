/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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
#include "storageserviceprogressmanager.h"
#include "storagemanageradaptor.h"
#include "pimcommon_debug.h"

#include "settings/pimcommonsettings.h"
#include "storageservice/utils/storageserviceutils.h"

#include "storageservice/dialog/storageservicedownloaddialog.h"
#include "storageservice/dialog/storageservicedeletedialog.h"
#include "storageservice/dialog/storageservicechecknamedialog.h"

#include "dropbox/dropboxstorageservice.h"
#include "hubic/hubicstorageservice.h"
#include "yousendit/yousenditstorageservice.h"
#include "webdav/webdavstorageservice.h"
#include "box/boxstorageservice.h"
#ifdef KDEPIM_STORAGESERVICE_GDRIVE
#include "gdrive/gdrivestorageservice.h"
#endif
#include <KLocalizedString>
#include <KActionMenu>
#include <KMessageBox>

#include <QDBusConnection>
#include <QFileDialog>

using namespace PimCommon;

static QString newDBusObjectName()
{
    static int s_count = 0;
    QString name(QStringLiteral("/STORAGESERVICE_ServiceManager"));
    if (s_count++) {
        name += QLatin1Char('_');
        name += QString::number(s_count);
    }
    return name;
}

class PimCommon::StorageServiceManagerPrivate
{
public:
    StorageServiceManagerPrivate()
    {

    }
    ~StorageServiceManagerPrivate()
    {
        qDeleteAll(mListService);
    }

    QMap<QString, StorageServiceAbstract *> mListService;
    QString mDefaultUploadFolder;
};

StorageServiceManager::StorageServiceManager(QObject *parent)
    : QObject(parent),
      d(new PimCommon::StorageServiceManagerPrivate)
{
    new StorageManagerAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    const QString dbusPath = newDBusObjectName();
    setProperty("uniqueDBusPath", dbusPath);
    const QString dbusInterface = QStringLiteral("org.kde.pim.StorageManager");
    dbus.registerObject(dbusPath, this);
    dbus.connect(QString(), QString(), dbusInterface, QStringLiteral("configChanged"), this,
                 SLOT(slotConfigChanged(QString)));

    readConfig();
}

StorageServiceManager::~StorageServiceManager()
{
    delete d;
}

void StorageServiceManager::removeService(const QString &serviceName)
{
    if (d->mListService.contains(serviceName)) {
        d->mListService.remove(serviceName);
    }
}

QString StorageServiceManager::ourIdentifier() const
{
    const QString identifier = QStringLiteral("%1/%2").
                               arg(QDBusConnection::sessionBus().baseService(), property("uniqueDBusPath").toString());
    return identifier;
}

void StorageServiceManager::slotConfigChanged(const QString &id)
{
    qCDebug(PIMCOMMON_LOG) << " void StorageServiceManager::slotConfigChanged(const QString &id)" << id;
    if (id != ourIdentifier()) {
        readConfig();
        Q_EMIT servicesChanged();
    }
}

QMap<QString, StorageServiceAbstract *> StorageServiceManager::listService() const
{
    return d->mListService;
}

void StorageServiceManager::setListService(const QMap<QString, StorageServiceAbstract *> &lst)
{
    d->mListService = lst;
    writeConfig();

    // DBus signal for other IdentityManager instances
    Q_EMIT configChanged(ourIdentifier());
}

void StorageServiceManager::setDefaultUploadFolder(const QString &folder)
{
    d->mDefaultUploadFolder = folder;
}

QString StorageServiceManager::defaultUploadFolder() const
{
    return d->mDefaultUploadFolder;
}

KActionMenu *StorageServiceManager::menuShareLinkServices(QWidget *parent) const
{
    QList<PimCommon::StorageServiceAbstract::Capability> lstCapability;
    lstCapability << PimCommon::StorageServiceAbstract::ShareLinkCapability;
    lstCapability << PimCommon::StorageServiceAbstract::UploadFileCapability;
    return menuWithCapability(PimCommon::StorageServiceAbstract::ShareLinkCapability, lstCapability, parent);
}

KActionMenu *StorageServiceManager::menuUploadServices(QWidget *parent) const
{
    QList<PimCommon::StorageServiceAbstract::Capability> lstCapability;
    lstCapability << PimCommon::StorageServiceAbstract::UploadFileCapability;
    return menuWithCapability(PimCommon::StorageServiceAbstract::UploadFileCapability, lstCapability, parent);
}

KActionMenu *StorageServiceManager::menuDownloadServices(QWidget *parent) const
{
    QList<PimCommon::StorageServiceAbstract::Capability> lstCapability;
    lstCapability << PimCommon::StorageServiceAbstract::DownloadFileCapability;
    return menuWithCapability(PimCommon::StorageServiceAbstract::DownloadFileCapability, lstCapability, parent);
}

KActionMenu *StorageServiceManager::menuWithCapability(PimCommon::StorageServiceAbstract::Capability mainCapability, const QList<PimCommon::StorageServiceAbstract::Capability> &lstCapability, QWidget *parent) const
{
    KActionMenu *menuService = new KActionMenu(i18n("Storage service"), parent);
    if (d->mListService.isEmpty()) {
        QAction *act = new QAction(i18n("No Storage service configured"), menuService);
        act->setEnabled(false);
        menuService->addAction(act);
    } else {
        QMapIterator<QString, StorageServiceAbstract *> i(d->mListService);
        while (i.hasNext()) {
            i.next();
            //FIXME
            if (PimCommon::StorageServiceUtils::hasExactCapabilities(i.value()->capabilities(), lstCapability)) {
                QAction *act = new QAction(/*serviceToI18n(*/i.key(), menuService);
                act->setData(i.key());
                const QIcon icon = i.value()->icon();
                if (!icon.isNull()) {
                    act->setIcon(icon);
                }
                switch (mainCapability) {
                case PimCommon::StorageServiceAbstract::NoCapability:
                case PimCommon::StorageServiceAbstract::CreateFolderCapability:
                case PimCommon::StorageServiceAbstract::ListFolderCapability:
                case PimCommon::StorageServiceAbstract::RenameFolderCapability:
                case PimCommon::StorageServiceAbstract::RenameFileCapabilitity:
                case PimCommon::StorageServiceAbstract::MoveFolderCapability:
                case PimCommon::StorageServiceAbstract::MoveFileCapability:
                case PimCommon::StorageServiceAbstract::CopyFileCapability:
                case PimCommon::StorageServiceAbstract::CopyFolderCapability:
                    qCDebug(PIMCOMMON_LOG) << " not implemented ";
                    break;
                case PimCommon::StorageServiceAbstract::DeleteFolderCapability:
                    menuService->setText(i18n("Delete Folder..."));
                    connect(act, &QAction::triggered, this, &StorageServiceManager::slotDeleteFolder);
                    break;
                case PimCommon::StorageServiceAbstract::DownloadFileCapability:
                    menuService->setText(i18n("Download File..."));
                    connect(act, &QAction::triggered, this, &StorageServiceManager::slotDownloadFile);
                    break;
                case PimCommon::StorageServiceAbstract::UploadFileCapability:
                    menuService->setText(i18n("Upload File..."));
                    connect(act, &QAction::triggered, this, &StorageServiceManager::slotShareFile);
                    break;
                case PimCommon::StorageServiceAbstract::ShareLinkCapability:
                    menuService->setText(i18n("Share File..."));
                    connect(act, &QAction::triggered, this, &StorageServiceManager::slotShareFile);
                    break;
                case PimCommon::StorageServiceAbstract::DeleteFileCapability:
                    menuService->setText(i18n("Delete File..."));
                    connect(act, &QAction::triggered, this, &StorageServiceManager::slotDeleteFile);
                    break;
                case PimCommon::StorageServiceAbstract::AccountInfoCapability:
                    menuService->setText(i18n("Account Info..."));
                    connect(act, &QAction::triggered, this, &StorageServiceManager::slotAccountInfo);
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
    QAction *act = qobject_cast< QAction * >(sender());
    if (act) {
        const QString type = act->data().toString();
        if (d->mListService.contains(type)) {
            StorageServiceAbstract *service = d->mListService.value(type);
            if (service && service->hasUploadOrDownloadInProgress()) {
                KMessageBox::information(Q_NULLPTR, i18n("There is still an upload in progress."));
            } else {
                const QString fileName = QFileDialog::getOpenFileName(Q_NULLPTR, i18n("File to upload"));
                if (!fileName.isEmpty()) {
                    QFileInfo info(fileName);
                    const QRegExp disallowedSymbols = QRegExp(service->disallowedSymbols());
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
                    connect(service, &StorageServiceAbstract::uploadDownloadFileProgress, this, &StorageServiceManager::uploadDownloadFileProgress, Qt::UniqueConnection);
                    connect(service, &StorageServiceAbstract::uploadFileDone, this, &StorageServiceManager::uploadFileDone, Qt::UniqueConnection);
                    connect(service, &StorageServiceAbstract::uploadFileFailed, this, &StorageServiceManager::uploadFileFailed, Qt::UniqueConnection);
                    connect(service, &StorageServiceAbstract::shareLinkDone, this, &StorageServiceManager::shareLinkDone, Qt::UniqueConnection);
                    Q_EMIT uploadFileStart(service);
                    PimCommon::StorageServiceProgressManager::self()->addProgress(service, StorageServiceProgressManager::Upload);
                    service->uploadFile(fileName, newName, QString());
                }
            }
        }
    }
}

void StorageServiceManager::slotDownloadFile()
{
    QAction *act = qobject_cast< QAction * >(sender());
    if (act) {
        const QString type = act->data().toString();
        StorageServiceAbstract *service = d->mListService.value(type);
        if (service) {
            QPointer<PimCommon::StorageServiceDownloadDialog> dlg = new PimCommon::StorageServiceDownloadDialog(service, Q_NULLPTR);
            dlg->setDefaultDownloadPath(d->mDefaultUploadFolder);
            dlg->exec();
            delete dlg;
        }
    }
}

void StorageServiceManager::defaultConnect(StorageServiceAbstract *service)
{
    connect(service, &StorageServiceAbstract::actionFailed, this, &StorageServiceManager::actionFailed, Qt::UniqueConnection);
    connect(service, &StorageServiceAbstract::authenticationDone, this, &StorageServiceManager::authenticationDone, Qt::UniqueConnection);
    connect(service, &StorageServiceAbstract::authenticationFailed, this, &StorageServiceManager::authenticationFailed, Qt::UniqueConnection);
}

void StorageServiceManager::slotDeleteFile()
{
    QAction *act = qobject_cast< QAction * >(sender());
    if (act) {
        const QString type = act->data().toString();
        StorageServiceAbstract *service = d->mListService.value(type);
        if (service) {
            QPointer<StorageServiceDeleteDialog> dlg = new StorageServiceDeleteDialog(StorageServiceDeleteDialog::DeleteFiles, service);
            defaultConnect(service);
            connect(dlg.data(), &StorageServiceDeleteDialog::deleteFileDone, this, &StorageServiceManager::deleteFileDone);
            dlg->exec();
            delete dlg;
        }
    }
}

void StorageServiceManager::slotDeleteFolder()
{
    QAction *act = qobject_cast< QAction * >(sender());
    if (act) {
        const QString type = act->data().toString();
        StorageServiceAbstract *service = d->mListService.value(type);
        if (service) {
            QPointer<StorageServiceDeleteDialog> dlg = new StorageServiceDeleteDialog(StorageServiceDeleteDialog::DeleteFolders, service);
            defaultConnect(service);
            connect(dlg.data(), &StorageServiceDeleteDialog::deleteFolderDone, this, &StorageServiceManager::deleteFolderDone);
            dlg->exec();
            delete dlg;
        }
    }
}

void StorageServiceManager::slotAccountInfo()
{
    QAction *act = qobject_cast< QAction * >(sender());
    if (act) {
        const QString type = act->data().toString();
        StorageServiceAbstract *service = d->mListService.value(type);
        if (service) {
            defaultConnect(service);
            connect(service, &StorageServiceAbstract::accountInfoDone, this, &StorageServiceManager::accountInfoDone, Qt::UniqueConnection);
            service->accountInfo();
        }
    }
}

void StorageServiceManager::readConfig()
{
    d->mListService.clear();
    KConfig conf(kconfigName());
    KConfigGroup grp(&conf, QStringLiteral("General"));

    const QStringList services = grp.readEntry("Services", QStringList());
    Q_FOREACH (const QString &service, services) {
        PimCommon::StorageServiceAbstract *storageService = Q_NULLPTR;
        if (service == serviceName(DropBox)) {
            if (!d->mListService.contains(serviceName(DropBox))) {
                storageService = new DropBoxStorageService();
            }
        } else if (service == serviceName(Hubic)) {
            if (!d->mListService.contains(serviceName(Hubic))) {
                storageService = new HubicStorageService();
            }
        } else if (service == serviceName(YouSendIt)) {
            if (!d->mListService.contains(serviceName(YouSendIt))) {
                storageService = new YouSendItStorageService();
            }
        } else if (service == serviceName(WebDav)) {
            if (!d->mListService.contains(serviceName(WebDav))) {
                storageService = new WebDavStorageService();
            }
        } else if (service == serviceName(Box)) {
            if (!d->mListService.contains(serviceName(Box))) {
                storageService = new BoxStorageService();
            }
#ifdef KDEPIM_STORAGESERVICE_GDRIVE
        } else if (service == serviceName(GDrive)) {
            if (!d->mListService.contains(serviceName(GDrive))) {
                storageService = new GDriveStorageService();
            }
#endif
        }
        if (storageService) {
            d->mListService.insert(service, storageService);
        }
    }
}

void StorageServiceManager::writeConfig()
{
    KConfig conf(kconfigName());
    KConfigGroup grp(&conf, QStringLiteral("General"));
    grp.writeEntry("Services", d->mListService.keys());
    conf.sync();
}

QString StorageServiceManager::description(ServiceType type)
{
    switch (type) {
    case DropBox:
        return PimCommon::DropBoxStorageService::description();
    case Hubic:
        return PimCommon::HubicStorageService::description();
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
    switch (type) {
    case DropBox:
        return PimCommon::DropBoxStorageService::serviceUrl();
    case Hubic:
        return PimCommon::HubicStorageService::serviceUrl();
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
        return QUrl();
    }
    return QUrl();
}

QString StorageServiceManager::serviceName(ServiceType type)
{
    switch (type) {
    case DropBox:
        return PimCommon::DropBoxStorageService::serviceName();
    case Hubic:
        return PimCommon::HubicStorageService::serviceName();
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
    switch (type) {
    case DropBox:
        return PimCommon::DropBoxStorageService::name();
    case Hubic:
        return PimCommon::HubicStorageService::name();
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
    switch (type) {
    case DropBox:
        return PimCommon::DropBoxStorageService::iconName();
    case Hubic:
        return PimCommon::HubicStorageService::iconName();
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
    switch (type) {
    case DropBox:
        return PimCommon::DropBoxStorageService::serviceCapabilities();
    case Hubic:
        return PimCommon::HubicStorageService::serviceCapabilities();
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
    return QStringLiteral("storageservicerc");
}

