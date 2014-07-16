/*
    Copyright (c) 2009 Stephen Kelly <steveire@gmail.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "akonotes_noteslistapplet.h"

#include <QGraphicsLinearLayout>

#include <KConfigDialog>

#include <akonadi/changerecorder.h>
#include <akonadi/collection.h>
#include <akonadi/collectionfilterproxymodel.h>
#include <akonadi/entitydisplayattribute.h>
#include <akonadi/entitytreemodel.h>
#include <akonadi/entityrightsfiltermodel.h>
#include <akonadi/itemmodel.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/session.h>

#include "akonadi_next/note.h"

#include "../kjotsmodel.h"
#include "plasmatreeview.h"

using namespace Akonadi;

K_EXPORT_PLASMA_APPLET( akonotes_list, AkonotesListApplet )

AkonotesListApplet::AkonotesListApplet(QObject* parent, const QVariantList& args)
    : PopupApplet(parent, args)
{

  setHasConfigurationInterface( true );
  setPopupIcon( QLatin1String("kjots") );
  m_treeView = new PlasmaTreeView;

  setGraphicsWidget( m_treeView );

  setBackgroundHints( DefaultBackground );
}

void AkonotesListApplet::setupModel( Collection::Id id )
{
  //     This can be used when it is possible to monitor a non-root collection
  ItemFetchScope scope;
  scope.fetchFullPayload( true ); // Need to have full item when adding it to the internal data structure
  scope.fetchAttribute< EntityDisplayAttribute >();

  ChangeRecorder *monitor = new ChangeRecorder( this );
  monitor->fetchCollection( true );
  monitor->setItemFetchScope( scope );

  monitor->setCollectionMonitored( Collection( id ) );
  monitor->setMimeTypeMonitored( Akonotes::Note::mimeType() );

  EntityTreeModel *model = new KJotsModel( monitor, this );
  model->setCollectionFetchStrategy( EntityTreeModel::FetchNoCollections );

  m_treeView->setModel( model );
}

void AkonotesListApplet::init()
{
  KConfigGroup cg = config();
  Collection::Id rootCollectionId = cg.readEntry("rootCollection", -1);

  if ( rootCollectionId < 0 )
  {
    setConfigurationRequired( true, i18n("This widget needs to be configured.") );
    return;
  }

  setupModel( rootCollectionId );
}


void AkonotesListApplet::createConfigurationInterface(KConfigDialog *configDialog)
{
  QWidget *widget = new QWidget();
  ui.setupUi(widget);
  configDialog->addPage(widget, i18n("Selected Collection"), QLatin1String("view-media-visualization"));

  ui.treeView->header()->hide();

  // TODO: Refactor into a collection chooser widget.
  Akonadi::ChangeRecorder *monitor = new Akonadi::ChangeRecorder( this );
  monitor->fetchCollection( true );
  monitor->setMimeTypeMonitored( Akonotes::Note::mimeType(), true );
  monitor->setCollectionMonitored( Akonadi::Collection::root() );

  EntityTreeModel *model = new EntityTreeModel( monitor, this );
  model->setItemPopulationStrategy( EntityTreeModel::NoItemPopulation );

  CollectionFilterProxyModel *mimeTypeFilterModel = new CollectionFilterProxyModel( this );
  mimeTypeFilterModel->addMimeTypeFilter( Akonotes::Note::mimeType() );
  mimeTypeFilterModel->setSourceModel( model );

  EntityRightsFilterModel *rightsFilterModel = new EntityRightsFilterModel( this );
  rightsFilterModel->setSourceModel( mimeTypeFilterModel );

  ui.treeView->setModel(rightsFilterModel);

  connect(configDialog, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
  connect(configDialog, SIGNAL(okClicked()), this, SLOT(configAccepted()));
}

void AkonotesListApplet::configAccepted()
{
  KConfigGroup cg = config();

  QModelIndexList rows = ui.treeView->selectionModel()->selectedRows();

  if ( rows.isEmpty() )
    return;

  Q_ASSERT( rows.size() == 1 );

  Collection col = rows.at( 0 ).data( EntityTreeModel::CollectionRole ).value<Collection>();

  cg.writeEntry( "rootCollection", col.id() );

  setupModel( col.id() );

  setConfigurationRequired( false );

  emit configNeedsSaving();
}




