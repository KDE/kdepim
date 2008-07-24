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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "contacts-setup.h"

#include <akonadi/collectionfilterproxymodel.h>
#include <akonadi/collectionmodel.h>

#include <kiconloader.h>
#include <kaboutdata.h>

#include "options.h"

#include "contacts-setup.moc"
#include "contactsSettings.h"
#include "collectioncombobox.h"

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
	ConduitConfigBase( w ), fCollectionModified( false )
{
	FUNCTIONSETUP;

	fWidget = new QWidget();
	fUi.setupUi( fWidget );
	
	setupAkonadiTab();
	
	fConduitName = i18n("Contacts");
	
	fAbout = createAbout();
	ConduitConfigBase::addAboutPage( fUi.tabWidget, fAbout );

	// Do not connect this just to modified(). It is very important that the
	// collection doesn't get just saved.
	connect( fCollections, SIGNAL( selectionChanged( const Akonadi::Collection& ) )
		,this , SLOT( collectionModified() ) );

#define CM( a, b ) connect( fUi.a, b, this, SLOT( modified() ) );
	CM( fConflictResolution, SIGNAL( activated( int ) ) );
	CM( fOtherPhone, SIGNAL( activated( int ) ) );
	CM( fAddress, SIGNAL( activated( int ) ) );
	CM( fFax, SIGNAL( activated( int ) ) );
	CM( fCustom0, SIGNAL( activated( int ) ) );
	CM( fCustom1, SIGNAL( activated( int ) ) );
	CM( fCustom2, SIGNAL( activated( int ) ) );
	CM( fCustom3, SIGNAL( activated( int ) ) );
	CM( fCustomDate, SIGNAL( activated( int ) ) );
	CM( fCustomDate, SIGNAL( textChanged( const QString& ) ) );
#undef CM
}

ContactsWidgetSetup::~ContactsWidgetSetup()
{
	FUNCTIONSETUP;
}

/* virtual */ void ContactsWidgetSetup::commit()
{
	FUNCTIONSETUP;

	// Akonadi page.
	// *Only* save the collection if the user selected another one.
	if( fCollectionModified )
	{
		ContactsSettings::setAkonadiCollection( fCollections->selectedCollection().id() );
	}

	// Conflicts page
	ContactsSettings::setConflictResolution(
		fUi.fConflictResolution->currentIndex() + SyncAction::eCROffset );

	// Fields page
	ContactsSettings::setPilotOther( fUi.fOtherPhone->currentIndex() );
	ContactsSettings::setPilotStreet( fUi.fAddress->currentIndex() );
	ContactsSettings::setPilotFax( fUi.fFax->currentIndex() );

	// Custom fields page
	ContactsSettings::setCustom0( fUi.fCustom0->currentIndex() );
	ContactsSettings::setCustom1( fUi.fCustom1->currentIndex() );
	ContactsSettings::setCustom2( fUi.fCustom2->currentIndex() );
	ContactsSettings::setCustom3( fUi.fCustom3->currentIndex() );
	int fmtindex = fUi.fCustomDate->currentIndex();
	ContactsSettings::setCustomDateFormat(
	  (fmtindex == 0) ? QString() : fUi.fCustomDate->currentText() );

	ContactsSettings::self()->writeConfig();
	unmodified();
}

/* virtual */ void ContactsWidgetSetup::load()
{
	FUNCTIONSETUP;
	
	ContactsSettings::self()->readConfig();
	
	if( ContactsSettings::akonadiCollection() != -1 )
	{
		fCollections->setSelectedCollection( ContactsSettings::akonadiCollection() );
	}
	else
	{
		fUi.fWarnIcon1->setVisible( true );
		fUi.fSelectionWarnLabel->setVisible( true );
	}
	
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

void ContactsWidgetSetup::collectionModified()
{
	FUNCTIONSETUP;
	
	fCollectionModified = true;
	
	fUi.fWarnIcon1->setVisible( false );
	fUi.fSelectionWarnLabel->setVisible( false );
	
	modified();
}

void ContactsWidgetSetup::setupAkonadiTab()
{
	FUNCTIONSETUP;
	
	fCollectionModel = new Akonadi::CollectionModel( this );
	
	fCollectionFilterModel = new Akonadi::CollectionFilterProxyModel();
	fCollectionFilterModel->addMimeTypeFilter( "text/x-vcard" );
	fCollectionFilterModel->addMimeTypeFilter( "text/directory" );
	fCollectionFilterModel->addMimeTypeFilter( "text/vcard" );
	fCollectionFilterModel->setSourceModel( fCollectionModel );
	
	fCollectionsLabel = new QLabel( fUi.akonadiTab );
	fCollectionsLabel->setText( "Akonadi addresbook collection:" );
	fCollections = new CollectionComboBox( fUi.akonadiTab );
	fCollections->setModel( fCollectionFilterModel );
	
	fUi.fWarnIcon1->setVisible( false );
	fUi.fSelectionWarnLabel->setVisible( false );
	
	
	fUi.fWarnIcon1->setPixmap( 
		KIcon( QLatin1String( "dialog-warning" ) ).pixmap( 32 ) );
	fUi.fWarnIcon2->setPixmap( 
		KIcon( QLatin1String( "dialog-warning" ) ).pixmap( 32 ) );
	
	fUi.gridLayout2->addWidget( fCollections );
	fUi.hboxLayout->addWidget( fCollectionsLabel, 1 );
	fUi.hboxLayout->addWidget( fCollections, 2 );
}

/* static */ ConduitConfigBase* ContactsWidgetSetup::create(QWidget *w)
{
	return new ContactsWidgetSetup(w);
}

