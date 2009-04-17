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
#include <kaboutapplicationdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <QPushButton>
#include <kdebug.h>
#include <ktoolinvocation.h>
#include <kmailinterface.h>
#include "kimportpage.h"
#include "kselfilterpage.h"
#include "filters.hxx"

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
}

KMailCVT::~KMailCVT() {
  endImport();
}

void KMailCVT::endImport()
{
    QDBusConnectionInterface * sessionBus = 0;
    sessionBus = QDBusConnection::sessionBus().interface();
    if ( sessionBus && !sessionBus->isServiceRegistered( "org.kde.kmail" ) )
       KToolInvocation::startServiceByDesktopName( "kmail", QString() ); // Will wait until kmail is started

    org::kde::kmail::kmail kmail("org.kde.kmail", "/KMail", QDBusConnection::sessionBus());
    QDBusReply<int> reply = kmail.dbusAddMessage(QString(), QString(),QString());
    if ( !reply.isValid() ) return;

    QDBusReply<void> reply2 = kmail.dbusResetAddMessage();
    if ( !reply2.isValid() ) return;
}

void KMailCVT::next() {
	if( currentPage() == page1 ){
		// Save selected filter
		Filter *selectedFilter = selfilterpage->getSelectedFilter();
		// without filter don't go next
		if (!selectedFilter)
			return;
		// Goto next page
		KAssistantDialog::next();
		// Disable back & finish
		setValid( currentPage(), false );
		// Start import
		FilterInfo *info = new FilterInfo(importpage, this, selfilterpage->removeDupMsg_checked());
		info->setStatusMsg(i18n("Import in progress"));
		info->clear(); // Clear info from last time
		selectedFilter->import(info);
		info->setStatusMsg(i18n("Import finished"));
		// Cleanup
		delete info;
		// Enable finish & back buttons
		setValid( currentPage(), true );
	} else KAssistantDialog::next();
}

void KMailCVT::reject() {
	if ( currentPage() == page2 )
          FilterInfo::terminateASAP(); // ie. import in progress
	KAssistantDialog::reject();
}

void KMailCVT::help()
{
	KAboutApplicationDialog a( KGlobal::mainComponent().aboutData(), this );
	a.exec();
}

#include "kmailcvt.moc"

