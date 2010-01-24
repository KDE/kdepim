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
#include "filters.hxx"

// Akonadi includes
#include <Akonadi/Control>

// KDE includes
#include <kaboutapplicationdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <ktoolinvocation.h>

// Qt includes
#include <QPushButton>
#include <QTimer>
#include <kmessagebox.h>



KMailCVT::KMailCVT(QWidget *parent)
	: KAssistantDialog(parent) {
        setModal(true);
	setWindowTitle( i18n( "KMailCVT Import Tool" ) );


	selfilterpage = new KSelFilterPage;
	page1 = new KPageWidgetItem( selfilterpage, i18n( "Step 1: Select Filter" ) );

	addPage( page1);

	importpage = new KImportPage;
	page2 = new KPageWidgetItem( importpage, i18n( "Step 2: Importing..." ) );
	addPage( page2 );
        connect(this,SIGNAL(helpClicked()),this,SLOT(help()));
	QTimer::singleShot( 0, this, SLOT( delayedStart()) );
}

KMailCVT::~KMailCVT()
{
}

void KMailCVT::next()
{
  if( currentPage() == page1 ){
    // Save selected filter
    Filter *selectedFilter = selfilterpage->getSelectedFilter();
    Akonadi::Collection selectedCollection = selfilterpage->mCollectionRequestor->collection();
    // without filter don't go next
    if ( !selectedFilter )
      return;
    // Ensure we have a valid collection.
    if( !selectedCollection.isValid() )
      return;
    if ( !selectedFilter->needsSecondPage() ) {
      FilterInfo *info = new FilterInfo( importpage, this, selfilterpage->removeDupMsg_checked() );
      info->setRootCollection( selectedCollection );
      selectedFilter->import( info );
      accept();
      delete info;
    }
    else {
      // Goto next page
      KAssistantDialog::next();
      // Disable back & finish
      setValid( currentPage(), false );
      // Start import
      FilterInfo *info = new FilterInfo(importpage, this, selfilterpage->removeDupMsg_checked());
      info->setStatusMsg(i18n("Import in progress"));
      info->clear(); // Clear info from last time
      info->setRootCollection( selectedCollection );
      selectedFilter->import(info);
      accept();
      info->setStatusMsg(i18n("Import finished"));
      // Cleanup
      delete info;
      // Enable finish & back buttons
      setValid( currentPage(), true );
    }
  } else KAssistantDialog::next();
}

void KMailCVT::reject() {
	if ( currentPage() == page2 )
          FilterInfo::terminateASAP(); // ie. import in progress
	KAssistantDialog::reject();
}

void KMailCVT::delayedStart()
{
  if( !Akonadi::Control::start( this ) ) {
    KMessageBox::sorry( 0, i18n( "Akonadi failed to start. Please check your configuration." ),
			i18n( "KMailCVT" ) );
    qApp->exit( -1 );
    return;
  }
}


void KMailCVT::help()
{
	KAboutApplicationDialog a( KGlobal::mainComponent().aboutData(), this );
	a.exec();
}

#include "kmailcvt.moc"

