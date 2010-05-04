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

#include "kbreadcrumbnavigationcomponentfactory.h"

#include <QAbstractItemModel>
#include <QDeclarativeContext>

#include "kselectionproxymodel.h"
#include "akonadi_next/kbreadcrumbselectionmodel.h"
#include "akonadi_next/kproxyitemselectionmodel.h"
#include "akonadi_next/kmodelindexproxymapper.h"

#include "breadcrumbnavigation.h"

class KBreadcrumbNavigationComponentFactoryPrivate
{
  KBreadcrumbNavigationComponentFactoryPrivate(KBreadcrumbNavigationComponentFactory *qq)
    : q_ptr(qq),
      m_breadcrumbSelectionModel(0),
      m_selectionModel(0),
      m_childItemsSelectionModel(0),
      m_breadcrumbModel(0),
      m_selectedItemModel(0),
      m_unfilteredChildItemsModel(0),
      m_childItemsModel(0),
      m_breadcrumbDepth(-1),
      m_modelIndexProxyMapper(0)
  {

  }
  Q_DECLARE_PUBLIC(KBreadcrumbNavigationComponentFactory)
  KBreadcrumbNavigationComponentFactory * const q_ptr;

  QItemSelectionModel *m_breadcrumbSelectionModel;
  QItemSelectionModel *m_selectionModel;
  QItemSelectionModel *m_childItemsSelectionModel;

  QAbstractItemModel *m_breadcrumbModel;
  QAbstractItemModel *m_selectedItemModel;
  QAbstractItemModel *m_unfilteredChildItemsModel;
  QAbstractItemModel *m_childItemsModel;
  int m_breadcrumbDepth;
  Future::KModelIndexProxyMapper *m_modelIndexProxyMapper;
};

KBreadcrumbNavigationComponentFactory::KBreadcrumbNavigationComponentFactory(QObject* parent)
  : QObject(parent), d_ptr(new KBreadcrumbNavigationComponentFactoryPrivate(this))
{

}

void KBreadcrumbNavigationComponentFactory::setModel(QAbstractItemModel *model, QObject* parent)
{
  Q_D(KBreadcrumbNavigationComponentFactory);

  d->m_selectionModel = new QItemSelectionModel( model, parent );

  KSelectionProxyModel *currentCollectionSelectionModel = new KSelectionProxyModel( d->m_selectionModel, parent );
  currentCollectionSelectionModel->setFilterBehavior( KSelectionProxyModel::ExactSelection );
  currentCollectionSelectionModel->setSourceModel( model );
  d->m_selectedItemModel = currentCollectionSelectionModel;

  Future::KBreadcrumbSelectionModel *breadcrumbCollectionSelection
      = new Future::KBreadcrumbSelectionModel( d->m_selectionModel, Future::KBreadcrumbSelectionModel::Forward, parent );
  breadcrumbCollectionSelection->setIncludeActualSelection(false);
  breadcrumbCollectionSelection->setSelectionDepth( d->m_breadcrumbDepth );

  KBreadcrumbNavigationProxyModel *breadcrumbNavigationModel
      = new KBreadcrumbNavigationProxyModel( breadcrumbCollectionSelection, parent );
  breadcrumbNavigationModel->setSourceModel( model );
  breadcrumbNavigationModel->setFilterBehavior( KSelectionProxyModel::ExactSelection );
  d->m_breadcrumbModel = getBreadcrumbNavigationModel(breadcrumbNavigationModel);

  Future::KProxyItemSelectionModel *proxyBreadcrumbCollectionSelection
      = new Future::KProxyItemSelectionModel( d->m_breadcrumbModel, d->m_selectionModel, parent );

  d->m_breadcrumbSelectionModel = new KForwardingItemSelectionModel( d->m_breadcrumbModel,
                                                                     proxyBreadcrumbCollectionSelection,
                                                                     KForwardingItemSelectionModel::Reverse,
                                                                     parent );

  // Breadcrumbs done. (phew!)

  KForwardingItemSelectionModel *oneway = new KForwardingItemSelectionModel( model, d->m_selectionModel, parent );

  KNavigatingProxyModel *navigatingProxyModel = new KNavigatingProxyModel( oneway, parent );
  navigatingProxyModel->setSourceModel( model );
  d->m_unfilteredChildItemsModel = navigatingProxyModel;

  d->m_childItemsModel = getChildItemsModel(d->m_unfilteredChildItemsModel);

  d->m_childItemsSelectionModel = new Future::KProxyItemSelectionModel( d->m_childItemsModel, d->m_selectionModel, parent );

  d->m_modelIndexProxyMapper = new Future::KModelIndexProxyMapper(model, d->m_childItemsModel, this);

}

