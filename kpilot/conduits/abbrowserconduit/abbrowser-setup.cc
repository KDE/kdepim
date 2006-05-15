/* KPilot
**
** Copyright (C) 2001 by Dan Pilone
** Copyright (C) 2002-2003 Reinhold Kainhofer
**
** This file defines the setup dialog for the abbrowser-conduit plugin.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qbuttongroup.h>
#include <kurlrequester.h>

#include "kaddressbookConduit.h"
#include "abbrowser-factory.h"
#include "abbrowser-setup.h"
#include "abbrowserSettings.h"

#include "uiDialog.h"

AbbrowserWidgetSetup::AbbrowserWidgetSetup(QWidget *w, const char *n) :
	ConduitConfigBase(w,n),
	fConfigWidget(new AbbrowserWidget(w))
{
	FUNCTIONSETUP;

	fConduitName=i18n("Addressbook");
	UIDialog::addAboutPage(fConfigWidget->tabWidget,AbbrowserConduitFactory::about());
	fWidget=fConfigWidget;
	fConfigWidget->fAbookFile->setMode(KFile::File);
#define CM(a,b) connect(fConfigWidget->a,b,this,SLOT(modified()));
	CM(fSyncDestination,SIGNAL(clicked(int)));
	CM(fAbookFile,SIGNAL(textChanged(const QString &)));
	CM(fArchive,SIGNAL(toggled(bool)));
	CM(fConflictResolution,SIGNAL(activated(int)));
	CM(fOtherPhone,SIGNAL(activated(int)));
	CM(fAddress,SIGNAL(activated(int)));
	CM(fFax,SIGNAL(activated(int)));
	CM(fCustom0,SIGNAL(activated(int)));
	CM(fCustom1,SIGNAL(activated(int)));
	CM(fCustom2,SIGNAL(activated(int)));
	CM(fCustom3,SIGNAL(activated(int)));
	CM(fCustomDate, SIGNAL(activated(int)));
	CM(fCustomDate, SIGNAL(textChanged(const QString&)));
#undef CM
}

AbbrowserWidgetSetup::~AbbrowserWidgetSetup()
{
	FUNCTIONSETUP;
}

/* virtual */ void AbbrowserWidgetSetup::commit()
{
	FUNCTIONSETUP;
	
	QButtonGroup*grp=fConfigWidget->fSyncDestination;
	AbbrowserSettings::setAddressbookType(grp->id(grp->selected()));
	AbbrowserSettings::setFileName(fConfigWidget->fAbookFile->url());
	AbbrowserSettings::setArchiveDeleted(fConfigWidget->fArchive->isChecked());

	// Conflicts page
	AbbrowserSettings::setConflictResolution(
		fConfigWidget->fConflictResolution->currentItem()+SyncAction::eCROffset);

	// Fields page
	AbbrowserSettings::setPilotOther(fConfigWidget->fOtherPhone->currentItem());
	AbbrowserSettings::setPilotStreet(fConfigWidget->fAddress->currentItem());
	AbbrowserSettings::setPilotFax(fConfigWidget->fFax->currentItem());

	// Custom fields page
	AbbrowserSettings::setCustom0(fConfigWidget->fCustom0->currentItem());
	AbbrowserSettings::setCustom1(fConfigWidget->fCustom1->currentItem());
	AbbrowserSettings::setCustom2(fConfigWidget->fCustom2->currentItem());
	AbbrowserSettings::setCustom3(fConfigWidget->fCustom3->currentItem());
#ifdef DEBUG
	DEBUGCONDUIT <<fname<<
	  "Custom0: "<<fConfigWidget->fCustom0->currentItem()<<"   "<<
	  "Custom1: "<<fConfigWidget->fCustom1->currentItem()<<"   "<<
	  "Custom2: "<<fConfigWidget->fCustom2->currentItem()<<"   "<<
	  "Custom3: "<<fConfigWidget->fCustom3->currentItem()<<"   "
		<< " eCustom[0]=" << AbbrowserSettings::custom0()<<"    "
		<< " eCustom[1]=" << AbbrowserSettings::custom1()<<"    "
		<< " eCustom[2]=" << AbbrowserSettings::custom2()<<"    "
		<< " eCustom[3]=" << AbbrowserSettings::custom3()<<"    "<<
		endl;
#endif
	int fmtindex=fConfigWidget->fCustomDate->currentItem();
	AbbrowserSettings::setCustomDateFormat(
	  (fmtindex==0)?(QString::null):fConfigWidget->fCustomDate->currentText() );

	AbbrowserSettings::self()->writeConfig();
	unmodified();
}

/* virtual */ void AbbrowserWidgetSetup::load()
{
	FUNCTIONSETUP;
	AbbrowserSettings::self()->readConfig();

#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": Settings "
		<< " fPilotStreetHome=" << AbbrowserSettings::pilotStreet()
		<< " fPilotFaxHome=" << AbbrowserSettings::pilotFax()
		<< " fArchive=" << AbbrowserSettings::archiveDeleted()
		<< " eCustom[0]=" << AbbrowserSettings::custom0()
		<< " eCustom[1]=" << AbbrowserSettings::custom1()
		<< " eCustom[2]=" << AbbrowserSettings::custom2()
		<< " eCustom[3]=" << AbbrowserSettings::custom3()
		<< endl;
#endif
	
	// General page
	fConfigWidget->fSyncDestination->setButton(AbbrowserSettings::addressbookType());
	fConfigWidget->fAbookFile->setURL(AbbrowserSettings::fileName());
	fConfigWidget->fArchive->setChecked(AbbrowserSettings::archiveDeleted());

	// Conflicts page
	fConfigWidget->fConflictResolution->setCurrentItem(
	  AbbrowserSettings::conflictResolution() - SyncAction::eCROffset );

	// Fields page
	fConfigWidget->fOtherPhone->setCurrentItem(AbbrowserSettings::pilotOther());
	fConfigWidget->fAddress->setCurrentItem(AbbrowserSettings::pilotStreet());
	fConfigWidget->fFax->setCurrentItem(AbbrowserSettings::pilotFax());

	// Custom fields page
	fConfigWidget->fCustom0->setCurrentItem(AbbrowserSettings::custom0());
	fConfigWidget->fCustom1->setCurrentItem(AbbrowserSettings::custom1());
	fConfigWidget->fCustom2->setCurrentItem(AbbrowserSettings::custom2());
	fConfigWidget->fCustom3->setCurrentItem(AbbrowserSettings::custom3());
	QString datefmt=AbbrowserSettings::customDateFormat();
	if (datefmt.isEmpty())
	{
		fConfigWidget->fCustomDate->setCurrentItem(0);
	}
	else
	{
		fConfigWidget->fCustomDate->setCurrentText(datefmt);
	}

	unmodified();
}

/* static */ ConduitConfigBase *AbbrowserWidgetSetup::create(QWidget *w, const char *n)
{
	return new AbbrowserWidgetSetup(w,n);
}

