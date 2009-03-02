/* contactsconfig.cc			KPilot
**
** Copyright (C) 2008 by Bertjan Broeksema <b.broeksema@kdemail.net>
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "contactsconfig.h"

#include <kaboutdata.h>

#include "akonadisetupwidget.h"
#include "options.h"
#include "contactssettings.h"

static KAboutData *createAbout()
{
	KAboutData *fAbout = new KAboutData( "To-do conduit", 0
		, ki18n( "To-do Conduit for KPilot" )
		, KPILOT_VERSION
		, ki18n( "Configures the To-do Conduit for KPilot" )
		, KAboutData::License_GPL
		, ki18n( "(C) 2008, Bertjan Broeksema" )
	);
	
	fAbout->addAuthor( ki18n( "Bertjan Broeksema" )
		, ki18n( "Rewrite and port to Akonadi and Base libraries" )
		, "b.broeksema@kdemail.net"
		, "http://bertjan.broeksemaatjes.nl"
	);
	
	fAbout->addCredit( ki18n( "Dan Pilone" ), ki18n( "Original author of the old conduit" ) );
	fAbout->addCredit( ki18n( "Preston Brown" ), ki18n( "Original author of the old conduit" ) );
	fAbout->addCredit( ki18n( "Herwin-Jan Steenhouwer" ), ki18n( "Original author of the old conduit" ) );
	fAbout->addCredit( ki18n( "Adriaan de Groot" ), ki18n( "Maintainer" ) );
	fAbout->addCredit( ki18n( "Reinhold Kainhofer" ), ki18n( "Maintainer" ) );
	
	return fAbout;
}

ContactsConfig::ContactsConfig( QWidget* w, const QVariantList& ) : ConduitConfigBase( w )
{
	FUNCTIONSETUP;

	fConduitName = i18n( "Contacts" );
	fWidget = new QWidget( w );
	fUi.setupUi( fWidget );
	
	// Set up the akonadi tab.
	QWidget* akonadiTab = fUi.fTabWidget->widget( 0 );
	QStringList mimeTypes;
	mimeTypes << "text/directory" << "text/x-vcard" << "text/vcard";
	
	fAkonadiWidget = new AkonadiSetupWidget( akonadiTab );
	fAkonadiWidget->setCollectionLabel( i18n( "Select Addressbook: " ) );
	fAkonadiWidget->setMimeTypes( mimeTypes );
	
	fLayout = new QGridLayout( akonadiTab );
	fLayout->addWidget( fAkonadiWidget );
	
	connect( fAkonadiWidget, SIGNAL( collectionChanged() ), SLOT( modified() ) );
	
	// Add the about page.
	addAboutPage( fUi.fTabWidget, createAbout() );
}

ContactsConfig::~ContactsConfig()
{
}

void ContactsConfig::load()
{
	ContactsSettings::self()->readConfig();
	fAkonadiWidget->setCollection( ContactsSettings::akonadiCollection() );
	
	// General page
	//fConfigWidget->fArchive->setChecked(AbbrowserSettings::archiveDeleted());

	// Conflicts page
	//fUi.fConflictResolution->setCurrentItem(
	//  ContactsSettings::conflictResolution() - SyncAction::eCROffset );

	// Fields page
	fUi.fOtherPhone->setCurrentIndex( ContactsSettings::pilotOther() );
	fUi.fAddress->setCurrentIndex( ContactsSettings::pilotStreet() );
	fUi.fFax->setCurrentIndex( ContactsSettings::pilotFax() );

	// Custom fields page
	fUi.fCustom0->setCurrentIndex( ContactsSettings::custom0() );
	fUi.fCustom1->setCurrentIndex( ContactsSettings::custom1() );
	fUi.fCustom2->setCurrentIndex( ContactsSettings::custom2() );
	fUi.fCustom3->setCurrentIndex( ContactsSettings::custom3() );
	QString datefmt = ContactsSettings::customDateFormat();
	
	if( datefmt.isEmpty() )
	{
		fUi.fCustomDate->setCurrentIndex( 0 );
	}
	else
	{
		QAbstractItemModel* model = fUi.fCustomDate->model();
		QModelIndexList i = model->match( model->index( 0, 0 ), Qt::DisplayRole
			, QVariant( datefmt ), Qt::MatchCaseSensitive );
		fUi.fCustomDate->setCurrentIndex( i.first().row() );
	}
	
	unmodified();
}

void ContactsConfig::commit()
{
	// Akonadi page.
	// *Only* save the collection if the user selected another one.
	if( fAkonadiWidget->modified() )
	{
		ContactsSettings::setAkonadiCollection( fAkonadiWidget->collection() );
	}
	
	ContactsSettings::self()->writeConfig();
	unmodified();
}
