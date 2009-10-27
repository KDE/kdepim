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

#include "akonotes_applet.h"

#include <plasma/svg.h>
#include <plasma/widgets/textedit.h>
#include <plasma/extenderitem.h>
#include <plasma/extender.h>

#include <QtGui/QPainter>

#include <kdescendantsproxymodel.h>

#include <akonadi/changerecorder.h>
#include <akonadi/collectionfilterproxymodel.h>
#include <akonadi/entitydisplayattribute.h>
#include <akonadi/entitymimetypefiltermodel.h>
#include <akonadi/entityrightsfiltermodel.h>
#include <akonadi/entitytreemodel.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/itemmodel.h>
#include <akonadi/session.h>

#include <KMime/KMimeMessage>
#include <KConfigDialog>

#include <kdebug.h>

using namespace Akonadi;

// This is the command that links your applet to the .desktop file
K_EXPORT_PLASMA_APPLET( akonotes, AkonotesMasterApplet )

AkonotesMasterApplet::AkonotesMasterApplet( QObject *parent, const QVariantList &args )
    : Plasma::PopupApplet( parent, args ), m_svg(this), m_model( 0 )
{
  setPopupIcon( QLatin1String( "kjots" ) );

  m_svg.setImagePath( QLatin1String( "/home/kde-devel/kde/src/akonadi-ports/kdepim/kjots/plasmoid/background.svg" ) );
  // this will get us the standard applet background, for free!
//   setBackgroundHints(DefaultBackground);
  resize(200, 200);

  setHasConfigurationInterface(true);
}


AkonotesMasterApplet::~AkonotesMasterApplet()
{
  if ( hasFailedToLaunch() )
  {
    // Nothing to do yet
  }
}

void AkonotesMasterApplet::init()
{
  KConfigGroup cg = config();
  m_rootCollectionId = cg.readEntry("rootCollection", -1);

//   m_rootCollectionId = 323;

  if (m_rootCollectionId < 0)
  {
    setConfigurationRequired( true, "This widget needs to be configured." );
    return;
  }


//   This can be used when it is possible to monitor a non-root collection
//   ItemFetchScope scope;
//   scope.fetchFullPayload( true ); // Need to have full item when adding it to the internal data structure
//   scope.fetchAttribute< EntityDisplayAttribute >();
//
//   ChangeRecorder *monitor = new ChangeRecorder( this );
//   monitor->fetchCollection( true );
//   monitor->setItemFetchScope( scope );
//   // TODO: Make setting a non-root collection work here.
//   monitor->setCollectionMonitored( Collection::root() );
//   monitor->setMimeTypeMonitored( QLatin1String( "text/x-vnd.akonadi.note" ) );
//
//   Session *session = new Session( QByteArray( "EntityTreeModel-" ) + QByteArray::number( qrand() ), this );
//
//   EntityTreeModel *model = new EntityTreeModel( session, monitor, this );
//
//   KDescendantsProxyModel *descsProxy = new KDescendantsProxyModel(this);
//   descsProxy->setSourceModel( model );
//
//   EntityMimeTypeFilterModel *filter = new EntityMimeTypeFilterModel(this);
//   filter->addMimeTypeExclusionFilter( Collection::mimeType() );
//   filter->setSourceModel( descsProxy );
//   m_model = filter;

  Akonadi::ItemModel *itemModel = new Akonadi::ItemModel(this);
  itemModel->fetchScope().fetchFullPayload(true);

  itemModel->setCollection( Collection( m_rootCollectionId ) );

  m_model = itemModel;

  connect( m_model, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(itemsAdded(QModelIndex,int,int)) );
  connect( m_model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), SLOT(itemsRemoved(QModelIndex,int,int)) );
  connect( m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(dataChanged(QModelIndex,QModelIndex)) );

}

void AkonotesMasterApplet::createConfigurationInterface(KConfigDialog *configDialog)
{

  QWidget *widget = new QWidget();
  ui.setupUi(widget);
  configDialog->addPage(widget, i18n("Selected Collection"), "view-media-visualization");

  ui.treeView->header()->hide();

  // TODO: Refactor into a collection chooser widget.
  Akonadi::ChangeRecorder *monitor = new Akonadi::ChangeRecorder( this );
  monitor->fetchCollection( true );
  monitor->setMimeTypeMonitored( "text/x-vnd.akonadi.note", true );
  monitor->setCollectionMonitored( Akonadi::Collection::root() );

  EntityTreeModel *model = new EntityTreeModel( Session::defaultSession(), monitor, this );
  model->setItemPopulationStrategy( EntityTreeModel::NoItemPopulation );

  CollectionFilterProxyModel *mimeTypeFilterModel = new CollectionFilterProxyModel( this );
  mimeTypeFilterModel->addMimeTypeFilter( "text/x-vnd.akonadi.note" );
  mimeTypeFilterModel->setSourceModel( model );

  EntityRightsFilterModel *rightsFilterModel = new EntityRightsFilterModel( this );
  rightsFilterModel->setSourceModel( mimeTypeFilterModel );

  ui.treeView->setModel(rightsFilterModel);

  connect(configDialog, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
  connect(configDialog, SIGNAL(okClicked()), this, SLOT(configAccepted()));
}

void AkonotesMasterApplet::configAccepted()
{
  KConfigGroup cg = config();

  QModelIndexList rows = ui.treeView->selectionModel()->selectedRows();

  if ( rows.isEmpty() )
    return;

  Q_ASSERT( rows.size() == 1 );

  Collection col = rows.at( 0 ).data( EntityTreeModel::CollectionRole ).value<Collection>();

  cg.writeEntry("rootCollection", col.id());

  Akonadi::ItemModel *itemModel = new Akonadi::ItemModel(this);
  itemModel->fetchScope().fetchFullPayload(true);
  itemModel->setCollection( col );

  if ( m_model )
  {
    disconnect( m_model, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(itemsAdded(QModelIndex,int,int)) );
    disconnect( m_model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), this, SLOT(itemsRemoved(QModelIndex,int,int)) );
    disconnect( m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(dataChanged(QModelIndex,QModelIndex)) );
  }

  QList<Plasma::ExtenderItem *> extenderList = extender()->items();
  QList<Plasma::ExtenderItem *>::iterator it;
  QList<Plasma::ExtenderItem *>::iterator item;
  const QList<Plasma::ExtenderItem *>::iterator end = extenderList.end();

  for ( it = extenderList.begin(); it != end; )
  {
    item = it;
    it = extenderList.erase(it);
    ( *item )->destroy();
  }

  m_model = itemModel;

  connect( m_model, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(itemsAdded(QModelIndex,int,int)) );
  connect( m_model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), SLOT(itemsRemoved(QModelIndex,int,int)) );
  connect( m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(dataChanged(QModelIndex,QModelIndex)) );

  setConfigurationRequired( false );

  emit configNeedsSaving();
}

