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
#include "storageservice/dropbox/dropboxstorageservice.h"
#include "storageservice/hubic/hubicstorageservice.h"
#include "storageservice/ubuntuone/ubuntuonestorageservice.h"
#include "storageservice/yousendit/yousenditstorageservice.h"
#include "storageservice/box/boxstorageservice.h"
#include "storageservice/gdrive/gdrivestorageservice.h"
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

using namespace PimCommon;

StorageServiceSettingsWidget::StorageServiceSettingsWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *mainLayout = new QHBoxLayout;

    QVBoxLayout *vlay = new QVBoxLayout;

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

    mAccountSize = new QLabel;
    mQuota = new QLabel;
    mShared = new QLabel;
    setDefaultLabel();

    vbox->addWidget(mAccountSize);
    vbox->addWidget(mQuota);
    vbox->addWidget(mShared);
    mainLayout->addLayout(vbox);
    setLayout(mainLayout);
    connect(mListService, SIGNAL(itemSelectionChanged()), this, SLOT(slotServiceSelected()));
    updateButtons();
}

StorageServiceSettingsWidget::~StorageServiceSettingsWidget()
{

}

void StorageServiceSettingsWidget::setDefaultLabel()
{
    mAccountSize->setText(i18n("Account size:"));
    mQuota->setText(i18n("Quota:"));
    mShared->setText(i18n("Shared:"));
}

void StorageServiceSettingsWidget::updateButtons()
{
    mRemoveService->setEnabled(mListService->currentItem());
    mModifyService->setEnabled(mListService->currentItem());
}

