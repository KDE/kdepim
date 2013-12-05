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

#include "storageservicesettingswidget.h"
#include "addservicestoragedialog.h"
#include "storageservice/storageservicemanager.h"
#include "storageservice/dropbox/dropboxstorageservice.h"
#include "storageservice/hubic/hubicstorageservice.h"
#include "storageservice/ubuntuone/ubuntuonestorageservice.h"
#include "storageservice/yousendit/yousenditstorageservice.h"
#include "settings/pimcommonsettings.h"
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

    vlay->addLayout(hlay);


    mainLayout->addLayout(vlay);

    QVBoxLayout *vbox = new QVBoxLayout;
    mDescription = new KTextBrowser;
    mDescription->setReadOnly(true);
    vbox->addWidget(mDescription);

    mAccountSize = new QLabel;
    mQuota = new QLabel;
    mShared = new QLabel;

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

void StorageServiceSettingsWidget::updateButtons()
{
    mRemoveService->setEnabled(mListService->currentItem());
}

void StorageServiceSettingsWidget::setListService(const QMap<QString, StorageServiceAbstract *> &lst)
{
    mListStorageService = lst;
    QMapIterator<QString, StorageServiceAbstract*> i(mListStorageService);
    while (i.hasNext()) {
        i.next();
        QString serviceName;
        PimCommon::StorageServiceManager::ServiceType type = PimCommon::StorageServiceManager::Unknown;
        if (i.key() == PimCommon::StorageServiceManager::serviceName(PimCommon::StorageServiceManager::DropBox)) {
            serviceName = PimCommon::StorageServiceManager::serviceToI18n(PimCommon::StorageServiceManager::DropBox);
            type = PimCommon::StorageServiceManager::DropBox;
        } else if (i.key() == PimCommon::StorageServiceManager::serviceName(PimCommon::StorageServiceManager::Hubic)) {
            serviceName = PimCommon::StorageServiceManager::serviceToI18n(PimCommon::StorageServiceManager::Hubic);
            type = PimCommon::StorageServiceManager::Hubic;
        } else if (i.key() == PimCommon::StorageServiceManager::serviceName(PimCommon::StorageServiceManager::UbuntuOne)) {
            serviceName = PimCommon::StorageServiceManager::serviceToI18n(PimCommon::StorageServiceManager::UbuntuOne);
            type = PimCommon::StorageServiceManager::UbuntuOne;
        } else if (i.key() == PimCommon::StorageServiceManager::serviceName(PimCommon::StorageServiceManager::YouSendIt)) {
            serviceName = PimCommon::StorageServiceManager::serviceToI18n(PimCommon::StorageServiceManager::YouSendIt);
            type = PimCommon::StorageServiceManager::YouSendIt;
        }
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(serviceName);
        item->setData(Name, i.key());
        item->setData(Type, type);
        mListService->addItem(item);
    }
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
        default:
            break;
        }
        if (storage) {
            QListWidgetItem *item = new QListWidgetItem;
            item->setText(serviceName);
            item->setData(Name,service);
            item->setData(Type, type);
            mListService->addItem(item);
            storage->authentification();
            mListStorageService.insert(service, storage);
        }
    }
    delete dlg;
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
            connect(storage, SIGNAL(accountInfoDone(QString,PimCommon::AccountInfo)), this, SLOT(slotUpdateAccountInfo(QString, PimCommon::AccountInfo)),Qt::UniqueConnection);
            storage->accountInfo();
        }
    } else {
        mDescription->clear();
        mAccountSize->clear();
        mQuota->clear();
        mShared->clear();
    }
    updateButtons();
}

void StorageServiceSettingsWidget::slotUpdateAccountInfo(const QString &serviceName, const PimCommon::AccountInfo &info)
{
    if (mListService->currentItem() && (mListService->currentItem()->data(Name).toString()==serviceName)) {
        if (info.accountSize != -1) {
            mAccountSize->setText(i18n("Account size: %1", KGlobal::locale()->formatByteSize(info.accountSize,1)));
        } else {
            mAccountSize->clear();
        }
        if (info.quota != -1) {
            mQuota->setText(i18n("Quota: %1", KGlobal::locale()->formatByteSize(info.quota,1)));
        } else {
            mQuota->clear();
        }
        if (info.accountSize != -1) {
            mShared->setText(i18n("Shared: %1", KGlobal::locale()->formatByteSize(info.accountSize,1)));
        } else {
            mShared->clear();
        }
    }
}
