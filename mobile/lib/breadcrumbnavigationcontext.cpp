/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Stephen Kelly <stephen@kdab.com>

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

#include "breadcrumbnavigationcontext.h"

#include "breadcrumbnavigation.h"
#include "kbreadcrumbselectionmodel.h"
#include "klinkitemselectionmodel.h"
#include "kmodelindexproxymapper.h"
#include <kdescendantsproxymodel.h>
#include "kselectionproxymodel.h"
#include "qmlcheckableproxymodel.h"
#include "qmllistselectionmodel.h"

#include <QtCore/QAbstractItemModel>

class KBreadcrumbNavigationFactoryPrivate
{
  KBreadcrumbNavigationFactoryPrivate( KBreadcrumbNavigationFactory *qq )
    : q_ptr( qq ),
      m_breadcrumbSelectionModel( 0 ),
      m_selectionModel( 0 ),
      m_childItemsSelectionModel( 0 ),
      m_breadcrumbModel( 0 ),
      m_selectedItemModel( 0 ),
      m_unfilteredChildItemsModel( 0 ),
      m_childItemsModel( 0 ),
      m_breadcrumbDepth( -1 ),
      m_modelIndexProxyMapper( 0 ),
      m_checkModel( 0 ),
      m_qmlBreadcrumbSelectionModel( 0 ),
      m_qmlSelectedItemSelectionModel( 0 ),
      m_qmlChildSelectionModel( 0 ),
      m_qmlBreadcrumbCheckModel( 0 ),
      m_qmlSelectedItemCheckModel( 0 ),
      m_qmlChildCheckModel( 0 ),
      m_checkedItemsModel( 0 ),
      m_checkedItemsCheckModel( 0 ),
      m_qmlCheckedItemsCheckModel( 0 )
  {
  }

  Q_DECLARE_PUBLIC( KBreadcrumbNavigationFactory )
  KBreadcrumbNavigationFactory * const q_ptr;

  QItemSelectionModel *m_breadcrumbSelectionModel;
  QItemSelectionModel *m_selectionModel;
  QItemSelectionModel *m_childItemsSelectionModel;

  QAbstractItemModel *m_breadcrumbModel;
  QAbstractItemModel *m_selectedItemModel;
  QAbstractItemModel *m_unfilteredChildItemsModel;
  QAbstractItemModel *m_childItemsModel;
  int m_breadcrumbDepth;
  KModelIndexProxyMapper *m_modelIndexProxyMapper;

  QItemSelectionModel *m_checkModel;

  QMLListSelectionModel *m_qmlBreadcrumbSelectionModel;
  QMLListSelectionModel *m_qmlSelectedItemSelectionModel;
  QMLListSelectionModel *m_qmlChildSelectionModel;

  QMLListSelectionModel *m_qmlBreadcrumbCheckModel;
  QMLListSelectionModel *m_qmlSelectedItemCheckModel;
  QMLListSelectionModel *m_qmlChildCheckModel;

  KSelectionProxyModel *m_checkedItemsModel;
  QItemSelectionModel *m_checkedItemsCheckModel;
  QMLListSelectionModel *m_qmlCheckedItemsCheckModel;
};


KBreadcrumbNavigationFactory::KBreadcrumbNavigationFactory( QObject *parent )
  : QObject( parent ),
    d_ptr( new KBreadcrumbNavigationFactoryPrivate( this ) )
{
}

KBreadcrumbNavigationFactory::~KBreadcrumbNavigationFactory()
{
  delete d_ptr;
}

