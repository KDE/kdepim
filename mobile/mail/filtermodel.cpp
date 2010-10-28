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

FilterModel::FilterModel(QObject* parent): QAbstractListModel(parent)
{

}

QVariant FilterModel::data(const QModelIndex& index, int role) const
{
  if ( role == Qt::DisplayRole ) {
    QString name = FilterIf->filterManager()->filters().at( index.row() )->name();
    return QVariant( name );
  } 

  return QVariant();
}

int FilterModel::rowCount(const QModelIndex& parent) const
{
  return FilterIf->filterManager()->filters().size();
}

#include "filtermodel.moc"