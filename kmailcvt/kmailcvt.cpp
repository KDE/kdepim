/***************************************************************************
                          kmailcvt2.cpp  -  description
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

#include "filter_oe4.hxx"
#include "filter_oe5.hxx"
#include "filter_pmail.hxx"
#include "filter_plain.hxx"
#include "filter_pab.hxx"
#include "filter_eudora_ab.hxx"
#include "filter_ldif.hxx"

Kmailcvt2::Kmailcvt2(QWidget *parent, const char *name)
	: KWizard(parent, name, true) {

	_parent = parent;

	selfilterpage = new KSelFilterPage(this);
	addPage( selfilterpage, i18n( "Step 1: Select filter" ) );

	importpage = new KImportPage(this);
	addPage( importpage, i18n( "Step 2: Importing..." ) );
	setFinishEnabled(QWizard::page(2), false);

	selfilterpage->addFilter(new filter_oe5);
	selfilterpage->addFilter(new filter_oe4);
	selfilterpage->addFilter(new filter_pmail);
	selfilterpage->addFilter(new filter_plain);
	selfilterpage->addFilter(new filter_pab);
	selfilterpage->addFilter(new filter_ldif);
	selfilterpage->addFilter(new filter_eudora_ab);
}

Kmailcvt2::~Kmailcvt2() {
}

void Kmailcvt2::next() {
	if(currentPage()==selfilterpage){
		// Save selected filter
		filter *selectedFilter = selfilterpage->getSelectedFilter();
		// Goto next page
		QWizard::next();
		// Disable cancel & back
		QWizard::cancelButton()->setEnabled(false);
		setBackEnabled(QWizard::currentPage(), false);
		// Start import
		filterInfo *info = new filterInfo(importpage, _parent);
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

void Kmailcvt2::back() {
	QWizard::cancelButton()->setEnabled(true); // Re-enable cancel
	QWizard::back();
}

void Kmailcvt2::help()
{
	KAboutData aboutData( "KMailCVT", I18N_NOOP("KMAILCVT"),
		KMAILCVT_VERSION, KMAILCVT, 
		KAboutData::License_GPL_V2,
		"(c) 2000, Hans Dijkema");
	aboutData.addAuthor("Hans Dijkema","Original author", "kmailcvt@hum.org", "http://www.hum.org/kmailcvt.html");
	selfilterpage->setAuthors(aboutData);

	KAboutApplication a(&aboutData, _parent);
	a.exec();
}

#include "kmailcvt.moc"
