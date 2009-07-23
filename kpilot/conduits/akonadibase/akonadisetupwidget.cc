/* akonadisetupwidget.cc                       KPilot
**
** Copyright (C) 2008 Bertjan Broeksema <b.broeksema@kdemail.net>
**
** This file defines the widget and behavior for the config dialog
** of the Contacts conduit.
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

#include "akonadisetupwidget.h"

#include <akonadi/control.h>
#include <akonadi/collectionfetchjob.h>
#include <akonadi/collectionmodel.h>
#include <akonadi/collectionfilterproxymodel.h>

#include <akonadi/collectionview.h>
#include "options.h"

using namespace Akonadi;

class AkonadiSetupWidget::Private
{
public:
	Private() 
		: fCollectionFilterModel( 0L )
		, fCollections( 0L )
	{
	}
	
	Ui::AkonadiWidget fUi;
	Akonadi::CollectionFilterProxyModel* fCollectionFilterModel;
	CollectionView* fCollections;
	Entity::Id fConfiguredCollection;
	bool fCollectionModified;
};

AkonadiSetupWidget::AkonadiSetupWidget( QWidget* parent )
	: QWidget( parent ), d( new AkonadiSetupWidget::Private )
{
	FUNCTIONSETUP;
	
	d->fUi.setupUi( this );
	
	Akonadi::CollectionModel* collectionModel = new Akonadi::CollectionModel( this );
	
	d->fCollectionFilterModel = new Akonadi::CollectionFilterProxyModel();
	d->fCollectionFilterModel->setSourceModel( collectionModel );
	
	d->fCollections = new CollectionView( this );
	d->fCollections->setModel( d->fCollectionFilterModel );
	
	connect( d->fCollections, SIGNAL( currentChanged( const Akonadi::Collection& ) )
			,this , SLOT( changeCollection( const Akonadi::Collection& ) ) );
	
	d->fUi.fWarnIcon1->setPixmap(
		KIcon( QLatin1String( "dialog-warning" ) ).pixmap( 32 ) );
	d->fUi.fWarnIcon2->setPixmap( 
		KIcon( QLatin1String( "dialog-warning" ) ).pixmap( 32 ) );
  d->fUi.fErrorIcon->setPixmap(
    KIcon( QLatin1String( "dialog-error" ) ).pixmap( 32 ) );

	d->fUi.hboxLayout->addWidget( d->fCollections, 2 );

  d->fUi.fErrorIcon->setVisible(true);
  d->fUi.fNonExistingCollection->setVisible(true);

  Akonadi::Control::widgetNeedsAkonadi( this );
}

AkonadiSetupWidget::~AkonadiSetupWidget()
{
	KPILOT_DELETE( d );
}

void AkonadiSetupWidget::changeCollection( const Akonadi::Collection& col )
{
	FUNCTIONSETUP;
	DEBUGKPILOT << "collection id: "<< col.id() << ", name: " << col.name()
		    << ", resource: " << col.resource() << ", mimeType: " << col.mimeType();

	if( d->fConfiguredCollection != col.id() && col.id() >= 0 )
	{
		d->fCollectionModified = true;

		d->fUi.fErrorIcon->setVisible(false);
		d->fUi.fNonExistingCollection->setVisible(false);

		d->fUi.fWarnIcon1->setVisible( false );
		d->fUi.fSelectionWarnLabel->setVisible( false );
		
		emit collectionChanged();
	}
	selectedId = col.id();
}

Akonadi::Item::Id AkonadiSetupWidget::collection() const
{
	return selectedId;
}

bool AkonadiSetupWidget::modified() const
{
	return d->fCollectionModified;
}

void AkonadiSetupWidget::setCollection( Akonadi::Item::Id id )
{
	FUNCTIONSETUP;

	DEBUGKPILOT << "request to set collection to id: " << id;

	// This is a bit ugly but currently I see no other way how to fix this. We
	// assume here that if the fetch job fails, the collection does not exist
	// (anymore). The user probably has deleted the collection and created new
	// ones, maybe even a new one for the same file. However in the latter case
	// the resource gets a new ID so even though it is a resource for the same
	// file the user has to update the configuration of KPilot.
	// The CollectionModel loads asynchronous so checking if the collection model
	// contains the id might fail even though the collection still exists.
	// Therefore I use a synchronous fetchjob here to work around that problem and
	// be a bit more sure that the collection actually does or does not exists.
	CollectionFetchJob *job = new CollectionFetchJob( Collection( id ), CollectionFetchJob::Base );
	if ( !job->exec() ) {
		DEBUGKPILOT << "The collection does not exist." << id;
		// Make clear to the user that the configured collection does not exist
		// anymore.
		d->fUi.fErrorIcon->setVisible(true);
		d->fUi.fNonExistingCollection->setVisible(true);
	}
	else
	{
		d->fConfiguredCollection = id;
		d->fUi.fErrorIcon->setVisible(false);
		d->fUi.fNonExistingCollection->setVisible(false);

		d->fUi.fWarnIcon1->setVisible( false );
		d->fUi.fSelectionWarnLabel->setVisible( false );

		QModelIndexList result = d->fCollections->model()->match( d->fCollections->model()->index( 0, 0 ), CollectionModel::CollectionIdRole, id );
  		selectedId = id;
  		if( !result.isEmpty() ) {
    			d->fCollections->setCurrentIndex( result.first());
  		} else {
    			DEBUGKPILOT << "invalid id requested.";
  		}
	}
}

void AkonadiSetupWidget::setCollectionLabel( const QString& label )
{
	d->fUi.fCollectionsLabel->setText( label );
}

void AkonadiSetupWidget::setMimeTypes( const QStringList& mimeTypes )
{
	d->fCollectionFilterModel->clearFilters();
	d->fCollectionFilterModel->addMimeTypeFilters( mimeTypes );
}

