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

#include "kmailcvt.h"
#include <kaboutapplication.h>
#include <qpushbutton.h>

#include "filters.hxx"

KMailCVT::KMailCVT(QWidget *parent, const char *name)
	: KWizard(parent, name, true) {

	setCaption( i18n( "KMailCVT Import Tool" ) );

	selfilterpage = new KSelFilterPage(this);
	addPage( selfilterpage, i18n( "Step 1: Select Filter" ) );

	importpage = new KImportPage(this);
	addPage( importpage, i18n( "Step 2: Importing..." ) );

}

KMailCVT::~KMailCVT() {
}

void KMailCVT::next() {
	if( currentPage() == selfilterpage ){
		// Save selected filter
		Filter *selectedFilter = selfilterpage->getSelectedFilter();
		// without filter don't go next
		if (!selectedFilter)
			return;
		// Goto next page
		KWizard::next();
		// Disable back & finish
		setBackEnabled( currentPage(), false );
		setFinishEnabled( currentPage(), false );
		// Start import
		FilterInfo *info = new FilterInfo(importpage, this, selfilterpage->removeDupMsg_checked());
		info->setStatusMsg(i18n("Import in progress"));
		info->clear(); // Clear info from last time
		selectedFilter->import(info);
		info->setStatusMsg(i18n("Import finished"));
		// Cleanup
		delete info;
		// Enable finish & back buttons
		setFinishEnabled( currentPage(), true );
		setBackEnabled( currentPage(), true );
	} else KWizard::next();
}

void KMailCVT::reject() {
	if ( currentPage() == importpage && ! finishButton()->isEnabled() ) FilterInfo::terminateASAP(); // ie. import in progress
	else KWizard::reject();
}

void KMailCVT::help()
{
	KAboutApplication a( this );
	a.exec();
}

#include "kmailcvt.moc"
