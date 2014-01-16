/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "storageservicemanagermainwindow.h"
#include "storageservicetabwidget.h"
#include "storageserviceconfiguredialog.h"
#include "storageservicemanagersettingsjob.h"
#include "pimcommon/storageservice/storageservicemanager.h"
#include "pimcommon/storageservice/storageservicejobconfig.h"
#include "pimcommon/storageservice/storageserviceabstract.h"


#include <KStandardAction>
#include <KLocalizedString>
#include <KActionCollection>
#include <KApplication>
#include <KConfigGroup>
#include <KAction>
#include <KStatusBar>

#include <QPointer>
#include <QLabel>


StorageServiceManagerMainWindow::StorageServiceManagerMainWindow()
    : KXmlGuiWindow()
{
    StorageServiceManagerSettingsJob *settingsJob = new StorageServiceManagerSettingsJob(this);
    PimCommon::StorageServiceJobConfig *configJob = PimCommon::StorageServiceJobConfig::self();
    configJob->registerConfigIf(settingsJob);

    mStorageManager = new PimCommon::StorageServiceManager(this);
    mStorageServiceTabWidget = new StorageServiceTabWidget;
    connect(mStorageServiceTabWidget, SIGNAL(currentChanged(QWidget*)), this, SLOT(slotUpdateActions()));
    connect(mStorageServiceTabWidget, SIGNAL(updateStatusBarMessage(QString)), this, SLOT(slotSetStatusBarMessage(QString)));
    setCentralWidget(mStorageServiceTabWidget);

    connect( Solid::Networking::notifier(), SIGNAL(statusChanged(Solid::Networking::Status)),
              this, SLOT(slotSystemNetworkStatusChanged(Solid::Networking::Status)) );

    setupActions();
    setupGUI();
    readConfig();
    mStorageServiceTabWidget->setListStorageService(mStorageManager->listService());
    mStatusBarInfo = new QLabel;
    statusBar()->insertWidget(0, mStatusBarInfo);
    slotUpdateActions();
}

StorageServiceManagerMainWindow::~StorageServiceManagerMainWindow()
{
    KSharedConfig::Ptr config = KGlobal::config();

    KConfigGroup group = config->group( QLatin1String("StorageServiceManagerMainWindow") );
    group.writeEntry( "Size", size() );
}

void StorageServiceManagerMainWindow::slotSystemNetworkStatusChanged(Solid::Networking::Status status)
{
    if ( status == Solid::Networking::Connected || status == Solid::Networking::Unknown) {
    } else {
    }
}

void StorageServiceManagerMainWindow::slotUpdateActions()
{
    const PimCommon::StorageServiceAbstract::Capabilities capabilities = mStorageServiceTabWidget->capabilities();
    mDownloadFile->setEnabled(capabilities & PimCommon::StorageServiceAbstract::DownloadFileCapability);
    mCreateFolder->setEnabled(capabilities & PimCommon::StorageServiceAbstract::CreateFolderCapability);
    mAccountInfo->setEnabled(capabilities & PimCommon::StorageServiceAbstract::AccountInfoCapability);
    mUploadFile->setEnabled(capabilities & PimCommon::StorageServiceAbstract::UploadFileCapability);
    mDeleteFile->setEnabled(capabilities & PimCommon::StorageServiceAbstract::DeleteFileCapability);
    mAuthenticate->setDisabled(capabilities & PimCommon::StorageServiceAbstract::NoCapability);
    mRefreshList->setDisabled(capabilities & PimCommon::StorageServiceAbstract::NoCapability);
}

void StorageServiceManagerMainWindow::setupActions()
{
    KActionCollection *ac = actionCollection();
    KStandardAction::quit(this, SLOT(slotClose()), ac );

    KAction *act = ac->addAction(QLatin1String("add_storage_service"), this, SLOT(slotAddStorageService()));
    act->setText(i18n("Add Storage Service..."));

    mAuthenticate = ac->addAction(QLatin1String("authenticate"), mStorageServiceTabWidget, SLOT(slotAuthenticate()));
    mAuthenticate->setText(i18n("Authenticate..."));

    mCreateFolder = ac->addAction(QLatin1String("create_folder"), mStorageServiceTabWidget, SLOT(slotCreateFolder()));
    mCreateFolder->setText(i18n("Create Folder..."));

    mRefreshList = ac->addAction(QLatin1String("refresh_list"), mStorageServiceTabWidget, SLOT(slotRefreshList()));
    mRefreshList->setText(i18n("Refresh List"));
    mRefreshList->setShortcut(QKeySequence( Qt::Key_F5 ));

    mAccountInfo = ac->addAction(QLatin1String("account_info"), mStorageServiceTabWidget, SLOT(slotAccountInfo()));
    mAccountInfo->setText(i18n("Account Info..."));

    mUploadFile = ac->addAction(QLatin1String("upload_file"), mStorageServiceTabWidget, SLOT(slotUploadFile()));
    mUploadFile->setText(i18n("Upload File..."));

    mDeleteFile = ac->addAction(QLatin1String("delete_file"), mStorageServiceTabWidget, SLOT(slotDeleteFile()));
    mDeleteFile->setText(i18n("Delete File..."));

    mDownloadFile = ac->addAction(QLatin1String("download_file"), mStorageServiceTabWidget, SLOT(slotDownloadFile()));
    mDownloadFile->setText(i18n("Download File..."));

    KStandardAction::preferences( this, SLOT(slotConfigure()), ac );
}

void StorageServiceManagerMainWindow::slotAddStorageService()
{

}

void StorageServiceManagerMainWindow::slotClose()
{
    if (!mStorageServiceTabWidget->hasUploadDownloadProgress()) {
        close();
    }
}

void StorageServiceManagerMainWindow::slotConfigure()
{
    QPointer<StorageServiceConfigureDialog> dlg = new StorageServiceConfigureDialog(this);
    dlg->setListService(mStorageManager->listService());
    if (dlg->exec()) {
        mStorageManager->setListService(dlg->listService());
        mStorageServiceTabWidget->updateListService(dlg->listService());
        dlg->writeSettings();
    }
    delete dlg;
}

void StorageServiceManagerMainWindow::readConfig()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = KConfigGroup( config, "StorageServiceManagerMainWindow" );
    const QSize sizeDialog = group.readEntry( "Size", QSize(800,600) );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    }
}

void StorageServiceManagerMainWindow::slotSetStatusBarMessage(const QString &message)
{
    mStatusBarInfo->setText(message);
}

