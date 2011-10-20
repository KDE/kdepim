/* -*- mode: C++; c-file-style: "gnu" -*-
  This file is part of KMail, the KDE mail client.
  Copyright (c) 2009, 2010 Montel Laurent <montel@kde.org>

  KMail is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  KMail is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "foldertreewidget.h"
#include "foldertreeview.h"
#include "imapaclattribute.h"
#include "foldertreewidgetproxymodel.h"
#include "mailkernel.h"
#include "entitycollectionorderproxymodel.h"

#include "messageviewer/globalsettings.h"
#include "messagecore/globalsettings.h"

#include <akonadi/attributefactory.h>
#include <akonadi/entitytreeview.h>
#include <akonadi/changerecorder.h>
#include <akonadi/session.h>
#include <akonadi/entitytreemodel.h>
#include <akonadi/entitymimetypefiltermodel.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/collection.h>
#include <akonadi/statisticsproxymodel.h>
#include <akonadi_next/quotacolorproxymodel.h>
#include <akonadi/recursivecollectionfilterproxymodel.h>
#include <akonadi/etmviewstatesaver.h>
#include <krecursivefilterproxymodel.h>

#include <QKeyEvent>
#include <QLabel>

#include <klineedit.h>
#include <klocalizedstring.h>

namespace MailCommon {
  
class FolderTreeWidget::FolderTreeWidgetPrivate
{
public:
  FolderTreeWidgetPrivate()
    :filterModel( 0 ),
     folderTreeView( 0 ),
     quotaModel( 0 ),
     readableproxy( 0 ),
     entityOrderProxy( 0 ),
     filterFolderLineEdit( 0 ),
     saver( 0 ),
     label( 0 ), 
     dontKeyFilter( false )
  {
  }
  QString filter;
  QString oldFilterStr;
  Akonadi::StatisticsProxyModel *filterModel;
  FolderTreeView *folderTreeView;
  Akonadi::QuotaColorProxyModel *quotaModel;
  FolderTreeWidgetProxyModel *readableproxy;
  EntityCollectionOrderProxyModel *entityOrderProxy;
  KLineEdit *filterFolderLineEdit;
  QPointer<Akonadi::ETMViewStateSaver> saver;
  QStringList expandedItems;
  QString currentItem;
  QLabel *label;
  bool dontKeyFilter;
};


FolderTreeWidget::FolderTreeWidget( QWidget* parent, KXMLGUIClient* xmlGuiClient, FolderTreeWidget::TreeViewOptions options, FolderTreeWidgetProxyModel::FolderTreeWidgetProxyModelOptions optReadableProxy )
  : QWidget( parent ), d( new FolderTreeWidgetPrivate() )
{
  Akonadi::AttributeFactory::registerAttribute<MailCommon::ImapAclAttribute>();

  d->folderTreeView = new FolderTreeView( xmlGuiClient, this, options & ShowUnreadCount );
  d->folderTreeView->showStatisticAnimation( options & ShowCollectionStatisticAnimation );

  connect( d->folderTreeView, SIGNAL(manualSortingChanged(bool)), this, SLOT(slotManualSortingChanged(bool)) );

  QVBoxLayout *lay = new QVBoxLayout( this );
  lay->setMargin( 0 );

  d->label = new QLabel( i18n("You can start typing to filter the list of folders."), this);
  lay->addWidget( d->label );

  d->filterFolderLineEdit = new KLineEdit( this );
  d->filterFolderLineEdit->setClearButtonShown( true );
  d->filterFolderLineEdit->setClickMessage( i18nc( "@info/plain Displayed grayed-out inside the "
                                                   "textbox, verb to search", "Search" ) );
  lay->addWidget( d->filterFolderLineEdit );

  Akonadi::RecursiveCollectionFilterProxyModel *recurfilter = new Akonadi::RecursiveCollectionFilterProxyModel( this );
  recurfilter->addContentMimeTypeInclusionFilter( KMime::Message::mimeType() );
  recurfilter->setSourceModel( KernelIf->collectionModel() );

  // ... with statistics...
  d->filterModel = new Akonadi::StatisticsProxyModel( this );
  d->filterModel->setSourceModel( recurfilter );

  d->quotaModel = new Akonadi::QuotaColorProxyModel( this );
  d->quotaModel->setSourceModel( d->filterModel );

  d->readableproxy = new FolderTreeWidgetProxyModel( this, optReadableProxy );
  d->readableproxy->setSourceModel( d->quotaModel );


  connect( d->folderTreeView, SIGNAL(changeTooltipsPolicy(FolderTreeWidget::ToolTipDisplayPolicy)),
           this, SLOT(slotChangeTooltipsPolicy(FolderTreeWidget::ToolTipDisplayPolicy)) );

  d->folderTreeView->setSelectionMode( QAbstractItemView::SingleSelection );
  d->folderTreeView->setEditTriggers( QAbstractItemView::NoEditTriggers );
  d->folderTreeView->installEventFilter( this );

  
  //Order proxy
  d->entityOrderProxy = new EntityCollectionOrderProxyModel( this );
  d->entityOrderProxy->setSourceModel( d->readableproxy );
  KConfigGroup grp( KernelIf->config(), "CollectionTreeOrder" );
  d->entityOrderProxy->setOrderConfig( grp );
  d->folderTreeView->setModel( d->entityOrderProxy );

  if ( options & UseDistinctSelectionModel )
    d->folderTreeView->setSelectionModel( new QItemSelectionModel( d->entityOrderProxy, this ) );

  lay->addWidget( d->folderTreeView );

  d->dontKeyFilter = ( options & DontKeyFilter );

  if ( ( options & UseLineEditForFiltering ) ) {
    connect( d->filterFolderLineEdit, SIGNAL(textChanged(QString)),
             this, SLOT(slotFilterFixedString(QString)) );
    d->label->hide();
  } else {
    d->filterFolderLineEdit->hide();
  }
  connect( KGlobalSettings::self(), SIGNAL(kdisplayFontChanged()), this,  SLOT(slotGeneralFontChanged()) );
  readConfig();
}

FolderTreeWidget::~FolderTreeWidget()
{
  delete d;
}

void FolderTreeWidget::slotFilterFixedString( const QString& text )
{
  delete d->saver;
  if ( d->oldFilterStr.isEmpty() ) {
    //Save it.
    Akonadi::ETMViewStateSaver saver;
    saver.setView( folderTreeView() );
    d->expandedItems = saver.expansionKeys();
    d->currentItem = saver.currentIndexKey();
  } else if ( text.isEmpty() ) {
    
    d->saver = new Akonadi::ETMViewStateSaver;
    d->saver->setView( folderTreeView() );
    QString currentIndex = d->saver->currentIndexKey();
    if( d->saver->selectionKeys().isEmpty() )
	currentIndex = d->currentItem;
    else if( !currentIndex.isEmpty() ) 
        d->expandedItems<<currentIndex;
    d->saver->restoreExpanded( d->expandedItems );
    d->saver->restoreCurrentItem( currentIndex );
  } else {
    d->folderTreeView->expandAll();

  }
  d->oldFilterStr = text;
  d->readableproxy->setFilterFolder( text );
}
  
void FolderTreeWidget::disableContextMenuAndExtraColumn()
{
  d->folderTreeView->disableContextMenuAndExtraColumn();
}

void FolderTreeWidget::selectCollectionFolder( const Akonadi::Collection &collection )
{
  const QModelIndex index = Akonadi::EntityTreeModel::modelIndexForCollection( d->folderTreeView->model(), collection );

  d->folderTreeView->selectionModel()->select( index, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows );
  d->folderTreeView->setExpanded( index, true );
  d->folderTreeView->scrollTo( index );
}

void FolderTreeWidget::setSelectionMode( QAbstractItemView::SelectionMode mode )
{
  d->folderTreeView->setSelectionMode( mode );
}

QAbstractItemView::SelectionMode FolderTreeWidget::selectionMode() const
{
  return d->folderTreeView->selectionMode();
}


QItemSelectionModel * FolderTreeWidget::selectionModel () const
{
  return d->folderTreeView->selectionModel();
}

QModelIndex FolderTreeWidget::currentIndex() const
{
  return d->folderTreeView->currentIndex();
}


Akonadi::Collection FolderTreeWidget::selectedCollection() const
{
  if ( d->folderTreeView->selectionMode() == QAbstractItemView::SingleSelection ) {
    const QModelIndex selectedIndex = d->folderTreeView->currentIndex();
    QModelIndex index = selectedIndex.sibling( selectedIndex.row(), 0 );
    if ( index.isValid() )
      return index.model()->data( index, Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
  }

  return Akonadi::Collection();
}

Akonadi::Collection::List FolderTreeWidget::selectedCollections() const
{
  Akonadi::Collection::List collections;
  const QItemSelectionModel *selectionModel = d->folderTreeView->selectionModel();
  const QModelIndexList selectedIndexes = selectionModel->selectedIndexes();
  foreach ( const QModelIndex &index, selectedIndexes ) {
    if ( index.isValid() ) {
      const Akonadi::Collection collection =
          index.model()->data( index, Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
      if ( collection.isValid() )
        collections.append( collection );
    }
  }

  return collections;
}

FolderTreeView* FolderTreeWidget::folderTreeView() const
{
  return d->folderTreeView;
}

void FolderTreeWidget::slotGeneralFontChanged()
{
  // Custom/System font support
  if (MessageCore::GlobalSettings::self()->useDefaultFonts() ) {
    setFont( KGlobalSettings::generalFont() );
  }
}
  
void FolderTreeWidget::readConfig()
{
  // Custom/System font support
  if (!MessageCore::GlobalSettings::self()->useDefaultFonts() ) {
    KConfigGroup fontConfig( KernelIf->config(), "Fonts" );
    setFont( fontConfig.readEntry("folder-font", KGlobalSettings::generalFont() ) );
  } else {
    setFont( KGlobalSettings::generalFont() );
  }

  KConfigGroup mainFolderView( KernelIf->config(), "MainFolderView" );
  const int checkedFolderToolTipsPolicy = mainFolderView.readEntry( "ToolTipDisplayPolicy", 0 );
  changeToolTipsPolicyConfig( ( ToolTipDisplayPolicy )checkedFolderToolTipsPolicy );

  d->folderTreeView->setDropActionMenuEnabled( SettingsIf->showPopupAfterDnD() );
  readQuotaConfig();
}

void FolderTreeWidget::restoreHeaderState( const QByteArray& data )
{
  d->folderTreeView->restoreHeaderState( data );
}


void FolderTreeWidget::slotChangeTooltipsPolicy( FolderTreeWidget::ToolTipDisplayPolicy policy)
{
  changeToolTipsPolicyConfig( policy );
}

void FolderTreeWidget::changeToolTipsPolicyConfig( ToolTipDisplayPolicy policy )
{
  switch( policy ){
  case DisplayAlways:
  case DisplayWhenTextElided: //Need to implement in the future
    d->filterModel->setToolTipEnabled( true );
    break;
  case DisplayNever:
    d->filterModel->setToolTipEnabled( false );
  }
  d->folderTreeView->setTooltipsPolicy( policy );
}

void FolderTreeWidget::quotaWarningParameters( const QColor &color, qreal threshold )
{
  d->quotaModel->setWarningThreshold( threshold );
  d->quotaModel->setWarningColor( color );
}

void FolderTreeWidget::readQuotaConfig()
{
  QColor quotaColor;
  qreal threshold = 100;
  if ( !MessageCore::GlobalSettings::self()->useDefaultColors() ) {
    KConfigGroup readerConfig( KernelIf->config(), "Reader" );
    quotaColor = readerConfig.readEntry( "CloseToQuotaColor", quotaColor  );
    threshold = SettingsIf->closeToQuotaThreshold();
  }
  quotaWarningParameters( quotaColor, threshold );
}

Akonadi::StatisticsProxyModel * FolderTreeWidget::statisticsProxyModel() const
{
  return d->filterModel;
}

FolderTreeWidgetProxyModel *FolderTreeWidget::folderTreeWidgetProxyModel() const
{
  return d->readableproxy;
}

EntityCollectionOrderProxyModel *FolderTreeWidget::entityOrderProxy() const
{
  return d->entityOrderProxy;
}

KLineEdit *FolderTreeWidget::filterFolderLineEdit() const
{
  return d->filterFolderLineEdit;
}

void FolderTreeWidget::applyFilter( const QString &filter )
{
  d->label->setText( filter.isEmpty() ? i18n( "You can start typing to filter the list of folders." )
                                      : i18n( "Path: (%1)", filter ) );
  d->readableproxy->setFilterFolder( filter );
  d->folderTreeView->expandAll();
}

void FolderTreeWidget::clearFilter()
{
  d->filter.clear();
  applyFilter( d->filter );
  if ( !d->folderTreeView->selectionModel()->selectedIndexes().isEmpty() )
    d->folderTreeView->scrollTo( d->folderTreeView->selectionModel()->selectedIndexes().first() );
}

void FolderTreeWidget::slotManualSortingChanged( bool active )
{
  d->entityOrderProxy->setManualSortingActive( active );
}

bool FolderTreeWidget::eventFilter( QObject* o, QEvent *e )
{
  Q_UNUSED( o );
  if ( d->dontKeyFilter )
    return false;

  if ( e->type() == QEvent::KeyPress ) {
    const QKeyEvent* const ke = static_cast<QKeyEvent*>( e );
    switch( ke->key() )
    {
      case Qt::Key_Backspace:
        {
        const int filterLength(d->filter.length() ); 
        if ( filterLength > 0 )
          d->filter.truncate( filterLength-1 );
        applyFilter( d->filter );
        return false;
        }
        break;
      case Qt::Key_Delete:
        d->filter.clear();
        applyFilter( d->filter);
        return false;
        break;
      default:
      {
        const QString s = ke->text();
        if ( !s.isEmpty() && s.at( 0 ).isPrint() ) {
          d->filter += s;
          applyFilter( d->filter );
          return false;
        }
      }
      break;
    }
  }
  return false;
}

}

#include "foldertreewidget.moc"
