/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Stephen Kelly <stephen@kdab.com>

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

#ifndef QMLCHECKABLEPROXYMODEL_H
#define QMLCHECKABLEPROXYMODEL_H

#include <kcheckableproxymodel.h>

class QMLCheckableItemProxyModel : public KCheckableProxyModel
{
public:
  enum MoreRoles {
    CheckOn = Qt::UserRole + 3000
  };
  explicit QMLCheckableItemProxyModel (QObject* parent = 0)
    : KCheckableProxyModel(parent)
  {
  }

  virtual void setSourceModel(QAbstractItemModel* sourceModel)
  {
    KCheckableProxyModel::setSourceModel(sourceModel);

    QHash<int, QByteArray> roles = roleNames();
    roles.insert( CheckOn, "checkOn" );
    setRoleNames(roles);
  }

  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const
  {
    if ( role == CheckOn )
      return (index.data(Qt::CheckStateRole) == Qt::Checked);
    return KCheckableProxyModel::data(index, role);
  }

};

#endif
