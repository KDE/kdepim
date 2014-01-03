/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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
#include "pimcommon/storageservice/storageservicemanager.h"

#include <KStandardAction>
#include <KLocalizedString>
#include <KActionCollection>
#include <KApplication>
#include <KConfigGroup>
#include <KAction>

#include <QPointer>

StorageServiceManagerMainWindow::StorageServiceManagerMainWindow()
    : KXmlGuiWindow()
{
    mStorageManager = new PimCommon::StorageServiceManager(this);
    mStorageServiceTabWidget = new StorageServiceTabWidget;
    setCentralWidget(mStorageServiceTabWidget);

    setupActions();
    setupGUI();
    readConfig();
}

StorageServiceManagerMainWindow::~StorageServiceManagerMainWindow()
{
    KSharedConfig::Ptr config = KGlobal::config();

    KConfigGroup group = config->group( QLatin1String("StorageServiceManagerMainWindow") );
    group.writeEntry( "Size", size() );
}

void StorageServiceManagerMainWindow::setupActions()
{
    KActionCollection *ac = actionCollection();
    KStandardAction::quit(this, SLOT(slotQuitApp()), ac );

    KAction *act = ac->addAction(QLatin1String("authenticate"), mStorageServiceTabWidget, SLOT(slotAuthenticate()));
    act->setText(i18n("Authenticate..."));

    act = ac->addAction(QLatin1String("create_folder"), mStorageServiceTabWidget, SLOT(slotCreateFolder()));
    act->setText(i18n("Create Folder..."));

    act = ac->addAction(QLatin1String("add_storage_service"), this, SLOT(slotAddStorageService()));
    act->setText(i18n("Add Storage Service..."));

    act = ac->addAction(QLatin1String("refresh_list"), mStorageServiceTabWidget, SLOT(slotRefreshList()));
    act->setText(i18n("Refresh List"));

    act = ac->addAction(QLatin1String("account_info"), mStorageServiceTabWidget, SLOT(slotAccountInfo()));
    act->setText(i18n("Account Info..."));

    act = ac->addAction(QLatin1String("upload_file"), mStorageServiceTabWidget, SLOT(slotUploadFile()));
    act->setText(i18n("Upload File..."));

    act = ac->addAction(QLatin1String("delete_file"), mStorageServiceTabWidget, SLOT(slotDeleteFile()));
    act->setText(i18n("Delete File..."));

    act = ac->addAction(QLatin1String("download_file"), mStorageServiceTabWidget, SLOT(slotDownloadFile()));
    act->setText(i18n("Download File..."));

    KStandardAction::preferences( this, SLOT(slotConfigure()), ac );

    //TODO
}

void StorageServiceManagerMainWindow::slotAddStorageService()
{

}

void StorageServiceManagerMainWindow::slotQuitApp()
{
    kapp->quit();
}

void StorageServiceManagerMainWindow::slotConfigure()
{
    QPointer<StorageServiceConfigureDialog> dlg = new StorageServiceConfigureDialog(this);
    dlg->setListService(mStorageManager->listService());
    if (dlg->exec()) {
        mStorageManager->setListService(dlg->listService());
    }
    //TODO update tabwidget
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

