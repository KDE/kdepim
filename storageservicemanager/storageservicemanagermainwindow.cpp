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
#include "storageservicemanagerglobalconfig.h"
#include "pimcommon/storageservice/storageservicemanager.h"
#include "pimcommon/storageservice/storageserviceprogressmanager.h"
#include "pimcommon/storageservice/storageservicejobconfig.h"
#include "pimcommon/storageservice/storageserviceabstract.h"

#include "pimcommon/storageservice/settings/storageservicesettings.h"

#include "libkdepim/progresswidget/progressstatusbarwidget.h"
#include "libkdepim/progresswidget/statusbarprogresswidget.h"

#include <KStandardAction>
#include <KLocalizedString>
#include <KActionCollection>
#include <KApplication>
#include <KConfigGroup>
#include <KAction>
#include <KStatusBar>
#include <KMessageBox>
#include <knotifyconfigwidget.h>

#include <QPointer>
#include <QCloseEvent>
#include <QLabel>
#include <QDebug>



StorageServiceManagerMainWindow::StorageServiceManagerMainWindow()
    : KXmlGuiWindow(),
      mNetworkIsDown(false)
{
    StorageServiceManagerSettingsJob *settingsJob = new StorageServiceManagerSettingsJob(this);
    PimCommon::StorageServiceJobConfig *configJob = PimCommon::StorageServiceJobConfig::self();
    configJob->registerConfigIf(settingsJob);

    mStorageManager = new PimCommon::StorageServiceManager(this);
    mStorageServiceTabWidget = new StorageServiceTabWidget;
    connect(mStorageServiceTabWidget, SIGNAL(currentChanged(QWidget*)), this, SLOT(slotUpdateActions()));
    connect(mStorageServiceTabWidget, SIGNAL(updateStatusBarMessage(QString)), this, SLOT(slotSetStatusBarMessage(QString)));
    connect(mStorageServiceTabWidget, SIGNAL(listFileWasInitialized()), this, SLOT(slotUpdateActions()));
    setCentralWidget(mStorageServiceTabWidget);

    connect( Solid::Networking::notifier(), SIGNAL(statusChanged(Solid::Networking::Status)),
              this, SLOT(slotSystemNetworkStatusChanged(Solid::Networking::Status)) );

    setupActions();
    setupGUI(Keys | StatusBar | Save | Create);
    readConfig();
    mStorageServiceTabWidget->setListStorageService(mStorageManager->listService());
    slotUpdateActions();
    initStatusBar();
    const Solid::Networking::Status status = Solid::Networking::status();
    slotSystemNetworkStatusChanged(status);
}

StorageServiceManagerMainWindow::~StorageServiceManagerMainWindow()
{
    delete mStorageServiceTabWidget;
    KSharedConfig::Ptr config = KGlobal::config();

    KConfigGroup group = config->group( QLatin1String("StorageServiceManagerMainWindow") );
    group.writeEntry( "Size", size() );
    qDebug()<<" StorageServiceManagerMainWindow::~StorageServiceManagerMainWindow()";
    if (StorageServiceManagerGlobalConfig::self()->closeWallet())
        PimCommon::StorageServiceSettings::self()->closeWallet();
}

void StorageServiceManagerMainWindow::initStatusBar()
{
    mStatusBarInfo = new QLabel;
    statusBar()->insertWidget(0, mStatusBarInfo, 4);
    KPIM::ProgressStatusBarWidget *progressBar = new KPIM::ProgressStatusBarWidget( statusBar(), this, PimCommon::StorageServiceProgressManager::progressTypeValue() );
    statusBar()->addPermanentWidget( progressBar->littleProgress(), 0 );
}

void StorageServiceManagerMainWindow::slotSystemNetworkStatusChanged(Solid::Networking::Status status)
{
    if ( status == Solid::Networking::Connected || status == Solid::Networking::Unknown) {
        mStorageServiceTabWidget->setNetworkIsDown(false);
        slotSetStatusBarMessage(i18n("Network connection is up."));
        mNetworkIsDown = false;
    } else {
        mStorageServiceTabWidget->setNetworkIsDown(true);
        slotSetStatusBarMessage(i18n("Network connection is down."));
        mNetworkIsDown = true;
    }
    slotUpdateActions();
}

void StorageServiceManagerMainWindow::slotUpdateActions()
{
    if (mNetworkIsDown) {
        mDownloadFile->setDisabled(true);
        mCreateFolder->setDisabled(true);
        mAccountInfo->setDisabled(true);
        mUploadFile->setDisabled(true);
        mDeleteFile->setDisabled(true);
        mAuthenticate->setDisabled(true);
        mRefreshList->setDisabled(true);
        mShowLog->setDisabled(true);
    } else {
        const PimCommon::StorageServiceAbstract::Capabilities capabilities = mStorageServiceTabWidget->capabilities();
        const bool listFolderWasLoaded = mStorageServiceTabWidget->listFolderWasLoaded();
        mDownloadFile->setEnabled(listFolderWasLoaded && (capabilities & PimCommon::StorageServiceAbstract::DownloadFileCapability));
        mCreateFolder->setEnabled(listFolderWasLoaded && (capabilities & PimCommon::StorageServiceAbstract::CreateFolderCapability));
        mAccountInfo->setEnabled(capabilities & PimCommon::StorageServiceAbstract::AccountInfoCapability);
        mUploadFile->setEnabled(capabilities & PimCommon::StorageServiceAbstract::UploadFileCapability);
        mDeleteFile->setEnabled(listFolderWasLoaded && (capabilities & PimCommon::StorageServiceAbstract::DeleteFileCapability));
        mAuthenticate->setDisabled((capabilities & PimCommon::StorageServiceAbstract::NoCapability) || (mStorageServiceTabWidget->count() == 0));
        mRefreshList->setDisabled((capabilities & PimCommon::StorageServiceAbstract::NoCapability) || (mStorageServiceTabWidget->count() == 0));
        mShowLog->setDisabled((mStorageServiceTabWidget->count() == 0));
        mLogout->setEnabled(listFolderWasLoaded);
    }
}

