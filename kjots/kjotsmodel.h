/*
    This file is part of KJots.

    Copyright (c) 2009 Stephen Kelly <steveire@gmail.com>

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

#ifndef KJOTSMODEL_H
#define KJOTSMODEL_H

#include <akonadi/entitytreemodel.h>

class QTextDocument;

namespace Akonadi
{
class ChangeRecorder;
}

using namespace Akonadi;

/**
 * A wrapper QObject making some book and page properties available to Grantlee.
 */
class KJotsEntity : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QString title READ title)
  Q_PROPERTY(QString content READ content)
  Q_PROPERTY(QString plainContent READ plainContent)
  Q_PROPERTY(qint64 entityId READ entityId)
  Q_PROPERTY(bool isBook READ isBook)
  Q_PROPERTY(bool isPage READ isPage)
  Q_PROPERTY(QVariantList entities READ entities)
  Q_PROPERTY(QVariantList breadcrumbs READ breadcrumbs)

public:
  explicit KJotsEntity( const QModelIndex &index, QObject *parent = 0 );
  void setIndex( const QModelIndex &index );

  bool isBook() const;
  bool isPage() const;

  QString title() const;

  QString content() const;

  QString plainContent() const;

  qint64 entityId() const;

  QVariantList entities() const;

  QVariantList breadcrumbs() const;

private:
  QPersistentModelIndex m_index;
};

class KJotsModel : public EntityTreeModel
{
  Q_OBJECT
public:
  explicit KJotsModel( ChangeRecorder *monitor, QObject *parent = 0 );
  virtual ~KJotsModel();

  enum KJotsRoles
  {
    GrantleeObjectRole = EntityTreeModel::UserRole,
    DocumentRole,
    DocumentCursorPositionRole
  };

  // We don't reimplement the Collection overload.
  using EntityTreeModel::entityData;
  virtual QVariant entityData( const Akonadi::Item& item, int column, int role = Qt::DisplayRole ) const;

  QVariant data( const QModelIndex &index, int role ) const;

  virtual bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole );


private:
  QHash<Entity::Id, QColor> m_colors;
  mutable QHash<Item::Id, QTextDocument *> m_documents;
  QHash<Item::Id, int> m_cursorPositions;

};

#endif

