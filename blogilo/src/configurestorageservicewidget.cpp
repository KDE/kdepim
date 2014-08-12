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
#include "settings.h"
#include "pimcommon/storageservice/widgets/storageserviceconfigurewidget.h"
#include "pimcommon/storageservice/settings/storageservicesettingswidget.h"
#include "pimcommon/storageservice/storageservicemanager.h"

#include <KLocalizedString>
#include <KStandardDirs>
#include <KMessageBox>
#include <KUrlRequester>
#include <KUrl>

#include <QVBoxLayout>
#include <QProcess>
#include <QPushButton>
#include <QStandardPaths>

StorageServiceConfigureWidget::StorageServiceConfigureWidget(QWidget *parent)
    : PimCommon::StorageServiceConfigureWidget(parent)
{

}

StorageServiceConfigureWidget::~StorageServiceConfigureWidget()
{

}

void StorageServiceConfigureWidget::loadSettings()
{
    downloadFolder()->setUrl(KUrl(Settings::self()->downloadDirectory()));
}

void StorageServiceConfigureWidget::writeSettings()
{
    Settings::self()->setDownloadDirectory(downloadFolder()->url().path());
    Settings::self()->save();
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
    if (QStandardPaths::findExecutable(QLatin1String("storageservicemanager")).isEmpty()) {
        mManageStorageService->setEnabled(false);
    } else {
        connect(mManageStorageService, &QPushButton::clicked, this, &ConfigureStorageServiceWidget::slotManageStorageService);
    }
    setLayout(lay);
    //TODO need to implement save/load from KDialogConfig

    QList<PimCommon::StorageServiceAbstract::Capability> lst;
    lst.append(PimCommon::StorageServiceAbstract::UploadFileCapability);
    lst.append(PimCommon::StorageServiceAbstract::DownloadFileCapability);

    mStorageServiceConfigureWidget->storageServiceSettingsWidget()->setListService(mStorageManager->listService(), lst);

    doLoadFromGlobalSettings();
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
    mStorageServiceConfigureWidget->loadSettings();
}

#include "moc_configurestorageservicewidget.cpp"
