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

#include "storageservicesettingswidget.h"
#include "storagelistwidgetitem.h"
#include "addservicestoragedialog.h"
#include "storageservice/utils/storageserviceutils.h"
#include "storageservice/dropbox/dropboxstorageservice.h"
#include "storageservice/hubic/hubicstorageservice.h"
#include "storageservice/yousendit/yousenditstorageservice.h"
#include "storageservice/box/boxstorageservice.h"
#ifdef KDEPIM_STORAGESERVICE_GDRIVE
#include "storageservice/gdrive/gdrivestorageservice.h"
#endif
#include "storageservice/webdav/webdavstorageservice.h"
#include "settings/pimcommonsettings.h"
#include <KLocalizedString>
#include <KLocale>
#include <KMessageBox>
#include <KTextBrowser>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QPointer>
#include <QDebug>
#include <QStackedWidget>

using namespace PimCommon;

StorageServiceSettingsWidget::StorageServiceSettingsWidget(QWidget *parent)
    : QWidget(parent),
      mNeedCapability(QList<PimCommon::StorageServiceAbstract::Capability>() << PimCommon::StorageServiceAbstract::NoCapability)

{
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    QVBoxLayout *vlay = new QVBoxLayout;
    vlay->setMargin(0);
    vlay->setSpacing(0);

    mListService = new QListWidget;
    vlay->addWidget(mListService);

    QHBoxLayout *hlay = new QHBoxLayout;

    mAddService = new QPushButton(i18n("Add..."));
    connect(mAddService, SIGNAL(clicked()), this, SLOT(slotAddService()));
    hlay->addWidget(mAddService);

    mRemoveService = new QPushButton(i18n("Remove"));
    connect(mRemoveService, SIGNAL(clicked()), this, SLOT(slotRemoveService()));
    hlay->addWidget(mRemoveService);

    mModifyService = new QPushButton(i18n("Modify"));
    connect(mModifyService, SIGNAL(clicked()), this, SLOT(slotModifyService()));
    hlay->addWidget(mModifyService);


    vlay->addLayout(hlay);


    mainLayout->addLayout(vlay);

    QVBoxLayout *vbox = new QVBoxLayout;
    mDescription = new KTextBrowser;
    mDescription->setReadOnly(true);
    vbox->addWidget(mDescription);


    mStackWidget = new QStackedWidget;

    mInformationPage = new QWidget;
    mStackWidget->addWidget(mInformationPage);
    QVBoxLayout *informationLayout = new QVBoxLayout;
    mInformationPage->setLayout(informationLayout);
    mAccountSize = new QLabel;
    mQuota = new QLabel;
    mShared = new QLabel;
    informationLayout->addWidget(mAccountSize);
    informationLayout->addWidget(mQuota);
    informationLayout->addWidget(mShared);
    setDefaultLabel();

    mErrorPage = new QWidget;
    mStackWidget->addWidget(mErrorPage);
    mStackWidget->setCurrentWidget(mInformationPage);
    QVBoxLayout *errorLayout = new QVBoxLayout;
    mErrorPage->setLayout(errorLayout);
    mErrorInfo = new QLabel;
    errorLayout->addWidget(mErrorInfo);
    mAuthenticate = new QPushButton(i18n("Authenticate"));
    errorLayout->addWidget(mAuthenticate);
    connect(mAuthenticate, SIGNAL(clicked()), this, SLOT(slotAuthenticate()));


    mCanNotGetInfo = new QLabel(i18n("Unable to get account information."));
    mStackWidget->addWidget(mCanNotGetInfo);

    vbox->addWidget(mStackWidget);
    mainLayout->addLayout(vbox);
    setLayout(mainLayout);
    connect(mListService, SIGNAL(itemSelectionChanged()), this, SLOT(slotServiceSelected()));
    updateButtons();
}

StorageServiceSettingsWidget::~StorageServiceSettingsWidget()
{

}

void StorageServiceSettingsWidget::slotAuthenticate()
{
    QListWidgetItem *item = mListService->currentItem();
    if (item) {
        const QString serviceName = item->data(Name).toString();
        if (mListStorageService.contains(serviceName)) {
            StorageServiceAbstract *storage = mListStorageService.value(serviceName);
            storage->authentication();
        }
    }
}

