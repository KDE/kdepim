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
#include "settings/pimcommonsettings.h"
#include <KLocale>
#include <KMessageBox>
#include <KTextEdit>

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

    mDescription = new KTextEdit;
    mDescription->setReadOnly(true);
    mDescription->enableFindReplace(false);

    mainLayout->addWidget(mDescription);
    setLayout(mainLayout);
    connect(mListService, SIGNAL(itemSelectionChanged()), this, SLOT(slotServiceSelected()));
}

StorageServiceSettingsWidget::~StorageServiceSettingsWidget()
{

}

void StorageServiceSettingsWidget::slotRemoveService()
{
    if (mListService->currentItem()) {
        if (KMessageBox::Yes == KMessageBox::questionYesNo(this, i18n("Delete Service"), i18n("Do you want to delete this service '%1'?", mListService->currentItem()->text()))) {
            QListWidgetItem *item = mListService->currentItem();
            mListServiceRemoved.append(item->data(Name).toString());
            delete item;
        }
    }
}

void StorageServiceSettingsWidget::slotAddService()
{
    QPointer<AddServiceStorageDialog> dlg = new AddServiceStorageDialog(this);
    if (dlg->exec()) {
        const PimCommon::StorageServiceManager::ServiceType type = dlg->serviceSelected();
        const QString serviceName = PimCommon::StorageServiceManager::serviceToI18n(type);
        const QString service = PimCommon::StorageServiceManager::serviceName(type);
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(serviceName);
        item->setData(Name,service);
        item->setData(Type, type);
        mListService->addItem(item);
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
                QString::fromLatin1("<a href=\"%1\">").arg(serviceUrl.path()) + serviceUrl.path() + QLatin1String("</a>");
        mDescription->setText(descriptionStr);
    }
}

void StorageServiceSettingsWidget::loadConfig()
{
    const QStringList services = PimCommon::PimCommonSettings::self()->services();
    Q_FOREACH(const QString &service, services) {
        QString serviceName;
        PimCommon::StorageServiceManager::ServiceType type = PimCommon::StorageServiceManager::Unknown;
        if (service == PimCommon::StorageServiceManager::serviceName(PimCommon::StorageServiceManager::DropBox)) {
            serviceName = PimCommon::StorageServiceManager::serviceToI18n(PimCommon::StorageServiceManager::DropBox);
            type = PimCommon::StorageServiceManager::DropBox;
        } else if (service == PimCommon::StorageServiceManager::serviceName(PimCommon::StorageServiceManager::Hubic)) {
            serviceName = PimCommon::StorageServiceManager::serviceToI18n(PimCommon::StorageServiceManager::Hubic);
            type = PimCommon::StorageServiceManager::Hubic;
        }

        if (serviceName.isEmpty())
            continue;
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(serviceName);
        item->setData(Name,service);
        item->setData(Type, type);
        mListService->addItem(item);
    }
}

void StorageServiceSettingsWidget::writeConfig()
{
    QStringList lst;
    for (int i=0; i < mListService->count(); ++i) {
        const QString serviceName = mListService->item(i)->data(Name).toString();
        if (!lst.contains(serviceName)) {
            lst.append(serviceName);
        }
    }
    PimCommon::PimCommonSettings::self()->setServices(lst);
}

QStringList StorageServiceSettingsWidget::listServiceRemoved() const
{
    return mListServiceRemoved;
}

