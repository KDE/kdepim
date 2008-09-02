/* KPilot
**
** Copyright (C) 2001 by Dan Pilone <pilone@slac.com>
** Copyright (C) 2002-2003 Reinhold Kainhofer <reinhold@kainhofer.com>
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "contacts-setup.h"

#include <kaboutdata.h>

#include "options.h"

#include "contacts-setup.moc"
#include "contactsSettings.h"



static KAboutData *createAbout()
{
	KAboutData *fAbout = new KAboutData( "Contacts conduit", 0
		, ki18n( "Contacts Conduit for KPilot" )
		, KPILOT_VERSION
		, ki18n( "Configures the Contacts Conduit for KPilot" )
		, KAboutData::License_GPL
		, ki18n( "(C) 2008, Bertjan Broeksema" )
	);
	
	fAbout->addAuthor( ki18n( "Bertjan Broeksema" )
		, ki18n( "Primary Author" )
		, "b.broeksema@kdemail.net"
		, "http://bertjan.broeksemaatjes.nl"
	);
	
	return fAbout;
}

ContactsWidgetSetup::ContactsWidgetSetup( QWidget *w, const QVariantList & ) :
	ConduitConfigBase( w )
{
	FUNCTIONSETUP;

	fWidget = new QWidget();
	fUi.setupUi( fWidget );
	
	fConduitName  =i18n("Contacts");
	
	fAbout = createAbout();
	ConduitConfigBase::addAboutPage( fUi.tabWidget, fAbout );

	/*
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
	*/
}

ContactsWidgetSetup::~ContactsWidgetSetup()
{
	FUNCTIONSETUP;
}

/* virtual */ void ContactsWidgetSetup::commit()
{
	FUNCTIONSETUP;

	/*
	Q3ButtonGroup*grp=fConfigWidget->fSyncDestination;
	AbbrowserSettings::setAddressbookType(grp->id(grp->selected()));
	AbbrowserSettings::setFileName(fConfigWidget->fAbookFile->url().url());
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
	int fmtindex=fConfigWidget->fCustomDate->currentItem();
	AbbrowserSettings::setCustomDateFormat(
	  (fmtindex==0)?(QString::null):fConfigWidget->fCustomDate->currentText() );	//krazy:exclude=nullstrassign for old broken gcc

	AbbrowserSettings::self()->writeConfig();
	unmodified();
	*/
}

/* virtual */ void ContactsWidgetSetup::load()
{
	FUNCTIONSETUP;
	/*
	AbbrowserSettings::self()->readConfig();

	// General page
	fConfigWidget->fSyncDestination->setButton(AbbrowserSettings::addressbookType());
	fConfigWidget->fAbookFile->setUrl(AbbrowserSettings::fileName());
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
	*/
	unmodified();
}

/* static */ ConduitConfigBase* ContactsWidgetSetup::create(QWidget *w)
{
	return new ContactsWidgetSetup(w);
}