void StorageServiceSettingsWidget::setDefaultLabel()
{
    mStackWidget->setCurrentWidget(mInformationPage);
    mAccountSize->setText(i18n("Account size:"));
    mQuota->setText(i18n("Quota:"));
    mShared->setText(i18n("Shared:"));
}

void StorageServiceSettingsWidget::updateButtons()
{
    mRemoveService->setEnabled(mListService->currentItem());
    mModifyService->setEnabled(mListService->currentItem());
}

void StorageServiceSettingsWidget::setListService(const QMap<QString, StorageServiceAbstract *> &lst, const QList<PimCommon::StorageServiceAbstract::Capability> &lstCap)
{
    mListStorageService = lst;
    mNeedCapability = lstCap;
    QMapIterator<QString, StorageServiceAbstract*> i(mListStorageService);
    while (i.hasNext()) {
        i.next();
        QString serviceName;
        PimCommon::StorageServiceManager::ServiceType type = PimCommon::StorageServiceManager::Unknown;
        QString icon;
        bool showItem = true;
        if (i.key() == PimCommon::StorageServiceManager::serviceName(PimCommon::StorageServiceManager::DropBox)) {
            serviceName = PimCommon::StorageServiceManager::serviceToI18n(PimCommon::StorageServiceManager::DropBox);
            type = PimCommon::StorageServiceManager::DropBox;
            icon = PimCommon::StorageServiceManager::icon(PimCommon::StorageServiceManager::DropBox);
            showItem = PimCommon::StorageServiceUtils::hasCapabilities(PimCommon::StorageServiceManager::capabilities(PimCommon::StorageServiceManager::DropBox), lstCap);
        } else if (i.key() == PimCommon::StorageServiceManager::serviceName(PimCommon::StorageServiceManager::Hubic)) {
            serviceName = PimCommon::StorageServiceManager::serviceToI18n(PimCommon::StorageServiceManager::Hubic);
            type = PimCommon::StorageServiceManager::Hubic;
            icon = PimCommon::StorageServiceManager::icon(PimCommon::StorageServiceManager::Hubic);
            showItem = PimCommon::StorageServiceUtils::hasCapabilities(PimCommon::StorageServiceManager::capabilities(PimCommon::StorageServiceManager::Hubic), lstCap);
        } else if (i.key() == PimCommon::StorageServiceManager::serviceName(PimCommon::StorageServiceManager::YouSendIt)) {
            serviceName = PimCommon::StorageServiceManager::serviceToI18n(PimCommon::StorageServiceManager::YouSendIt);
            showItem = PimCommon::StorageServiceUtils::hasCapabilities(PimCommon::StorageServiceManager::capabilities(PimCommon::StorageServiceManager::YouSendIt), lstCap);
            type = PimCommon::StorageServiceManager::YouSendIt;
            icon = PimCommon::StorageServiceManager::icon(PimCommon::StorageServiceManager::YouSendIt);
        } else if (i.key() == PimCommon::StorageServiceManager::serviceName(PimCommon::StorageServiceManager::WebDav)) {
            serviceName = PimCommon::StorageServiceManager::serviceToI18n(PimCommon::StorageServiceManager::WebDav);
            type = PimCommon::StorageServiceManager::WebDav;
            icon = PimCommon::StorageServiceManager::icon(PimCommon::StorageServiceManager::WebDav);
            showItem = PimCommon::StorageServiceUtils::hasCapabilities(PimCommon::StorageServiceManager::capabilities(PimCommon::StorageServiceManager::WebDav), lstCap);
        } else if (i.key() == PimCommon::StorageServiceManager::serviceName(PimCommon::StorageServiceManager::Box)) {
            serviceName = PimCommon::StorageServiceManager::serviceToI18n(PimCommon::StorageServiceManager::Box);
            type = PimCommon::StorageServiceManager::Box;
            icon = PimCommon::StorageServiceManager::icon(PimCommon::StorageServiceManager::Box);
            showItem = PimCommon::StorageServiceUtils::hasCapabilities(PimCommon::StorageServiceManager::capabilities(PimCommon::StorageServiceManager::Box), lstCap);
#ifdef KDEPIM_STORAGESERVICE_GDRIVE
        } else if (i.key() == PimCommon::StorageServiceManager::serviceName(PimCommon::StorageServiceManager::GDrive)) {
            serviceName = PimCommon::StorageServiceManager::serviceToI18n(PimCommon::StorageServiceManager::GDrive);
            type = PimCommon::StorageServiceManager::GDrive;
            icon = PimCommon::StorageServiceManager::icon(PimCommon::StorageServiceManager::GDrive);
            showItem = PimCommon::StorageServiceUtils::hasCapabilities(PimCommon::StorageServiceManager::capabilities(PimCommon::StorageServiceManager::GDrive), lstCap);
#endif
        }
        PimCommon::StorageListWidgetItem *item = createItem(serviceName, i.key(), type, icon.isEmpty() ? KIcon() : KIcon(icon));
        if (showItem) {
            defaultConnection(i.value());
        } else {
            item->setHidden(true);
        }
    }
}

