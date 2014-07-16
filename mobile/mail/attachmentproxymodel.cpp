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

#include <messageviewer/viewer/nodehelper.h>

#include <QtCore/QStringList>

Q_DECLARE_METATYPE( KMime::Content* )

AttachmentProxyModel::AttachmentProxyModel( QObject* parent )
  : QSortFilterProxyModel( parent ),
    m_nodeHelper( new MessageViewer::NodeHelper )
{
  connect( this, SIGNAL(modelReset()), SLOT(slotModelReset()) );
}

AttachmentProxyModel::~AttachmentProxyModel()
{
  delete m_nodeHelper;
}

bool AttachmentProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const
{
  const QModelIndex sourceIndex = sourceModel()->index( sourceRow, 0, sourceParent );
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
  if ( sourceIndex.data( MessageViewer::MimeTreeModel::MainBodyPartRole ).toBool()
    || sourceIndex.data( MessageViewer::MimeTreeModel::AlternativeBodyPartRole ).toBool() ) {
    return false;
  }

  return QSortFilterProxyModel::filterAcceptsRow( sourceRow, sourceParent );
}

void AttachmentProxyModel::setSourceModel( QAbstractItemModel *sourceModel )
{
  QSortFilterProxyModel::setSourceModel( sourceModel );

  QHash<int, QByteArray> names = roleNames();
  names.insert( MessageViewer::MimeTreeModel::MimeTypeRole, "mimeType" );
  names.insert( AttachmentProxyModel::AttachmentUrlRole, "attachmentUrl" );
  setRoleNames( names );
}

QVariant AttachmentProxyModel::data( const QModelIndex &index, int role ) const
{
  if ( role == AttachmentUrlRole ) {
    KMime::Content *content = index.data( MessageViewer::MimeTreeModel::ContentRole ).value<KMime::Content*>();
    return m_nodeHelper->writeNodeToTempFile( content );
  }

  return QSortFilterProxyModel::data( index, role );
}

void AttachmentProxyModel::slotModelReset()
{
  m_nodeHelper->removeTempFiles();
  m_nodeHelper->clear();
  // moc doesn't allow property NOTIFY to use signals in the base class apparently...
  emit rowCountChanged();
}