void StorageServiceManagerMainWindow::setupActions()
{
    KActionCollection *ac = actionCollection();
    KStandardAction::quit(this, SLOT(close()), ac );

    mAuthenticate = ac->addAction(QLatin1String("authenticate"), mStorageServiceTabWidget, SLOT(slotAuthenticate()));
    mAuthenticate->setText(i18n("Authenticate..."));

    mCreateFolder = ac->addAction(QLatin1String("create_folder"), mStorageServiceTabWidget, SLOT(slotCreateFolder()));
    mCreateFolder->setText(i18n("Create Folder..."));
    mCreateFolder->setIcon(KIcon(QLatin1String("folder-new")));

    mRefreshList = ac->addAction(QLatin1String("refresh_list"), mStorageServiceTabWidget, SLOT(slotRefreshList()));
    mRefreshList->setText(i18n("Refresh List"));
    mRefreshList->setShortcut(QKeySequence( Qt::Key_F5 ));

    mAccountInfo = ac->addAction(QLatin1String("account_info"), mStorageServiceTabWidget, SLOT(slotAccountInfo()));
    mAccountInfo->setText(i18n("Account Info..."));

    mUploadFile = ac->addAction(QLatin1String("upload_file"), mStorageServiceTabWidget, SLOT(slotUploadFile()));
    mUploadFile->setText(i18n("Upload File..."));

    mDeleteFile = ac->addAction(QLatin1String("delete_file"), mStorageServiceTabWidget, SLOT(slotDeleteFile()));
    mDeleteFile->setText(i18n("Delete File..."));
    mDeleteFile->setIcon(KIcon(QLatin1String("edit-delete")));

    mDownloadFile = ac->addAction(QLatin1String("download_file"), mStorageServiceTabWidget, SLOT(slotDownloadFile()));
    mDownloadFile->setText(i18n("Download File..."));
    mDownloadFile->setIcon(KIcon(QLatin1String("download")));

    mShowLog = ac->addAction(QLatin1String("show_log"), mStorageServiceTabWidget, SLOT(slotShowLog()));
    mShowLog->setText(i18n("Show Log..."));

    mLogout = ac->addAction(QLatin1String("logout"), this, SLOT(slotLogout()));
    mLogout->setText(i18n("Logout"));

    mShutdownAllServices = ac->addAction(QLatin1String("shutdown_all_services"), this, SLOT(slotShutdownAllServices()));
    mShutdownAllServices->setText(i18n("Shutdown All Services"));

    mRefreshAll = ac->addAction(QLatin1String("refresh_all"), this, SLOT(slotRefreshAll()));
    mRefreshAll->setText(i18n("Refresh All"));
    mRefreshAll->setShortcut(QKeySequence( Qt::CTRL + Qt::Key_F5 ));

    KStandardAction::preferences( this, SLOT(slotConfigure()), ac );
    KStandardAction::configureNotifications(this, SLOT(slotShowNotificationOptions()), ac); // options_configure_notifications
}

void StorageServiceManagerMainWindow::slotShowNotificationOptions()
{
    KNotifyConfigWidget::configure(this);
}

void StorageServiceManagerMainWindow::slotLogout()
{
    mStorageServiceTabWidget->logout();
    slotUpdateActions();
}

void StorageServiceManagerMainWindow::closeEvent(QCloseEvent *e)
{
    if (mStorageServiceTabWidget->hasUploadDownloadProgress()) {
        if (KMessageBox::No == KMessageBox::warningYesNo(this, i18n("There is still upload or download in progress. Do you want to close anyway?"))) {
            e->ignore();
            return;
        }
    }
    e->accept();
}

void StorageServiceManagerMainWindow::slotConfigure()
{
    QPointer<StorageServiceConfigureDialog> dlg = new StorageServiceConfigureDialog(this);
    connect(dlg, SIGNAL(serviceRemoved(QString)), this, SLOT(slotServiceRemoved(QString)));
    dlg->setListService(mStorageManager->listService());
    if (dlg->exec()) {
        mStorageManager->setListService(dlg->listService());
        mStorageServiceTabWidget->updateListService(dlg->listService());
        dlg->writeSettings();
    }
    delete dlg;
}

void StorageServiceManagerMainWindow::slotServiceRemoved(const QString &serviceName)
{
    mStorageServiceTabWidget->serviceRemoved(serviceName);
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

void StorageServiceManagerMainWindow::slotShutdownAllServices()
{
    mStorageServiceTabWidget->shutdownAllServices();
    slotUpdateActions();
}

void StorageServiceManagerMainWindow::slotRefreshAll()
{
    mStorageServiceTabWidget->refreshAll();
    slotUpdateActions();
}
