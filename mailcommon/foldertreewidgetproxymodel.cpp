/*
    Copyright (c) 2011 Laurent Montel <montel@kde.org>


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
#include "foldertreewidgetproxymodel.h"

using namespace MailCommon;

FolderTreeWidgetProxyModel::FolderTreeWidgetProxyModel( QObject* parent )
  :KRecursiveFilterProxyModel( parent )
{
}

FolderTreeWidgetProxyModel::~FolderTreeWidgetProxyModel()
{
}

Qt::ItemFlags FolderTreeWidgetProxyModel::flags( const QModelIndex & index ) const
{
  if ( m_filterStr.isEmpty() )
    return KRecursiveFilterProxyModel::flags( index );
  else {
    if ( !index.data().toString().contains( m_filterStr, Qt::CaseInsensitive ) )
      return KRecursiveFilterProxyModel::flags( index ) & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    return KRecursiveFilterProxyModel::flags( index );
  }
}

void FolderTreeWidgetProxyModel::setFilterFolder( const QString& filter )
{
  m_filterStr = filter;
  setFilterWildcard( filter );
}


#include "foldertreewidgetproxymodel.moc"
