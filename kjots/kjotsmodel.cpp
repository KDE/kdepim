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

#include "kjotsmodel.h"

#include <QColor>

#include <akonadi/changerecorder.h>
#include <akonadi/entitydisplayattribute.h>

#include <kdebug.h>
#include <KMime/KMimeMessage>

#include "note.h"

KJotsEntity::KJotsEntity(const QModelIndex &index, QObject *parent)
  : QObject(parent)
{
  m_index = QPersistentModelIndex(index);
}

QString KJotsEntity::title()
{
  Item item = m_index.data(EntityTreeModel::ItemRole).value<Item>();
  if (item.isValid())
  {
    KMime::Message::Ptr page = item.payload<KMime::Message::Ptr>();
    return page->subject()->asUnicodeString();
  } else {
    Collection col = m_index.data(EntityTreeModel::CollectionRole).value<Collection>();
    if (col.isValid())
    {
      return col.name();
    }
  }
  return QString();
}

QString KJotsEntity::content()
{
  Item item = m_index.data(EntityTreeModel::ItemRole).value<Item>();
  if (item.isValid())
  {
    KMime::Message::Ptr page = item.payload<KMime::Message::Ptr>();
    return page->mainBodyPart()->decodedText();
  }
  return QString();
}

bool KJotsEntity::isBook()
{
  Collection col = m_index.data(EntityTreeModel::CollectionRole).value<Collection>();

  if (col.isValid())
  {
    return col.contentMimeTypes().contains( Note::mimeType() );
  }
  return false;
}

bool KJotsEntity::isPage()
{
  Item item = m_index.data(EntityTreeModel::ItemRole).value<Item>();
  if (item.isValid())
  {
    return item.hasPayload<KMime::Message::Ptr>();
  }
  return false;
}

QVariantList KJotsEntity::entities()
{
  QVariantList list;
  int row = 0;
  const int column = 0;
  QModelIndex childIndex = m_index.child(row++, column);
  while (childIndex.isValid())
  {
    QObject *obj = new KJotsEntity(childIndex);
    list << QVariant::fromValue(obj);
    childIndex = m_index.child(row++, column);
  }
  return list;
}

KJotsModel::KJotsModel(Session *session, ChangeRecorder *monitor, QObject *parent)
  : EntityTreeModel(session, monitor, parent)
{

}

KJotsModel::~KJotsModel()
{

}


bool KJotsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if ( role == Qt::EditRole )
  {
    Item item = index.data( ItemRole ).value<Item>();

    if ( !item.isValid() )
    {
      Collection col = index.data( CollectionRole ).value<Collection>();
      col.setName( value.toString() );
      return EntityTreeModel::setData(index, QVariant::fromValue( col ), CollectionRole );
    }
    KMime::Message::Ptr m = item.payload<KMime::Message::Ptr>();

    m->subject( true )->fromUnicodeString( value.toString(), "utf-8" );
    m->assemble();
    item.setPayload<KMime::Message::Ptr>( m );

    if ( item.hasAttribute<EntityDisplayAttribute>() ) {
      EntityDisplayAttribute *displayAttribute = item.attribute<EntityDisplayAttribute>( Entity::AddIfMissing );
      displayAttribute->setDisplayName( value.toString() );
      item.addAttribute( displayAttribute );
    }
    return EntityTreeModel::setData(index, QVariant::fromValue<Item>( item ), ItemRole);
  }

  if ( role == Qt::BackgroundColorRole )
  {
    // Set as attribute instead.

    Item item = index.data( ItemRole ).value<Item>();

    if ( item.isValid() )
    {
      m_colors[ item.id() ] = value.value<QColor>();
      return true;
    }
  }

  if ( role == KJotsModel::DocumentCursorPositionRole )
  {
    Item item = index.data( ItemRole ).value<Item>();
    m_cursorPositions.insert( item.id(), value.toInt() );
  }

  return EntityTreeModel::setData(index, value, role);
}

QVariant KJotsModel::data(const QModelIndex &index, int role) const
{
  if (GrantleeObjectRole == role)
  {
    QObject *obj = new KJotsEntity(index);
    return QVariant::fromValue(obj);
  }

  if ( role == Qt::BackgroundColorRole )
  {
    Item item = EntityTreeModel::data(index, ItemRole).value<Item>();
    if ( item.isValid() && m_colors.contains( item.id() ) )
      return m_colors.value( item.id() );
  }

  if ( role == KJotsModel::DocumentCursorPositionRole )
  {
    Item item = EntityTreeModel::data( index, EntityTreeModel::ItemRole ).value<Item>();
    if (!item.isValid())
      return 0;

    if ( m_cursorPositions.contains( item.id() ) )
      return m_cursorPositions.value( item.id() );

    return 0;
  }

  return EntityTreeModel::data(index, role);
}

QVariant KJotsModel::entityData( const Akonadi::Item& item, int column, int role) const
{
  if ( ( role == Qt::EditRole || role == Qt::DisplayRole ) && item.hasPayload<KMime::Message::Ptr>() )
  {
    KMime::Message::Ptr page = item.payload<KMime::Message::Ptr>();
    return page->subject()->asUnicodeString();
  }
  return EntityTreeModel::entityData( item, column, role );
}

#include "kjotsmodel.moc"
