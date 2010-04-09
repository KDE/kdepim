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
#include <QStringList>

AttachmentProxyModel::AttachmentProxyModel(QObject* parent): QSortFilterProxyModel(parent)
{
  // moc doesn't allow property NOTIFY to use signals in the base class apparently...
  connect( this, SIGNAL(modelReset()), SIGNAL(rowCountChanged()) );
}

bool AttachmentProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
  const QModelIndex sourceIndex = sourceModel()->index( source_row, 0, source_parent );
  const QString mimeType = sourceIndex.data( MessageViewer::MimeTreeModel::MimeTypeRole ).toString();

  // filter out structutral nodes and crypto stuff
  const QStringList blacklist = QStringList()
    << QLatin1String( "application/pgp-encrypted" )
    << QLatin1String( "application/pkcs7-mime" )
    << QLatin1String( "application/pgp-signature" )
    << QLatin1String( "application/pkcs7-signature" )
    << QLatin1String( "application/x-pkcs7-signature" );
  if ( mimeType.startsWith( QLatin1String( "multipart/" ) ) || blacklist.contains( mimeType ) )
    return false;

  // filter out the main body part
  if ( sourceIndex.data( MessageViewer::MimeTreeModel::MainBodyPartRole ).toBool() )
    return false;

  return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

#include "attachmentproxymodel.moc"