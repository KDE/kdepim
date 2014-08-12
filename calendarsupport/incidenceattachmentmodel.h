/*
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    Author: Stephen Kelly <stephen@kdab.com>

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

#ifndef CALENDARSUPPORT_INCIDENCEATTACHMENTMODEL_H
#define CALENDARSUPPORT_INCIDENCEATTACHMENTMODEL_H

#include "calendarsupport_export.h"

#include <Attribute>
#include <Item>

#include <KCalCore/Incidence>

#include <QtCore/QAbstractListModel>


namespace Akonadi {
  class Item;
}

namespace CalendarSupport {
class IncidenceAttachmentModelPrivate;

class CALENDARSUPPORT_EXPORT IncidenceAttachmentModel : public QAbstractListModel
{
  Q_OBJECT
  Q_PROPERTY( int attachmentCount READ rowCount NOTIFY rowCountChanged )

  public:
    enum Roles {
      AttachmentDataRole = Qt::UserRole,
      MimeTypeRole,
      AttachmentUrl,
      AttachmentCountRole,

      UserRole = Qt::UserRole + 100
    };

    explicit IncidenceAttachmentModel( const QPersistentModelIndex &modelIndex,
                                       QObject *parent = 0 );

    explicit IncidenceAttachmentModel( const Akonadi::Item &item, QObject *parent = 0 );

    explicit IncidenceAttachmentModel( QObject *parent = 0 );

    ~IncidenceAttachmentModel();

    KCalCore::Incidence::Ptr incidence() const;

    void setItem( const Akonadi::Item &item );
    void setIndex( const QPersistentModelIndex &modelIndex );

    /** @reimp */
    int rowCount( const QModelIndex &parent = QModelIndex() ) const;

    /** @reimp */
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;

    /** @reimp */
    QVariant headerData( int section, Qt::Orientation orientation,
                         int role = Qt::DisplayRole ) const;

  signals:
    void rowCountChanged();

  private:
    Q_DECLARE_PRIVATE( IncidenceAttachmentModel )
    IncidenceAttachmentModelPrivate *const d_ptr;

    Q_PRIVATE_SLOT( d_func(), void resetModel() )
    Q_PRIVATE_SLOT( d_func(), void itemFetched(Akonadi::Item::List) )
};

}

#endif