PimCommon::StorageListWidgetItem *StorageServiceSettingsWidget::createItem(const QString &serviceName, const QString &service, PimCommon::StorageServiceManager::ServiceType type, const KIcon &icon)
{
    PimCommon::StorageListWidgetItem *item = new PimCommon::StorageListWidgetItem;
    item->setText(serviceName);
    item->setData(Name,service);
    item->setData(Type, type);
    if (!icon.isNull()) {
        item->setDefaultIcon(icon);
        item->setIcon(icon);
    }
    mListService->addItem(item);
    return item;
}

QMap<QString, StorageServiceAbstract *> StorageServiceSettingsWidget::listService() const
{
    return mListStorageService;
}

void StorageServiceSettingsWidget::slotRemoveService()
{
    QListWidgetItem *item = mListService->currentItem();
    if (item) {
        if (KMessageBox::Yes == KMessageBox::questionYesNo(this, i18n("Do you want to delete this service '%1'?", item->text()), i18n("Delete Service") )) {
            const QString serviceName = item->data(Name).toString();
            if (mListStorageService.contains(serviceName)) {
                Q_EMIT serviceRemoved(serviceName);
                StorageServiceAbstract *storage = mListStorageService.take(serviceName);
                storage->removeConfig();
                Q_EMIT changed();
                delete storage;
            }
            delete item;
        }
    }
}

void StorageServiceSettingsWidget::slotAddService()
{
    QPointer<AddServiceStorageDialog> dlg = new AddServiceStorageDialog(mNeedCapability, mListStorageService.keys(), this);
    if (dlg->exec()) {
        const PimCommon::StorageServiceManager::ServiceType type = dlg->serviceSelected();
        if (type != PimCommon::StorageServiceManager::Unknown) {
            const QString serviceName = PimCommon::StorageServiceManager::serviceToI18n(type);
            const QString service = PimCommon::StorageServiceManager::serviceName(type);
            StorageServiceAbstract *storage = 0;
            switch(type) {
            case PimCommon::StorageServiceManager::DropBox: {
                storage = new PimCommon::DropBoxStorageService;
                break;
            }
            case PimCommon::StorageServiceManager::Hubic: {
                storage = new PimCommon::HubicStorageService;
                break;
            }
            case PimCommon::StorageServiceManager::YouSendIt: {
                storage = new PimCommon::YouSendItStorageService;
                break;
            }
            case PimCommon::StorageServiceManager::Box: {
                storage = new PimCommon::BoxStorageService;
                break;
            }
            case PimCommon::StorageServiceManager::WebDav: {
                storage = new PimCommon::WebDavStorageService;
                break;
            }
#ifdef KDEPIM_STORAGESERVICE_GDRIVE
            case PimCommon::StorageServiceManager::GDrive: {
                storage = new PimCommon::GDriveStorageService;
                break;
            }
#endif
            default:
                break;
            }
            if (storage) {
                mListStorageService.insert(service, storage);
                PimCommon::StorageListWidgetItem *item = createItem(serviceName, service, type, storage->icon());
                item->startAnimation();
                defaultConnection(storage);
                storage->authentication();
            }
        }
    }
    delete dlg;
}

void StorageServiceSettingsWidget::defaultConnection(StorageServiceAbstract *storage)
{
    connect(storage, SIGNAL(authenticationFailed(QString,QString)), this, SLOT(slotAuthenticationFailed(QString,QString)));
    connect(storage, SIGNAL(authenticationDone(QString)), this, SLOT(slotAuthenticationDone(QString)));
    connect(storage, SIGNAL(accountInfoDone(QString,PimCommon::AccountInfo)), this, SLOT(slotUpdateAccountInfo(QString,PimCommon::AccountInfo)));
    connect(storage, SIGNAL(actionFailed(QString,QString)), this, SLOT(slotUpdateAccountInfoFailed(QString,QString)));
}

