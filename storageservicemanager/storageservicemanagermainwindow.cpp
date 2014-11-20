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
#include "storageservicemanagermainwidget.h"
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
#include <KConfigGroup>
#include <QStatusBar>
#include <KMessageBox>
#include <knotifyconfigwidget.h>

#include <QPointer>
#include <QCloseEvent>
#include <QLabel>
#include <QDebug>
#include <KSharedConfig>
#include <QAction>
#include <QNetworkConfigurationManager>

StorageServiceManagerMainWindow::StorageServiceManagerMainWindow()
    : KXmlGuiWindow()
{
    StorageServiceManagerSettingsJob *settingsJob = new StorageServiceManagerSettingsJob(this);
    PimCommon::StorageServiceJobConfig *configJob = PimCommon::StorageServiceJobConfig::self();
    configJob->registerConfigIf(settingsJob);

    mStorageManager = new PimCommon::StorageServiceManager(this);
    connect(mStorageManager, &PimCommon::StorageServiceManager::servicesChanged, this, &StorageServiceManagerMainWindow::slotServicesChanged);
    mStorageServiceMainWidget = new StorageServiceManagerMainWidget;
    connect(mStorageServiceMainWidget, &StorageServiceManagerMainWidget::configureClicked, this, &StorageServiceManagerMainWindow::slotConfigure);
    connect(mStorageServiceMainWidget->storageServiceTabWidget(), SIGNAL(currentChanged(int)), this, SLOT(slotUpdateActions()));
    connect(mStorageServiceMainWidget->storageServiceTabWidget(), SIGNAL(updateStatusBarMessage(QString)), this, SLOT(slotSetStatusBarMessage(QString)));
    connect(mStorageServiceMainWidget->storageServiceTabWidget(), SIGNAL(listFileWasInitialized()), this, SLOT(slotUpdateActions()));
    connect(mStorageServiceMainWidget->storageServiceTabWidget(), SIGNAL(selectionChanged()), this, SLOT(slotUpdateActions()));
    setCentralWidget(mStorageServiceMainWidget);

    mNetworkConfigurationManager = new QNetworkConfigurationManager();
    connect(mNetworkConfigurationManager, &QNetworkConfigurationManager::onlineStateChanged, this, &StorageServiceManagerMainWindow::slotSystemNetworkOnlineStateChanged);

    setupActions();
    setupGUI(Keys | StatusBar | Save | Create);
    readConfig();
    mStorageServiceMainWidget->storageServiceTabWidget()->setListStorageService(mStorageManager->listService());
    slotUpdateActions();
    initStatusBar();
    slotSystemNetworkOnlineStateChanged(mNetworkConfigurationManager->isOnline());
}

StorageServiceManagerMainWindow::~StorageServiceManagerMainWindow()
{
    delete mStorageServiceMainWidget;
    KSharedConfig::Ptr config = KSharedConfig::openConfig();

    KConfigGroup group = config->group(QLatin1String("StorageServiceManagerMainWindow"));
    group.writeEntry("Size", size());
    qDebug() << " StorageServiceManagerMainWindow::~StorageServiceManagerMainWindow()";
    if (StorageServiceManagerGlobalConfig::self()->closeWallet()) {
        PimCommon::StorageServiceSettings::self()->closeWallet();
    }
    delete mNetworkConfigurationManager;
}

void StorageServiceManagerMainWindow::slotServicesChanged()
{
    mStorageServiceMainWidget->storageServiceTabWidget()->updateListService(mStorageManager->listService());
}

void StorageServiceManagerMainWindow::initStatusBar()
{
    mStatusBarInfo = new QLabel;
    statusBar()->insertWidget(0, mStatusBarInfo, 4);
    KPIM::ProgressStatusBarWidget *progressBar = new KPIM::ProgressStatusBarWidget(statusBar(), this, PimCommon::StorageServiceProgressManager::progressTypeValue());
    statusBar()->addPermanentWidget(progressBar->littleProgress(), 0);
}

void StorageServiceManagerMainWindow::slotSystemNetworkOnlineStateChanged(bool state)
{
    if (state) {
        mStorageServiceMainWidget->storageServiceTabWidget()->setNetworkIsDown(false);
        slotSetStatusBarMessage(i18n("Network connection is up."));
    } else {
        mStorageServiceMainWidget->storageServiceTabWidget()->setNetworkIsDown(true);
        slotSetStatusBarMessage(i18n("Network connection is down."));
    }
    slotUpdateActions();
}

