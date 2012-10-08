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
#include <mailcommon/mailkernel.h>


// KDE includes
#include <kaboutapplicationdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

// Qt includes
#include <QPushButton>

#include <akonadi/control.h>
using namespace MailImporter;

KMailCVT::KMailCVT(QWidget *parent)
  : KAssistantDialog(parent) {
  setModal(true);
  setWindowTitle( i18n( "KMailCVT Import Tool" ) );
  KGlobal::locale()->insertCatalog( "libmailimporter" );
  KGlobal::locale()->insertCatalog( "libmailcommon" );
  KMailCVTKernel *kernel = new KMailCVTKernel( this );
  CommonKernel->registerKernelIf( kernel ); //register KernelIf early, it is used by the Filter classes
  CommonKernel->registerSettingsIf( kernel ); //SettingsIf is used in FolderTreeWidget


  selfilterpage = new KSelFilterPage;
  page1 = new KPageWidgetItem( selfilterpage, i18n( "Step 1: Select Filter" ) );

  addPage( page1);

  importpage = new KImportPage;
  page2 = new KPageWidgetItem( importpage, i18n( "Step 2: Importing..." ) );
  addPage( page2 );
  connect(this,SIGNAL(helpClicked()),this,SLOT(help()));

  // Disable the 'next button to begin with.
  setValid( currentPage(), false );

  connect( selfilterpage->mWidget->mCollectionRequestor, SIGNAL(folderChanged(Akonadi::Collection)),
           this, SLOT(collectionChanged(Akonadi::Collection)) );
  Akonadi::Control::widgetNeedsAkonadi(this);
  readConfig();
}

KMailCVT::~KMailCVT()
{
  writeConfig();
}

void KMailCVT::readConfig()
{
  KConfigGroup group( KGlobal::config(), "FolderSelectionDialog" );
  if ( group.hasKey( "LastSelectedFolder" ) ) {
     selfilterpage->mWidget->mCollectionRequestor->setCollection( CommonKernel->collectionFromId(group.readEntry("LastSelectedFolder", -1 )));
  }
}

void KMailCVT::writeConfig()
{
  KConfigGroup group( KGlobal::config(), "FolderSelectionDialog" );
  group.writeEntry( "LastSelectedFolder", selfilterpage->mWidget->mCollectionRequestor->collection().id() );
  group.sync();
}

void KMailCVT::next()
{
  if( currentPage() == page1 ){
    // Save selected filter
    Filter *selectedFilter = selfilterpage->getSelectedFilter();
    Akonadi::Collection selectedCollection = selfilterpage->mWidget->mCollectionRequestor->collection();
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
    enableButton(KDialog::User3,false);
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
    enableButton(KDialog::User3,true);

  }
  else
    KAssistantDialog::next();
}

void KMailCVT::reject() {
  if ( currentPage() == page2 )
    FilterInfo::terminateASAP(); // ie. import in progress
  KAssistantDialog::reject();
}

void KMailCVT::collectionChanged( const Akonadi::Collection& selectedCollection )
{
 if( selectedCollection.isValid() ){
   setValid( currentPage(), true );
 } else {
   setValid( currentPage(), false );
 }
}

void KMailCVT::help()
{
  KAboutApplicationDialog a( KGlobal::mainComponent().aboutData(), this );
  a.exec();
}

#include "kmailcvt.moc"

