/*
    Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Copyright (c) 2010 Andras Mantia <andras@kdab.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#include "filtermodel.h"
#include "mailcommon/mailkernel.h"
#include "mailcommon/filtermanager.h"
#include "mailcommon/mailfilter.h"

FilterModel::FilterModel( QObject *parent )
  : QAbstractListModel( parent )
{
  connect( FilterIf->filterManager(), SIGNAL( filterListUpdated() ),
           this, SLOT( filterListUpdated() ) );
}

QVariant FilterModel::data( const QModelIndex &index, int role ) const
{
  if ( role == Qt::DisplayRole ) {
    return FilterIf->filterManager()->filters().at( index.row() )->name();
  }

  return QVariant();
}

int FilterModel::rowCount( const QModelIndex& ) const
{
  return FilterIf->filterManager()->filters().size();
}

void FilterModel::moveRow( int sourceRow, int destinationRow )
{
  if ( sourceRow == destinationRow )
    return;

  if ( sourceRow < 0 || sourceRow >= rowCount() )
    return;

  if ( destinationRow < 0 || destinationRow >= rowCount() )
    return;

  if ( !beginMoveRows( QModelIndex(), sourceRow, sourceRow, QModelIndex(), destinationRow ) )
    return;

  QList<MailCommon::MailFilter*> filters;

  MailCommon::FilterManager *manager = FilterIf->filterManager();

  foreach ( MailCommon::MailFilter *filter, manager->filters() ) {
    filters.append( new MailCommon::MailFilter( *filter ) ); // deep copy
  }

  filters.move( sourceRow, destinationRow );
  manager->setFilters( filters );

  endMoveRows();
}

bool FilterModel::insertRows( int row, int count, const QModelIndex &parent )
{
  beginInsertRows( parent, row, row + count - 1 );
  for ( int i = 0; i < count; ++i ) {
    MailCommon::MailFilter *filter = new MailCommon::MailFilter();
    FilterIf->filterManager()->appendFilters( QList<MailCommon::MailFilter*> () << filter );
  }
  endInsertRows();

  return true;
}

bool FilterModel::removeRows( int row, int count, const QModelIndex &parent )
{
  const QList<MailCommon::MailFilter*> filters = FilterIf->filterManager()->filters();

  beginRemoveRows( parent, row, row + count - 1 );
  for ( int i = 0; i < count; ++i ) {
    MailCommon::MailFilter *filter = filters.at( row );
    FilterIf->filterManager()->removeFilter( filter );
    delete filter;
  }
  endRemoveRows();

  return true;
}

void FilterModel::filterListUpdated()
{
  reset();
}

#include "filtermodel.moc"
