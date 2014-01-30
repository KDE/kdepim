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

#include "configurestorageservicewidget.h"
#include "pimcommon/storageservice/widgets/storageserviceconfigurewidget.h"
#include "pimcommon/storageservice/settings/storageservicesettingswidget.h"
#include "pimcommon/storageservice/storageservicemanager.h"

#include <KLocalizedString>
#include <KStandardDirs>
#include <KMessageBox>

#include <QVBoxLayout>
#include <QProcess>
#include <QPushButton>

StorageServiceConfigureWidget::StorageServiceConfigureWidget(QWidget *parent)
    : PimCommon::StorageServiceConfigureWidget(parent)
{

}

StorageServiceConfigureWidget::~StorageServiceConfigureWidget()
{

}

void StorageServiceConfigureWidget::loadSettings()
{
    //TODO
}

void StorageServiceConfigureWidget::writeSettings()
{
    //TODO
}

ConfigureStorageServiceWidget::ConfigureStorageServiceWidget(PimCommon::StorageServiceManager *storageManager, QWidget *parent)
    : QWidget(parent),
      mStorageManager(storageManager)
{
    QVBoxLayout *lay = new QVBoxLayout;
    mStorageServiceConfigureWidget = new StorageServiceConfigureWidget;
    connect(mStorageServiceConfigureWidget, SIGNAL(changed()), this, SIGNAL(changed()));
    lay->addWidget(mStorageServiceConfigureWidget);

    QHBoxLayout *hbox = new QHBoxLayout;
    mManageStorageService = new QPushButton(i18n("Manage Storage Service"));
    hbox->addWidget(mManageStorageService);
    hbox->addStretch();
    lay->addLayout(hbox);
    if (KStandardDirs::findExe(QLatin1String("storageservicemanager")).isEmpty()) {
        mManageStorageService->setEnabled(false);
    } else {
        connect(mManageStorageService, SIGNAL(clicked(bool)), this, SLOT(slotManageStorageService()));
    }
    setLayout(lay);
}

ConfigureStorageServiceWidget::~ConfigureStorageServiceWidget()
{
}

void ConfigureStorageServiceWidget::slotManageStorageService()
{
    if ( !QProcess::startDetached(QLatin1String("storageservicemanager") ) )
        KMessageBox::error( this, i18n( "Could not start storage service manager; "
                                        "please check your installation." ),
                            i18n( "KMail Error" ) );
}

void ConfigureStorageServiceWidget::save()
{
    mStorageManager->setListService(mStorageServiceConfigureWidget->storageServiceSettingsWidget()->listService());
    mStorageServiceConfigureWidget->writeSettings();
}

void ConfigureStorageServiceWidget::doLoadFromGlobalSettings()
{
    //FIXME capabilities
    mStorageServiceConfigureWidget->storageServiceSettingsWidget()->setListService(mStorageManager->listService(), PimCommon::StorageServiceAbstract::ShareLinkCapability);
    mStorageServiceConfigureWidget->loadSettings();
}

#include "moc_configurestorageservicewidget.cpp"