void StorageServiceManagerMainWindow::slotUpdateActions()
{
    if (!mNetworkConfigurationManager->isOnline()) {
        mDownloadFile->setDisabled(true);
        mCreateFolder->setDisabled(true);
        mAccountInfo->setDisabled(true);
        mUploadFile->setDisabled(true);
        mDelete->setDisabled(true);
        mAuthenticate->setDisabled(true);
        mRefreshList->setDisabled(true);
        mShowLog->setDisabled(true);
        mRenameItem->setDisabled(true);
        mRefreshAll->setDisabled(true);
    } else {
        const PimCommon::StorageServiceAbstract::Capabilities capabilities = mStorageServiceMainWidget->storageServiceTabWidget()->capabilities();
        const bool listFolderWasLoaded = mStorageServiceMainWidget->storageServiceTabWidget()->listFolderWasLoaded();
        PimCommon::StorageServiceTreeWidget::ItemType type = mStorageServiceMainWidget->storageServiceTabWidget()->itemTypeSelected();
        mDownloadFile->setEnabled(listFolderWasLoaded && (capabilities & PimCommon::StorageServiceAbstract::DownloadFileCapability) && (type == PimCommon::StorageServiceTreeWidget::File));
        mCreateFolder->setEnabled(listFolderWasLoaded && (capabilities & PimCommon::StorageServiceAbstract::CreateFolderCapability));
        mAccountInfo->setEnabled(capabilities & PimCommon::StorageServiceAbstract::AccountInfoCapability);
        mUploadFile->setEnabled(capabilities & PimCommon::StorageServiceAbstract::UploadFileCapability);
        mDelete->setEnabled(listFolderWasLoaded && (capabilities & PimCommon::StorageServiceAbstract::DeleteFileCapability) &&
                            (type == PimCommon::StorageServiceTreeWidget::File || type == PimCommon::StorageServiceTreeWidget::Folder));
        mAuthenticate->setDisabled((capabilities & PimCommon::StorageServiceAbstract::NoCapability) || (mStorageServiceMainWidget->storageServiceTabWidget()->count() == 0));
        mRefreshList->setDisabled((capabilities & PimCommon::StorageServiceAbstract::NoCapability) || (mStorageServiceMainWidget->storageServiceTabWidget()->count() == 0));
        mRenameItem->setEnabled(listFolderWasLoaded && (capabilities & PimCommon::StorageServiceAbstract::RenameFileCapabilitity || capabilities & PimCommon::StorageServiceAbstract::RenameFolderCapability) &&
                                (type == PimCommon::StorageServiceTreeWidget::File || type == PimCommon::StorageServiceTreeWidget::Folder));

        mShowLog->setDisabled((mStorageServiceMainWidget->storageServiceTabWidget()->count() == 0));
        mRefreshAll->setDisabled((mStorageServiceMainWidget->storageServiceTabWidget()->count() == 0));
        mLogout->setEnabled(listFolderWasLoaded);
    }
}