void KBreadcrumbNavigationFactory::createCheckableBreadcrumbContext( QAbstractItemModel *model, QObject *parent )
{
  Q_D( KBreadcrumbNavigationFactory );

  d->m_checkModel = new QItemSelectionModel( model, parent );

  QMLCheckableItemProxyModel *checkableProxy = new QMLCheckableItemProxyModel( parent );
  checkableProxy->setSourceModel( model );
  checkableProxy->setSelectionModel( d->m_checkModel );

  KDescendantsProxyModel *descProxy = new KDescendantsProxyModel( parent );
  descProxy->setDisplayAncestorData( true );
  descProxy->setSourceModel( checkableProxy );

  createBreadcrumbContext( checkableProxy, parent );

  KLinkItemSelectionModel *breadcrumbLinkSelectionModel = new KLinkItemSelectionModel( d->m_breadcrumbModel, d->m_checkModel, parent );
  KLinkItemSelectionModel *childLinkSelectionModel = new KLinkItemSelectionModel( d->m_childItemsModel, d->m_checkModel, parent );
  KLinkItemSelectionModel *selectedItemLinkSelectionModel = new KLinkItemSelectionModel( d->m_selectedItemModel, d->m_checkModel, parent );

  d->m_qmlBreadcrumbCheckModel = new QMLListSelectionModel( breadcrumbLinkSelectionModel, parent );
  d->m_qmlSelectedItemCheckModel = new QMLListSelectionModel( selectedItemLinkSelectionModel, parent );
  d->m_qmlChildCheckModel = new QMLListSelectionModel( childLinkSelectionModel, parent );

  d->m_checkedItemsModel = new KSelectionProxyModel( d->m_checkModel, parent );
  d->m_checkedItemsModel->setFilterBehavior( KSelectionProxyModel::ExactSelection );
  d->m_checkedItemsModel->setSourceModel( descProxy );

  d->m_checkedItemsCheckModel = new KLinkItemSelectionModel( d->m_checkedItemsModel, d->m_checkModel, parent );

  d->m_qmlCheckedItemsCheckModel = new QMLListSelectionModel( d->m_checkedItemsCheckModel, parent );
}

void KBreadcrumbNavigationFactory::createBreadcrumbContext( QAbstractItemModel *model, QObject *parent )
{
  Q_D(KBreadcrumbNavigationFactory);

  d->m_selectionModel = new QItemSelectionModel( model, parent );
  connect( d->m_selectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SIGNAL(collectionSelectionChanged()) );

  KSelectionProxyModel *currentCollectionSelectionModel = new KSelectionProxyModel( d->m_selectionModel, parent );
  currentCollectionSelectionModel->setFilterBehavior( KSelectionProxyModel::ExactSelection );
  currentCollectionSelectionModel->setSourceModel( model );
  d->m_selectedItemModel = currentCollectionSelectionModel;

  KBreadcrumbSelectionModel *breadcrumbCollectionSelection
      = new KBreadcrumbSelectionModel( d->m_selectionModel, KBreadcrumbSelectionModel::MakeBreadcrumbSelectionInOther, parent );
  breadcrumbCollectionSelection->setActualSelectionIncluded( false );
  breadcrumbCollectionSelection->setBreadcrumbLength( d->m_breadcrumbDepth );

  KBreadcrumbNavigationProxyModel *breadcrumbNavigationModel
      = new KBreadcrumbNavigationProxyModel( breadcrumbCollectionSelection, parent );
  breadcrumbNavigationModel->setSourceModel( model );
  breadcrumbNavigationModel->setFilterBehavior( KSelectionProxyModel::ExactSelection );
  d->m_breadcrumbModel = getBreadcrumbNavigationModel( breadcrumbNavigationModel );

  KLinkItemSelectionModel *proxyBreadcrumbCollectionSelection
      = new KLinkItemSelectionModel( d->m_breadcrumbModel, d->m_selectionModel, parent );

  d->m_breadcrumbSelectionModel = new KForwardingItemSelectionModel( d->m_breadcrumbModel,
                                                                     proxyBreadcrumbCollectionSelection,
                                                                     KForwardingItemSelectionModel::Reverse,
                                                                     parent );

  // Breadcrumbs done. (phew!)

  KForwardingItemSelectionModel *oneway = new KForwardingItemSelectionModel( model, d->m_selectionModel, parent );

  KNavigatingProxyModel *navigatingProxyModel = new KNavigatingProxyModel( oneway, parent );
  navigatingProxyModel->setSourceModel( model );
  d->m_unfilteredChildItemsModel = navigatingProxyModel;

  d->m_childItemsModel = getChildItemsModel( d->m_unfilteredChildItemsModel );

  d->m_childItemsSelectionModel = new KLinkItemSelectionModel( d->m_childItemsModel, d->m_selectionModel, parent );

  d->m_modelIndexProxyMapper = new KModelIndexProxyMapper( model, d->m_childItemsModel, parent );

  // Navigation stuff for QML:

  d->m_qmlBreadcrumbSelectionModel = new QMLListSelectionModel( d->m_breadcrumbSelectionModel, parent );
  d->m_qmlSelectedItemSelectionModel = new QMLListSelectionModel( d->m_selectionModel, parent );
  d->m_qmlChildSelectionModel = new QMLListSelectionModel( d->m_childItemsSelectionModel, parent );

  connect( d->m_selectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SIGNAL(selectedDisplayTextChanged()) );
}

