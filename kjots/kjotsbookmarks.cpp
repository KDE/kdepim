//
//  kjotsbookmarks
//
//  Copyright (C) 1997 Christoph Neerfeld <Christoph.Neerfeld@home.ivm.de>
//  Copyright (C) 2002, 2003 Aaron J. Seigo <aseigo@kde.org>
//  Copyright (C) 2003 Stanislav Kljuhhin <crz@hot.ee>
//  Copyright (C) 2005-2006 Jaison Lee <lee.jaison@gmail.com>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#include "kjotsbookmarks.h"

#include <QItemSelectionModel>

#include "kjotsmodel.h"
#include "kjotstreeview.h"

KJotsBookmarks::KJotsBookmarks( KJotsTreeView *treeView ) :
    m_treeView( treeView )
{
}

KJotsBookmarks::~KJotsBookmarks()
{
}

void KJotsBookmarks::openBookmark(const KBookmark & bookmark, Qt::MouseButtons, Qt::KeyboardModifiers)
{
#if 0
  QModelIndexList rows = m_treeView->model()->match( QModelIndex(), KJotsModel::EntityUrlRole, bookmark.url().url() );

  if ( rows.isEmpty() )
    return;

  // Arbitrarily chooses the first one if multiple are returned.
  return m_treeView->selectionModel()->select( rows.at( 0 ), QItemSelectionModel::ClearAndSelect );
#endif
}

QString KJotsBookmarks::currentUrl() const
{
  QModelIndexList rows = m_treeView->selectionModel()->selectedRows();

  if ( rows.size() != 1 )
    return QString();
#if 0
  return rows.at( 0 ).data( EntityTreeModel::EntityUrlRole ).toString();
#else
  return QString();
#endif
}

QString KJotsBookmarks::currentTitle() const
{
  return m_treeView->captionForSelection( ": " );
}

#include "kjotsbookmarks.moc"
/* kate: tab-indents off; replace-tabs on; tab-width 4; remove-trailing-space on; encoding utf-8;*/