QItemSelectionModel* KBreadcrumbNavigationComponentFactory::selectionModel() const
{
  Q_D(const KBreadcrumbNavigationComponentFactory);
  return d->m_selectionModel;
}

QAbstractItemModel* KBreadcrumbNavigationComponentFactory::selectedItemModel() const
{
  Q_D(const KBreadcrumbNavigationComponentFactory);
  return d->m_selectedItemModel;
}

int KBreadcrumbNavigationComponentFactory::breadcrumbDepth() const
{
  Q_D(const KBreadcrumbNavigationComponentFactory);
  return d->m_breadcrumbDepth;
}

void KBreadcrumbNavigationComponentFactory::setBreadcrumbDepth(int depth)
{
  Q_D(KBreadcrumbNavigationComponentFactory);
  d->m_breadcrumbDepth = depth;
}

QAbstractItemModel* KBreadcrumbNavigationComponentFactory::breadcrumbItemModel() const
{
  Q_D(const KBreadcrumbNavigationComponentFactory);
  return d->m_breadcrumbModel;
}

QItemSelectionModel* KBreadcrumbNavigationComponentFactory::breadcrumbSelectionModel() const
{
  Q_D(const KBreadcrumbNavigationComponentFactory);
  return d->m_breadcrumbSelectionModel;
}

QAbstractItemModel* KBreadcrumbNavigationComponentFactory::childItemModel() const
{
  Q_D(const KBreadcrumbNavigationComponentFactory);
  return d->m_childItemsModel;
}

QAbstractItemModel* KBreadcrumbNavigationComponentFactory::unfilteredChildItemModel() const
{
  Q_D(const KBreadcrumbNavigationComponentFactory);
  return d->m_unfilteredChildItemsModel;
}

QAbstractItemModel* KBreadcrumbNavigationComponentFactory::getBreadcrumbNavigationModel(QAbstractItemModel* model)
{
  return model;
}

QAbstractItemModel* KBreadcrumbNavigationComponentFactory::getChildItemsModel(QAbstractItemModel* model)
{
  return model;
}

QItemSelectionModel* KBreadcrumbNavigationComponentFactory::childSelectionModel() const
{
  Q_D(const KBreadcrumbNavigationComponentFactory);
  return d->m_childItemsSelectionModel;
}

void KBreadcrumbNavigationComponentFactory::selectBreadcrumb(int row)
{
  Q_D(KBreadcrumbNavigationComponentFactory);
  if ( row < 0 )
  {
    d->m_selectionModel->clearSelection();
    return;
  }
  QModelIndex index = d->m_breadcrumbModel->index( row, 0 );
  d->m_breadcrumbSelectionModel->select( QItemSelection(index, index), QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect );
}

void KBreadcrumbNavigationComponentFactory::selectChild(int row)
{
  Q_D(KBreadcrumbNavigationComponentFactory);
  if ( row < 0 )
  {
    d->m_selectionModel->clearSelection();
    return;
  }
  const QModelIndex index = d->m_childItemsModel->index( row, 0 );
  d->m_childItemsSelectionModel->select( QItemSelection(index, index), QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect );
}

bool KBreadcrumbNavigationComponentFactory::childCollectionHasChildren(int row)
{
  if ( row < 0 )
    return false;

  Q_D(KBreadcrumbNavigationComponentFactory);

  static const int column = 0;
  const QModelIndex idx = d->m_modelIndexProxyMapper->mapRightToLeft(d->m_childItemsModel->index(row, column));
  if (!idx.isValid())
    return false;

  return idx.model()->rowCount( idx ) > 0;
}

#include "breadcrumbnavigationcontext.moc"