void AkonotesMasterApplet::initExtenderItem( Plasma::ExtenderItem *item, const QModelIndex &idx )
{
    Plasma::TextEdit *textEdit = new Plasma::TextEdit( item );

    Item akonadiItem = idx.data( ItemModel::ItemRole ).value<Akonadi::Item>();
//     Item akonadiItem = idx.data( EntityTreeModel::ItemRole ).value<Akonadi::Item>();
    Q_ASSERT( akonadiItem.isValid() );

    if (!akonadiItem.hasPayload<KMime::Message::Ptr>())
      return;

    KMime::Message::Ptr msg = akonadiItem.payload<KMime::Message::Ptr>();

    textEdit->setText( msg->mainBodyPart()->decodedText() );

    item->setTitle( msg->subject()->asUnicodeString() );

    item->setWidget( textEdit );
}

void AkonotesMasterApplet::paintInterface(QPainter *p,
        const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    p->setRenderHint( QPainter::SmoothPixmapTransform );
    p->setRenderHint( QPainter::Antialiasing );

    // Now we draw the applet, starting with our svg
    m_svg.resize( static_cast<int>( contentsRect.width() ), static_cast<int>( contentsRect.height() ) );
    m_svg.paint( p, static_cast<int>( contentsRect.left() ), static_cast<int>( contentsRect.top() ) );
}

void AkonotesMasterApplet::itemsAdded(const QModelIndex &parent, int start, int end )
{
  const int column = 0;
  for(int row = start; row <= end; ++row)
  {
    Plasma::ExtenderItem *extenderItem = new Plasma::ExtenderItem(extender());
    QModelIndex idx = m_model->index(row, column, parent);
    Q_ASSERT( idx.isValid() );
    initExtenderItem(extenderItem, idx);

    Akonadi::Item item = idx.data(ItemModel::ItemRole ).value<Akonadi::Item>();
    kDebug() << item.isValid() << item.id();

    extenderItem->setName( nameForItem( item ) );

  }
}

void AkonotesMasterApplet::updateItem( const Item &item )
{
    Plasma::ExtenderItem *extenderItem = extenderItemForItem(item);

    if (!extenderItem)
      return;

    if ( !item.hasPayload<KMime::Message::Ptr>() )
      return;

    KMime::Message::Ptr msg = item.payload<KMime::Message::Ptr>();

    Plasma::TextEdit *te = dynamic_cast<Plasma::TextEdit*>( extenderItem->widget() );

    if (!te)
      return;

    te->setText( msg->mainBodyPart()->decodedText() );
}

void AkonotesMasterApplet::dataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight )
{
  QModelIndex idx = topLeft;
  QModelIndex bottomIndex = bottomRight.sibling(bottomRight.row(), 0 );

  while ( ( idx < bottomIndex || idx == bottomIndex ) && idx.isValid() )
  {
    Item item = idx.data(ItemModel::ItemRole).value<Item>();
//     Item item = idx.data(EntityTreeModel::ItemRole).value<Item>();

    updateItem(item);
    idx = idx.sibling( idx.row() + 1, idx.column() );
  }
}

Plasma::ExtenderItem* AkonotesMasterApplet::extenderItemForItem( const Item &item )
{
  return extender()->item( QString::fromLatin1( "item_%1" ).arg( item.id() ) );
}

QString AkonotesMasterApplet::nameForItem( const Akonadi::Item &item )
{
    return QString::fromLatin1( "item_%1" ).arg( item.id() );
}

void AkonotesMasterApplet::itemsRemoved( const QModelIndex &parent, int start, int end )
{
  const int column = 0;
  for(int row = start; row <= end; ++row)
  {
    QModelIndex idx = m_model->index( row, column, parent );

    Q_ASSERT( idx.isValid() );

    Akonadi::Item item = idx.data(ItemModel::ItemRole).value<Item>();
//     Akonadi::Item item = idx.data(EntityTreeModel::ItemRole).value<Item>();

    Q_ASSERT( item.isValid() );

    Plasma::ExtenderItem *extenderItem = extenderItemForItem(item);

    extenderItem->destroy();

  }

}

#include "akonotes_applet.moc"
