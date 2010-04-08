/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

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

#include "attachmentproxymodel.h"
#include <messageviewer/mimetreemodel.h>
#include <KDebug>

AttachmentProxyModel::AttachmentProxyModel(QObject* parent): QSortFilterProxyModel(parent)
{
}

bool AttachmentProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
  const QModelIndex sourceTypeIndex = sourceModel()->index( source_row, 1, source_parent );
  const QString mimeType = sourceTypeIndex.data( MessageViewer::MimeTreeModel::MimeTypeRole ).toString();
  kDebug() << mimeType;
  // TODO complete blacklist, filter out main body part
  if ( mimeType.startsWith( QLatin1String( "multipart/" ) ) || mimeType == QLatin1String( "application/pgp-signature" ) )
    return false;
  return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

#include "attachmentproxymodel.moc"