/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>

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

#ifndef CONTACTLISTPROXY_H
#define CONTACTLISTPROXY_H

#include "listproxy.h"

#include <AkonadiCore/entitytreemodel.h>
#include <QDeclarativeImageProvider>

class ContactImageProvider : public QDeclarativeImageProvider
{
  public:
    ContactImageProvider();

    QPixmap requestPixmap( const QString &id, QSize *size, const QSize &requestedSize );

    void setModel( QAbstractItemModel *model );

  private:
    QAbstractItemModel *mModel;
};

/** Adaptor proxy for contact access from QML. */
class ContactListProxy : public ListProxy
{
  Q_OBJECT
  public:
    enum Role {
      NameRole = Akonadi::EntityTreeModel::UserRole + 1,
      PictureRole,
      TypeRole
    };

    explicit ContactListProxy( QObject* parent = 0 );

    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
    void setSourceModel(QAbstractItemModel* sourceModel);
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const;

  public slots:
    QString typeForIndex( int row ) const;
};

#endif /* CONTACTLISTPROXY_H */