void StorageServiceSettingsWidget::slotAuthenticationFailed(const QString &serviceName, const QString &error)
{
    PimCommon::StorageListWidgetItem *item = 0;
    for (int i=0; i <mListService->count(); ++i) {
        if (mListService->item(i)->data(Name).toString() == serviceName) {
            item = static_cast<PimCommon::StorageListWidgetItem*>(mListService->item(i));
            item->stopAnimation();
            if (mListStorageService.contains(serviceName)) {
                mListStorageService.value(serviceName)->removeConfig();
            }
            mListStorageService.remove(serviceName);
            delete item;
            break;
        }
    }
    KMessageBox::error(this, error, i18n("Authentication Failed"));
}

void StorageServiceSettingsWidget::slotAuthenticationDone(const QString &serviceName)
{
    for (int i=0; i <mListService->count(); ++i) {
        if (mListService->item(i)->data(Name).toString() == serviceName) {
            PimCommon::StorageListWidgetItem *item = static_cast<PimCommon::StorageListWidgetItem*>(mListService->item(i));
            item->stopAnimation();
            Q_EMIT changed();
            break;
        }
    }
}

void StorageServiceSettingsWidget::slotServiceSelected()
{
    QListWidgetItem *item = mListService->currentItem();
    if (item) {
        const PimCommon::StorageServiceManager::ServiceType type = static_cast<PimCommon::StorageServiceManager::ServiceType>(item->data(Type).toInt());
        const QString description = PimCommon::StorageServiceManager::description(type);
        const QString name = PimCommon::StorageServiceManager::serviceToI18n(type);
        const QUrl serviceUrl = PimCommon::StorageServiceManager::serviceUrl(type);
        const QString descriptionStr = QLatin1String("<b>") + i18n("Name: %1",name) + QLatin1String("</b><br>") + description + QLatin1String("<br>") +
                QString::fromLatin1("<a href=\"%1\">").arg(serviceUrl.toString()) + serviceUrl.toString() + QLatin1String("</a>");
        mDescription->setText(descriptionStr);
        if (mListStorageService.contains(mListService->currentItem()->data(Name).toString())) {
            StorageServiceAbstract *storage = mListStorageService.value(mListService->currentItem()->data(Name).toString());
            storage->accountInfo();
        }
    } else {
        mDescription->clear();
        setDefaultLabel();
    }
    updateButtons();
}

void StorageServiceSettingsWidget::slotUpdateAccountInfoFailed(const QString &serviceName, const QString &error)
{
    Q_UNUSED(error);
    if (mListService->currentItem() && (mListService->currentItem()->data(Name).toString()==serviceName)) {
        mStackWidget->setCurrentWidget(mErrorPage);
        setDefaultLabel();
    }
}

void StorageServiceSettingsWidget::slotUpdateAccountInfo(const QString &serviceName, const PimCommon::AccountInfo &info)
{
    if (mListService->currentItem() && (mListService->currentItem()->data(Name).toString()==serviceName)) {
        if (info.isValid()) {
            mStackWidget->setCurrentWidget(mInformationPage);
            if (info.accountSize >= 0) {
                mAccountSize->setText(i18n("Account size: %1", KGlobal::locale()->formatByteSize(info.accountSize,1)));
            } else {
                mAccountSize->setText(i18n("Account size:"));
            }
            if (info.quota >= 0) {
                mQuota->setText(i18n("Quota: %1", KGlobal::locale()->formatByteSize(info.quota,1)));
            } else {
                mQuota->setText(i18n("Quota:"));
            }
            if (info.shared >= 0) {
                mShared->setText(i18n("Shared: %1", KGlobal::locale()->formatByteSize(info.shared,1)));
            } else {
                mShared->setText(i18n("Shared:"));
            }
        } else {
            mStackWidget->setCurrentWidget(mCanNotGetInfo);
        }
    }
}

void StorageServiceSettingsWidget::slotModifyService()
{
    QListWidgetItem *item = mListService->currentItem();
    if (item) {
        const QString serviceName = item->data(Name).toString();
        if (mListStorageService.contains(serviceName)) {
            StorageServiceAbstract *storage = mListStorageService.value(serviceName);
            storage->authentication();
        }
    }
}
