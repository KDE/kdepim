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


#include "filtermodel_p.h"

#include "filtermanager.h"
#include "mailfilter.h"
#include "mailkernel.h"

using namespace MailCommon;

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
  qDebug() << "FilterModel::moveRow(from=" << sourceRow << " to=" << destinationRow << ")";
  if ( sourceRow == destinationRow )
    return;

  if ( sourceRow < 0 || sourceRow >= rowCount() )
    return;

  if ( destinationRow < 0 || destinationRow >= rowCount() )
    return;

  const int startRow = sourceRow;
  const int endRow = (sourceRow < destinationRow ? destinationRow + 1 : destinationRow - 1);

  qDebug() << "beginMoveRows(from=" << startRow << " to=" << endRow << ")";
  //if ( !beginMoveRows( QModelIndex(), startRow, startRow, QModelIndex(), endRow ) )
  //  return;

  QList<MailFilter*> filters;

  FilterManager *manager = FilterIf->filterManager();

  foreach ( MailFilter *filter, manager->filters() ) {
    filters.append( new MailFilter( *filter ) ); // deep copy
  }

  filters.move( sourceRow, destinationRow );
  manager->setFilters( filters );

  //endMoveRows();

  reset();
}

bool FilterModel::insertRows( int row, int count, const QModelIndex &parent )
{
  beginInsertRows( parent, row, row + count - 1 );
  for ( int i = 0; i < count; ++i ) {
    MailFilter *filter = new MailFilter();
    FilterIf->filterManager()->appendFilters( QList<MailFilter*> () << filter );
  }
  endInsertRows();

  return true;
}

bool FilterModel::removeRows( int row, int count, const QModelIndex &parent )
{
  const QList<MailFilter*> filters = FilterIf->filterManager()->filters();

  beginRemoveRows( parent, row, row + count - 1 );
  for ( int i = 0; i < count; ++i ) {
    MailFilter *filter = filters.at( row );
    FilterIf->filterManager()->removeFilter( filter );
    delete filter;
  }
  endRemoveRows();

  return true;
}

void FilterModel::filterListUpdated()
{
  // Since the FilterManager doesn't tell use which filter has been
  // updated, we emit dataChanged() for all of them
  emit dataChanged( index( 0, 0 ), index( rowCount() - 1, 0 ) );
}

#include "filtermodel_p.moc"