QItemSelectionModel* KBreadcrumbNavigationFactory::selectionModel() const
{
  Q_D( const KBreadcrumbNavigationFactory );
  return d->m_selectionModel;
}

QAbstractItemModel* KBreadcrumbNavigationFactory::selectedItemModel() const
{
  Q_D( const KBreadcrumbNavigationFactory );
  return d->m_selectedItemModel;
}

int KBreadcrumbNavigationFactory::breadcrumbDepth() const
{
  Q_D( const KBreadcrumbNavigationFactory );
  return d->m_breadcrumbDepth;
}

void KBreadcrumbNavigationFactory::setBreadcrumbDepth( int depth )
{
  Q_D( KBreadcrumbNavigationFactory );
  d->m_breadcrumbDepth = depth;
}

QAbstractItemModel* KBreadcrumbNavigationFactory::breadcrumbItemModel() const
{
  Q_D( const KBreadcrumbNavigationFactory );
  return d->m_breadcrumbModel;
}

QItemSelectionModel* KBreadcrumbNavigationFactory::breadcrumbSelectionModel() const
{
  Q_D( const KBreadcrumbNavigationFactory );
  return d->m_breadcrumbSelectionModel;
}

QAbstractItemModel* KBreadcrumbNavigationFactory::childItemModel() const
{
  Q_D( const KBreadcrumbNavigationFactory );
  return d->m_childItemsModel;
}

QAbstractItemModel* KBreadcrumbNavigationFactory::unfilteredChildItemModel() const
{
  Q_D( const KBreadcrumbNavigationFactory );
  return d->m_unfilteredChildItemsModel;
}

QAbstractItemModel* KBreadcrumbNavigationFactory::getBreadcrumbNavigationModel( QAbstractItemModel *model )
{
  return model;
}

QAbstractItemModel* KBreadcrumbNavigationFactory::getChildItemsModel( QAbstractItemModel *model )
{
  return model;
}

QItemSelectionModel* KBreadcrumbNavigationFactory::childSelectionModel() const
{
  Q_D( const KBreadcrumbNavigationFactory );
  return d->m_childItemsSelectionModel;
}

void KBreadcrumbNavigationFactory::selectBreadcrumb( int row )
{
  Q_D( KBreadcrumbNavigationFactory );

  if ( row < 0 ) {
    d->m_selectionModel->clearSelection();
    return;
  }

  const QModelIndex index = d->m_breadcrumbModel->index( row, 0 );
  d->m_breadcrumbSelectionModel->select( QItemSelection( index, index ), QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect );
}

