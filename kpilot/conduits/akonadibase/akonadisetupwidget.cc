/* akonadisetupwidget.cc                       KPilot
**
** Copyright (C) 2008 Bertjan Broeksema
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

#include <akonadi/collectionmodel.h>
#include <akonadi/collectionfilterproxymodel.h>

#include "collectioncombobox.h"
#include "options.h"

class AkonadiSetupWidget::Private
{
public:
	Private() : fCollectionFilterModel( 0L ), fCollections( 0L )
	{
	}
	
	Ui::AkonadiWidget fUi;
	Akonadi::CollectionFilterProxyModel* fCollectionFilterModel;
	QLabel* fCollectionsLabel;
	CollectionComboBox* fCollections;
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
	
	d->fCollectionsLabel = new QLabel( this );
	
	d->fCollections = new CollectionComboBox( this );
	d->fCollections->setModel( d->fCollectionFilterModel );
	
	connect( d->fCollections, SIGNAL( selectionChanged( const Akonadi::Collection& ) )
			,this , SLOT( changeCollection( const Akonadi::Collection& ) ) );
	
	d->fUi.fWarnIcon1->setPixmap(
		KIcon( QLatin1String( "dialog-warning" ) ).pixmap( 32 ) );
	d->fUi.fWarnIcon2->setPixmap( 
		KIcon( QLatin1String( "dialog-warning" ) ).pixmap( 32 ) );
	
	d->fUi.hboxLayout->addWidget( d->fCollectionsLabel, 1 );
	d->fUi.hboxLayout->addWidget( d->fCollections, 2 );
}

AkonadiSetupWidget::~AkonadiSetupWidget()
{
	KPILOT_DELETE( d );
}

void AkonadiSetupWidget::changeCollection( const Akonadi::Collection& col )
{
	FUNCTIONSETUP;
	
	if( col.id() >= 0 )
	{
		d->fCollectionModified = true;
		d->fUi.fWarnIcon1->setVisible( false );
		d->fUi.fSelectionWarnLabel->setVisible( false );
		
		emit collectionChanged();
	}
}

Akonadi::Item::Id AkonadiSetupWidget::collection() const
{
	return d->fCollections->selectedCollection().id();
}

bool AkonadiSetupWidget::modified() const
{
	return d->fCollectionModified;
}

void AkonadiSetupWidget::setCollection( Akonadi::Item::Id id )
{
	FUNCTIONSETUP;
	
	if( id >= 0 )
	{
		d->fUi.fWarnIcon1->setVisible( false );
		d->fUi.fSelectionWarnLabel->setVisible( false );
		d->fCollections->setSelectedCollection( id );
	}
}

void AkonadiSetupWidget::setCollectionLabel( const QString& label )
{
	d->fCollectionsLabel->setText( label );
}

void AkonadiSetupWidget::setMimeTypes( const QStringList& mimeTypes )
{
	d->fCollectionFilterModel->clearFilters();
	foreach( const QString& mimeType, mimeTypes )
	{
		d->fCollectionFilterModel->addMimeTypeFilter( mimeType );
	}
}

