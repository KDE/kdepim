/***************************************************************************
                          kmailcvt.cpp  -  description
                             -------------------
    begin                : Wed Aug  2 11:23:04 CEST 2000
    copyright            : (C) 2000 by Hans Dijkema
    email                : kmailcvt@hum.org
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
#include <kaboutdialog.h>
#include <klocale.h>
#include <kapplication.h>
#include <qlayout.h>
#include <kaboutdata.h>
#include <kaboutapplication.h>
#include <qpushbutton.h>

//////////////////////////////////////////////////////////////////////////////
//
// Add filters here
//
//////////////////////////////////////////////////////////////////////////////

#include "filters.hxx"

KMailCVT::KMailCVT(QWidget *parent, const char *name)
	: KWizard(parent, name, true) {

	_parent = parent;

	selfilterpage = new KSelFilterPage(this);
	addPage( selfilterpage, i18n( "Step 1: Select filter" ) );

	importpage = new KImportPage(this);
	addPage( importpage, i18n( "Step 2: Importing..." ) );
	setFinishEnabled(QWizard::page(2), false);

    Filter::List filters = Filter::createFilters();
    for ( Filter* filter = filters.first(); filter; filter = filters.next() )
        selfilterpage->addFilter( filter );
}

KMailCVT::~KMailCVT() {
}

void KMailCVT::next() {
	if(currentPage()==selfilterpage){
		// Save selected filter
		Filter *selectedFilter = selfilterpage->getSelectedFilter();
		// Goto next page
		QWizard::next();
		// Disable cancel & back
		QWizard::cancelButton()->setEnabled(false);
		setBackEnabled(QWizard::currentPage(), false);
		// Start import
		FilterInfo *info = new FilterInfo(importpage, _parent);
		info->clear(); // Clear info from last time
		selectedFilter->import(info);
		// Cleanup
		delete info;
		// Enable finish button also reenable back
		setFinishEnabled(QWizard::currentPage(), true);
		setBackEnabled(QWizard::currentPage(), true);
		return;
	}
	QWizard::next();
}

void KMailCVT::back() {
	QWizard::cancelButton()->setEnabled(true); // Re-enable cancel
	QWizard::back();
}

void KMailCVT::help()
{
	KAboutData aboutData( "kmailcvt", I18N_NOOP("KMailCVT"),
		KMAILCVT_VERSION, KMAILCVT, 
		KAboutData::License_GPL_V2,
		"(c) 2000-3, The KMailCVT developers");
	aboutData.addAuthor("Hans Dijkema","Original author", "kmailcvt@hum.org", "http://www.hum.org/kmailcvt.html");
	aboutData.addAuthor("Laurence Anderson","New GUI & cleanups", "l.d.anderson@warwick.ac.uk");
	selfilterpage->setAuthors(aboutData);

	KAboutApplication a(&aboutData, _parent);
	a.exec();
}

#include "kmailcvt.moc"
