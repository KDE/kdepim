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

#ifndef MESSAGEVIEWER_ATTACHMENTPROXYMODEL_H
#define MESSAGEVIEWER_ATTACHMENTPROXYMODEL_H

#include <messageviewer/viewer/mimetreemodel.h>

#include <QSortFilterProxyModel>

namespace MessageViewer {
  class NodeHelper;
}

/**
 * @short A proxy model to provide roles for accessing attachment properties from QML.
 */
class AttachmentProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT
  Q_PROPERTY( int attachmentCount READ rowCount NOTIFY rowCountChanged )

  public:
    enum Role {
      AttachmentUrlRole = MessageViewer::MimeTreeModel::UserRole
    };

    explicit AttachmentProxyModel( QObject *parent = 0 );
    ~AttachmentProxyModel();

    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const;
    void setSourceModel( QAbstractItemModel *sourceModel );
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;

  Q_SIGNALS:
    void rowCountChanged();

  private Q_SLOTS:
    void slotModelReset();

  private:
    MessageViewer::NodeHelper *m_nodeHelper;
};

#endif