void StorageServiceSettingsWidget::setListService(const QMap<QString, StorageServiceAbstract *> &lst, PimCommon::StorageServiceAbstract::Capability cap)
{
    mListStorageService = lst;
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
            showItem = (PimCommon::StorageServiceManager::capabilities(PimCommon::StorageServiceManager::DropBox) & cap);
        } else if (i.key() == PimCommon::StorageServiceManager::serviceName(PimCommon::StorageServiceManager::Hubic)) {
            serviceName = PimCommon::StorageServiceManager::serviceToI18n(PimCommon::StorageServiceManager::Hubic);
            type = PimCommon::StorageServiceManager::Hubic;
            icon = PimCommon::StorageServiceManager::icon(PimCommon::StorageServiceManager::Hubic);
            showItem = (PimCommon::StorageServiceManager::capabilities(PimCommon::StorageServiceManager::Hubic) & cap);
        } else if (i.key() == PimCommon::StorageServiceManager::serviceName(PimCommon::StorageServiceManager::UbuntuOne)) {
            serviceName = PimCommon::StorageServiceManager::serviceToI18n(PimCommon::StorageServiceManager::UbuntuOne);
            type = PimCommon::StorageServiceManager::UbuntuOne;
            icon = PimCommon::StorageServiceManager::icon(PimCommon::StorageServiceManager::UbuntuOne);
        } else if (i.key() == PimCommon::StorageServiceManager::serviceName(PimCommon::StorageServiceManager::YouSendIt)) {
            serviceName = PimCommon::StorageServiceManager::serviceToI18n(PimCommon::StorageServiceManager::YouSendIt);
            showItem = (PimCommon::StorageServiceManager::capabilities(PimCommon::StorageServiceManager::YouSendIt) & cap);
            type = PimCommon::StorageServiceManager::YouSendIt;
            icon = PimCommon::StorageServiceManager::icon(PimCommon::StorageServiceManager::YouSendIt);
        } else if (i.key() == PimCommon::StorageServiceManager::serviceName(PimCommon::StorageServiceManager::WebDav)) {
            serviceName = PimCommon::StorageServiceManager::serviceToI18n(PimCommon::StorageServiceManager::WebDav);
            type = PimCommon::StorageServiceManager::WebDav;
            icon = PimCommon::StorageServiceManager::icon(PimCommon::StorageServiceManager::WebDav);
            showItem = (PimCommon::StorageServiceManager::capabilities(PimCommon::StorageServiceManager::WebDav) & cap);
        } else if (i.key() == PimCommon::StorageServiceManager::serviceName(PimCommon::StorageServiceManager::Box)) {
            serviceName = PimCommon::StorageServiceManager::serviceToI18n(PimCommon::StorageServiceManager::Box);
            type = PimCommon::StorageServiceManager::Box;
            icon = PimCommon::StorageServiceManager::icon(PimCommon::StorageServiceManager::Box);
            showItem = (PimCommon::StorageServiceManager::capabilities(PimCommon::StorageServiceManager::Box) & cap);
        } else if (i.key() == PimCommon::StorageServiceManager::serviceName(PimCommon::StorageServiceManager::GDrive)) {
            serviceName = PimCommon::StorageServiceManager::serviceToI18n(PimCommon::StorageServiceManager::GDrive);
            type = PimCommon::StorageServiceManager::GDrive;
            icon = PimCommon::StorageServiceManager::icon(PimCommon::StorageServiceManager::GDrive);
            showItem = (PimCommon::StorageServiceManager::capabilities(PimCommon::StorageServiceManager::GDrive) & cap);
        }
        PimCommon::StorageListWidgetItem *item = createItem(serviceName, i.key(), type, icon.isEmpty() ? KIcon() : KIcon(icon));
        if (showItem) {
            connect(i.value(),SIGNAL(authenticationFailed(QString,QString)), this, SLOT(slotAuthenticationFailed(QString,QString)));
            connect(i.value(),SIGNAL(authenticationDone(QString)), this, SLOT(slotAuthenticationDone(QString)));
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
    if (mListService->currentItem()) {
        if (KMessageBox::Yes == KMessageBox::questionYesNo(this, i18n("Do you want to delete this service '%1'?", mListService->currentItem()->text()), i18n("Delete Service") )) {
            QListWidgetItem *item = mListService->currentItem();
            const QString serviceName = item->data(Name).toString();
            if (mListStorageService.contains(serviceName)) {
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
    QPointer<AddServiceStorageDialog> dlg = new AddServiceStorageDialog(mListStorageService.keys(), this);
    if (dlg->exec()) {
        const PimCommon::StorageServiceManager::ServiceType type = dlg->serviceSelected();
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
        case PimCommon::StorageServiceManager::UbuntuOne: {
            storage = new PimCommon::UbuntuoneStorageService;
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
        case PimCommon::StorageServiceManager::GDrive: {
            storage = new PimCommon::GDriveStorageService;
            break;
        }
        default:
            break;
        }
        if (storage) {
            mListStorageService.insert(service, storage);
            PimCommon::StorageListWidgetItem *item = createItem(serviceName, service, type, storage->icon());
            item->startAnimation();
            connect(storage,SIGNAL(authenticationFailed(QString,QString)), this, SLOT(slotAuthenticationFailed(QString,QString)));
            connect(storage,SIGNAL(authenticationDone(QString)), this, SLOT(slotAuthenticationDone(QString)));
            storage->authentication();
        }
    }
    delete dlg;
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
    if (mListService->currentItem()) {
        const PimCommon::StorageServiceManager::ServiceType type = static_cast<PimCommon::StorageServiceManager::ServiceType>(mListService->currentItem()->data(Type).toInt());
        const QString description = PimCommon::StorageServiceManager::description(type);
        const QString name = PimCommon::StorageServiceManager::serviceToI18n(type);
        const QUrl serviceUrl = PimCommon::StorageServiceManager::serviceUrl(type);
        const QString descriptionStr = QLatin1String("<b>") + i18n("Name: %1",name) + QLatin1String("</b><br>") + description + QLatin1String("<br>") +
                QString::fromLatin1("<a href=\"%1\">").arg(serviceUrl.toString()) + serviceUrl.toString() + QLatin1String("</a>");
        mDescription->setText(descriptionStr);
        if (mListStorageService.contains(mListService->currentItem()->data(Name).toString())) {
            StorageServiceAbstract *storage = mListStorageService.value(mListService->currentItem()->data(Name).toString());
            connect(storage, SIGNAL(accountInfoDone(QString,PimCommon::AccountInfo)), this, SLOT(slotUpdateAccountInfo(QString,PimCommon::AccountInfo)),Qt::UniqueConnection);
            connect(storage, SIGNAL(actionFailed(QString,QString)), this, SLOT(slotUpdateAccountInfoFailed(QString,QString)),Qt::UniqueConnection);
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
        setDefaultLabel();
    }
}

void StorageServiceSettingsWidget::slotUpdateAccountInfo(const QString &serviceName, const PimCommon::AccountInfo &info)
{
    if (mListService->currentItem() && (mListService->currentItem()->data(Name).toString()==serviceName)) {
        if (info.accountSize != -1) {
            mAccountSize->setText(i18n("Account size: %1", KGlobal::locale()->formatByteSize(info.accountSize,1)));
        } else {
            mAccountSize->setText(i18n("Account size:"));
        }
        if (info.quota != -1) {
            mQuota->setText(i18n("Quota: %1", KGlobal::locale()->formatByteSize(info.quota,1)));
        } else {
            mQuota->setText(i18n("Quota:"));
        }
        if (info.accountSize != -1) {
            mShared->setText(i18n("Shared: %1", KGlobal::locale()->formatByteSize(info.accountSize,1)));
        } else {
            mShared->setText(i18n("Shared:"));
        }
    }
}

void StorageServiceSettingsWidget::slotModifyService()
{
    if (mListService->currentItem()) {
        const QString serviceName = mListService->currentItem()->data(Name).toString();
        if (mListStorageService.contains(serviceName)) {
            StorageServiceAbstract *storage = mListStorageService.value(serviceName);
            storage->authentication();
        }
    }
}