void StorageServiceManagerMainWindow::setupActions()
{
    KActionCollection *ac = actionCollection();
    KStandardAction::quit(this, SLOT(close()), ac);

    mAuthenticate = ac->addAction(QLatin1String("authenticate"), mStorageServiceMainWidget->storageServiceTabWidget(), SLOT(slotAuthenticate()));
    mAuthenticate->setText(i18n("Authenticate..."));

    mCreateFolder = ac->addAction(QLatin1String("create_folder"), mStorageServiceMainWidget->storageServiceTabWidget(), SLOT(slotCreateFolder()));
    mCreateFolder->setText(i18n("Create Folder..."));
    mCreateFolder->setIcon(QIcon::fromTheme(QLatin1String("folder-new")));

    mRefreshList = ac->addAction(QLatin1String("refresh_list"), mStorageServiceMainWidget->storageServiceTabWidget(), SLOT(slotRefreshList()));
    mRefreshList->setText(i18n("Refresh List"));
    mRefreshList->setIcon(QIcon::fromTheme(QLatin1String("view-refresh")));
    ac->setDefaultShortcut(mRefreshList, QKeySequence(Qt::Key_F5));

    mAccountInfo = ac->addAction(QLatin1String("account_info"), mStorageServiceMainWidget->storageServiceTabWidget(), SLOT(slotAccountInfo()));
    mAccountInfo->setText(i18n("Account Info..."));

    mUploadFile = ac->addAction(QLatin1String("upload_file"), mStorageServiceMainWidget->storageServiceTabWidget(), SLOT(slotUploadFile()));
    mUploadFile->setText(i18n("Upload File..."));

    mDelete = ac->addAction(QLatin1String("delete"), mStorageServiceMainWidget->storageServiceTabWidget(), SLOT(slotDelete()));
    ac->setDefaultShortcut(mDelete, QKeySequence(Qt::Key_Delete));
    mDelete->setText(i18n("Delete..."));
    mDelete->setIcon(QIcon::fromTheme(QLatin1String("edit-delete")));

    mDownloadFile = ac->addAction(QLatin1String("download_file"), mStorageServiceMainWidget->storageServiceTabWidget(), SLOT(slotDownloadFile()));
    mDownloadFile->setText(i18n("Download File..."));
    mDownloadFile->setIcon(QIcon::fromTheme(QLatin1String("download")));

    mShowLog = ac->addAction(QLatin1String("show_log"), mStorageServiceMainWidget->storageServiceTabWidget(), SLOT(slotShowLog()));
    mShowLog->setText(i18n("Show Log..."));

    mLogout = ac->addAction(QLatin1String("logout"), this, SLOT(slotLogout()));
    mLogout->setText(i18n("Logout"));

    mShutdownAllServices = ac->addAction(QLatin1String("shutdown_all_services"), this, SLOT(slotShutdownAllServices()));
    mShutdownAllServices->setText(i18n("Shutdown All Services"));

    mRefreshAll = ac->addAction(QLatin1String("refresh_all"), this, SLOT(slotRefreshAll()));
    mRefreshAll->setText(i18n("Refresh All"));
    ac->setDefaultShortcut(mRefreshAll, QKeySequence(Qt::CTRL + Qt::Key_F5));

    mRenameItem = ac->addAction(QLatin1String("rename"), mStorageServiceMainWidget->storageServiceTabWidget(), SLOT(slotRename()));
    mRenameItem->setText(i18n("Rename..."));
    ac->setDefaultShortcut(mRenameItem, QKeySequence(Qt::Key_F2));

    KStandardAction::preferences(this, SLOT(slotConfigure()), ac);
    KStandardAction::configureNotifications(this, SLOT(slotShowNotificationOptions()), ac); // options_configure_notifications
}

void StorageServiceManagerMainWindow::slotShowNotificationOptions()
{
    KNotifyConfigWidget::configure(this);
}

void StorageServiceManagerMainWindow::slotLogout()
{
    mStorageServiceMainWidget->storageServiceTabWidget()->logout();
    slotUpdateActions();
}

void StorageServiceManagerMainWindow::closeEvent(QCloseEvent *e)
{
    if (mStorageServiceMainWidget->storageServiceTabWidget()->hasUploadDownloadProgress()) {
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
    connect(dlg.data(), &StorageServiceConfigureDialog::serviceRemoved, this, &StorageServiceManagerMainWindow::slotServiceRemoved);
    dlg->setListService(mStorageManager->listService());
    if (dlg->exec()) {
        mStorageManager->setListService(dlg->listService());
        mStorageServiceMainWidget->storageServiceTabWidget()->updateListService(dlg->listService());
        dlg->writeSettings();
    }
    delete dlg;
}

void StorageServiceManagerMainWindow::slotServiceRemoved(const QString &serviceName)
{
    mStorageServiceMainWidget->storageServiceTabWidget()->serviceRemoved(serviceName);
    mStorageManager->removeService(serviceName);
}

void StorageServiceManagerMainWindow::readConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group = KConfigGroup(config, "StorageServiceManagerMainWindow");
    const QSize sizeDialog = group.readEntry("Size", QSize(800, 600));
    if (sizeDialog.isValid()) {
        resize(sizeDialog);
    }
}

void StorageServiceManagerMainWindow::slotSetStatusBarMessage(const QString &message)
{
    mStatusBarInfo->setText(message);
}

void StorageServiceManagerMainWindow::slotShutdownAllServices()
{
    mStorageServiceMainWidget->storageServiceTabWidget()->shutdownAllServices();
    slotUpdateActions();
}

void StorageServiceManagerMainWindow::slotRefreshAll()
{
    mStorageServiceMainWidget->storageServiceTabWidget()->refreshAll();
    slotUpdateActions();
}