void KBreadcrumbNavigationFactory::selectChild( int row )
{
  Q_D( KBreadcrumbNavigationFactory );

  if ( row < 0 ) {
    d->m_selectionModel->clearSelection();
    return;
  }

  const QModelIndex index = d->m_childItemsModel->index( row, 0 );
  d->m_childItemsSelectionModel->select( QItemSelection( index, index ), QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect );
}

bool KBreadcrumbNavigationFactory::childCollectionHasChildren( int row ) const
{
  if ( row < 0 )
    return false;

  Q_D( const KBreadcrumbNavigationFactory );

  static const int column = 0;
  const QModelIndex index = d->m_modelIndexProxyMapper->mapRightToLeft( d->m_childItemsModel->index( row, column ) );
  if ( !index.isValid() )
    return false;

  return (index.model()->rowCount( index ) > 0);
}

QObject* KBreadcrumbNavigationFactory::qmlBreadcrumbSelectionModel() const
{
  Q_D( const KBreadcrumbNavigationFactory );
  return d->m_qmlBreadcrumbSelectionModel;
}

QObject* KBreadcrumbNavigationFactory::qmlBreadcrumbsModel() const
{
  Q_D( const KBreadcrumbNavigationFactory );
  return d->m_breadcrumbModel;
}

QObject* KBreadcrumbNavigationFactory::qmlChildItemsModel() const
{
  Q_D( const KBreadcrumbNavigationFactory );
  return d->m_childItemsModel;
}

QObject* KBreadcrumbNavigationFactory::qmlChildSelectionModel() const
{
  Q_D( const KBreadcrumbNavigationFactory );
  return d->m_qmlChildSelectionModel;
}

QObject* KBreadcrumbNavigationFactory::qmlSelectedItemModel() const
{
  Q_D( const KBreadcrumbNavigationFactory );
  return d->m_selectedItemModel;
}

QObject* KBreadcrumbNavigationFactory::qmlSelectionModel() const
{
  Q_D( const KBreadcrumbNavigationFactory );
  return d->m_qmlSelectedItemSelectionModel;
}

QObject* KBreadcrumbNavigationFactory::qmlBreadcrumbCheckModel() const
{
  Q_D( const KBreadcrumbNavigationFactory );
  return d->m_qmlBreadcrumbCheckModel;
}

QObject* KBreadcrumbNavigationFactory::qmlChildCheckModel() const
{
  Q_D( const KBreadcrumbNavigationFactory );
  return d->m_qmlChildCheckModel;
}

QObject* KBreadcrumbNavigationFactory::qmlSelectedItemCheckModel() const
{
  Q_D( const KBreadcrumbNavigationFactory );
  return d->m_qmlSelectedItemCheckModel;
}

QItemSelectionModel* KBreadcrumbNavigationFactory::checkedItemsCheckModel() const
{
  Q_D( const KBreadcrumbNavigationFactory );
  return d->m_checkedItemsCheckModel;
}

QAbstractItemModel* KBreadcrumbNavigationFactory::checkedItemsModel() const
{
  Q_D( const KBreadcrumbNavigationFactory );
  return d->m_checkedItemsModel;
}

QItemSelectionModel* KBreadcrumbNavigationFactory::checkModel() const
{
  Q_D( const KBreadcrumbNavigationFactory );
  return d->m_checkModel;
}

QObject* KBreadcrumbNavigationFactory::qmlCheckedItemsCheckModel() const
{
  Q_D( const KBreadcrumbNavigationFactory );
  return d->m_qmlCheckedItemsCheckModel;
}

QObject* KBreadcrumbNavigationFactory::qmlCheckedItemsModel() const
{
  Q_D( const KBreadcrumbNavigationFactory );
  return d->m_checkedItemsModel;
}

QString KBreadcrumbNavigationFactory::selectedDisplayText() const
{
  Q_D( const KBreadcrumbNavigationFactory );
  return d->m_selectedItemModel->index( 0, 0 ).data().toString();
}

