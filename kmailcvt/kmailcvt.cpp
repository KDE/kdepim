/***************************************************************************
                          kmailcvt.cpp  -  description
                             -------------------
    copyright            : (C) 2003 by Laurence Anderson
    email                : l.d.anderson@warwick.ac.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// Local includes
#include "kmailcvt.h"
#include "kimportpage.h"
#include "kselfilterpage.h"
#include <filters.h>
#include "kmailcvtfilterinfogui.h"
#include "kmailcvtkernel.h"
#include <mailcommon/kernel/mailkernel.h>


// KDE includes
#include <kaboutapplicationdialog.h>
#include <kglobal.h>
#include <KLocalizedString>
#include <qdebug.h>
#include <KHelpMenu>
#include <KConfigGroup>
#include <KAboutData>
// Qt includes
#include <QPushButton>

#include <AkonadiCore/control.h>
#include <KSharedConfig>
using namespace MailImporter;

KMailCVT::KMailCVT(QWidget *parent)
    : KAssistantDialog(parent) {
    setModal(true);
    setWindowTitle( i18n( "KMailCVT Import Tool" ) );
    KMailCVTKernel *kernel = new KMailCVTKernel( this );
    CommonKernel->registerKernelIf( kernel ); //register KernelIf early, it is used by the Filter classes
    CommonKernel->registerSettingsIf( kernel ); //SettingsIf is used in FolderTreeWidget
    setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Help);

    selfilterpage = new KSelFilterPage;
    page1 = new KPageWidgetItem( selfilterpage, i18n( "Step 1: Select Filter" ) );

    addPage( page1);

    importpage = new KImportPage;
    page2 = new KPageWidgetItem( importpage, i18n( "Step 2: Importing..." ) );
    addPage( page2 );

    // Disable the 'next button to begin with.
    setValid( currentPage(), false );

    connect( selfilterpage->widget()->mCollectionRequestor, SIGNAL(folderChanged(Akonadi::Collection)),
             this, SLOT(collectionChanged(Akonadi::Collection)) );
    Akonadi::Control::widgetNeedsAkonadi(this);
    readConfig();

    KHelpMenu *helpMenu = new KHelpMenu(this, KAboutData::applicationData(), true);
    //Initialize menu
    QMenu *menu = helpMenu->menu();
    helpMenu->action(KHelpMenu::menuAboutApp)->setIcon(QIcon::fromTheme(QLatin1String("kmail")));
    button(QDialogButtonBox::Help)->setMenu(menu);

#if 0 //QT5
    setHelp(QString(), QLatin1String("kmailcvt"));
#endif
}

KMailCVT::~KMailCVT()
{
    writeConfig();
}

void KMailCVT::readConfig()
{
    KConfigGroup group( KSharedConfig::openConfig(), "FolderSelectionDialog" );
    if ( group.hasKey( "LastSelectedFolder" ) ) {
        selfilterpage->widget()->mCollectionRequestor->setCollection( CommonKernel->collectionFromId(group.readEntry("LastSelectedFolder", -1 )));
    }
}

void KMailCVT::writeConfig()
{
    KConfigGroup group( KSharedConfig::openConfig(), "FolderSelectionDialog" );
    group.writeEntry( "LastSelectedFolder", selfilterpage->widget()->mCollectionRequestor->collection().id() );
    group.sync();
}

void KMailCVT::next()
{
    if( currentPage() == page1 ){
        // Save selected filter
        Filter *selectedFilter = selfilterpage->getSelectedFilter();
        Akonadi::Collection selectedCollection = selfilterpage->widget()->mCollectionRequestor->collection();
        // without filter don't go next
        if ( !selectedFilter )
            return;
        // Ensure we have a valid collection.
        if( !selectedCollection.isValid() )
            return;
        // Goto next page
        KAssistantDialog::next();
        // Disable back & finish
        setValid( currentPage(), false );
        backButton()->setEnabled(false);
        // Start import
        FilterInfo *info = new FilterInfo();
        KMailCvtFilterInfoGui *infoGui = new KMailCvtFilterInfoGui(importpage, this);
        info->setFilterInfoGui(infoGui);
        info->setStatusMessage(i18n("Import in progress"));
        info->setRemoveDupMessage( selfilterpage->removeDupMsg_checked() );
        info->clear(); // Clear info from last time
        info->setRootCollection( selectedCollection );
        selectedFilter->setFilterInfo( info );
        selectedFilter->import();
        info->setStatusMessage(i18n("Import finished"));
        // Cleanup
        delete info;
        // Enable finish & back buttons
        setValid( currentPage(), true );
        backButton()->setEnabled(true);

    }
    else
        KAssistantDialog::next();
}

void KMailCVT::reject() {
    if ( currentPage() == page2 )
        FilterInfo::terminateASAP(); // ie. import in progress
    KAssistantDialog::reject();
}

void KMailCVT::collectionChanged( const Akonadi::Collection &selectedCollection )
{
    if( selectedCollection.isValid() ){
        setValid( currentPage(), true );
    } else {
        setValid( currentPage(), false );
    }
}


