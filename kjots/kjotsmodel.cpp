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

#include <QTextDocument>

#include <KIcon>

#include <akonadi/changerecorder.h>
#include <akonadi/entitydisplayattribute.h>

#include "akonadi_next/note.h"

#include <kdebug.h>
#include <KMime/KMimeMessage>

#include <kpimtextedit/textutils.h>

#include <grantlee/markupdirector.h>
#include <grantlee/texthtmlbuilder.h>
#include <grantlee/plaintextmarkupbuilder.h>
#include "noteshared/attributes/notelockattribute.h"

Q_DECLARE_METATYPE(QTextDocument*)
KJotsEntity::KJotsEntity(const QModelIndex &index, QObject *parent)
  : QObject(parent)
{
  m_index = QPersistentModelIndex(index);
}

void KJotsEntity::setIndex(const QModelIndex &index)
{
  m_index = QPersistentModelIndex(index);
}

QString KJotsEntity::title() const
{
  return m_index.data().toString();
}

QString KJotsEntity::content() const
{
  QTextDocument *document = m_index.data( KJotsModel::DocumentRole ).value<QTextDocument*>();
  if (!document)
    return QString();

  Grantlee::TextHTMLBuilder builder;
  Grantlee::MarkupDirector director(&builder);

  director.processDocument(document);
  QString result = builder.getResult();

  return result;
}

QString KJotsEntity::plainContent() const
{
  QTextDocument *document = m_index.data( KJotsModel::DocumentRole ).value<QTextDocument*>();
  if (!document)
    return QString();

  Grantlee::PlainTextMarkupBuilder builder;
  Grantlee::MarkupDirector director(&builder);

  director.processDocument(document);
  QString result = builder.getResult();

  return result;
}

qint64 KJotsEntity::entityId() const
{
  Item item = m_index.data(EntityTreeModel::ItemRole).value<Item>();
  if (!item.isValid())
  {
    Collection col = m_index.data(EntityTreeModel::CollectionRole).value<Collection>();
    if ( !col.isValid() )
      return -1;
    return col.id();
  }
  return item.id();
}

bool KJotsEntity::isBook() const
{
  Collection col = m_index.data(EntityTreeModel::CollectionRole).value<Collection>();

  if (col.isValid())
  {
    return col.contentMimeTypes().contains( Akonotes::Note::mimeType() );
  }
  return false;
}

bool KJotsEntity::isPage() const
{
  Item item = m_index.data(EntityTreeModel::ItemRole).value<Item>();
  if (item.isValid())
  {
    return item.hasPayload<KMime::Message::Ptr>();
  }
  return false;
}

QVariantList KJotsEntity::entities() const
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

QVariantList KJotsEntity::breadcrumbs() const
{
  QVariantList list;
  int row = 0;
  const int column = 0;
  QModelIndex parent = m_index.parent();

  while ( parent.isValid() )
  {
    QObject *obj = new KJotsEntity(parent);
    list << QVariant::fromValue(obj);
    parent = parent.parent();
  }
  return list;
}

KJotsModel::KJotsModel( ChangeRecorder *monitor, QObject *parent )
  : EntityTreeModel( monitor, parent )
{

}

KJotsModel::~KJotsModel()
{
  qDeleteAll( m_documents );
}

bool KJotsModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
  if ( role == Qt::EditRole )
  {
    Item item = index.data( ItemRole ).value<Item>();

    if ( !item.isValid() )
    {
      Collection col = index.data( CollectionRole ).value<Collection>();
      col.setName( value.toString() );
      if (col.hasAttribute<EntityDisplayAttribute>())
      {
        EntityDisplayAttribute *eda = col.attribute<EntityDisplayAttribute>();
        eda->setDisplayName(value.toString());
      }
      return EntityTreeModel::setData(index, QVariant::fromValue( col ), CollectionRole );
    }
    KMime::Message::Ptr m = item.payload<KMime::Message::Ptr>();

    m->subject( true )->fromUnicodeString( value.toString(), "utf-8" );
    m->assemble();
    item.setPayload<KMime::Message::Ptr>( m );

    if ( item.hasAttribute<EntityDisplayAttribute>() ) {
      EntityDisplayAttribute *displayAttribute = item.attribute<EntityDisplayAttribute>();
      displayAttribute->setDisplayName( value.toString() );
    }
    return EntityTreeModel::setData(index, QVariant::fromValue<Item>( item ), ItemRole);
  }

  if ( role == KJotsModel::DocumentRole )
  {
    Item item = EntityTreeModel::data( index, ItemRole ).value<Item>();
    if ( !item.hasPayload<KMime::Message::Ptr>() )
      return false;
    KMime::Message::Ptr note = item.payload<KMime::Message::Ptr>();
    QTextDocument *document = value.value<QTextDocument*>();

    bool isRichText = KPIMTextEdit::TextUtils::containsFormatting( document );

    note->contentType()->setMimeType( isRichText ? "text/html" : "text/plain" );
    note->contentType()->setCharset("utf-8");
    note->contentTransferEncoding(true)->setEncoding(KMime::Headers::CEquPr);
    note->mainBodyPart()->fromUnicodeString( isRichText ? document->toHtml() : document->toPlainText() );
    note->assemble();
    item.setPayload<KMime::Message::Ptr>( note );
    return EntityTreeModel::setData(index, QVariant::fromValue<Item>( item ), ItemRole );
  }

  if ( role == KJotsModel::DocumentCursorPositionRole )
  {
    Item item = index.data( ItemRole ).value<Item>();
    m_cursorPositions.insert( item.id(), value.toInt() );
    return true;
  }

  return EntityTreeModel::setData(index, value, role);
}

QVariant KJotsModel::data( const QModelIndex &index, int role ) const
{
  if (GrantleeObjectRole == role)
  {
    QObject *obj = new KJotsEntity(index);
    return QVariant::fromValue(obj);
  }


  if ( role == KJotsModel::DocumentRole )
  {
    const Item item = index.data( ItemRole ).value<Item>();
    Entity::Id itemId = item.id();
    if ( m_documents.contains( itemId ) )
      return QVariant::fromValue( m_documents.value( itemId ) );
    if ( !item.hasPayload<KMime::Message::Ptr>() )
      return QVariant();

    KMime::Message::Ptr note = item.payload<KMime::Message::Ptr>();
    QTextDocument *document = new QTextDocument;
    if ( note->contentType()->isHTMLText() )
      document->setHtml( note->mainBodyPart()->decodedText() );
    else
      document->setPlainText( note->mainBodyPart()->decodedText() );

    m_documents.insert( itemId, document );
    return QVariant::fromValue( document );
  }

  if ( role == KJotsModel::DocumentCursorPositionRole )
  {
    const Item item = index.data( ItemRole ).value<Item>();
    if (!item.isValid())
      return 0;

    if ( m_cursorPositions.contains( item.id() ) )
      return m_cursorPositions.value( item.id() );

    return 0;
  }

  if ( role == Qt::DecorationRole )
  {
    const Item item = index.data( ItemRole ).value<Item>();
    if ( item.isValid() && item.hasAttribute<NoteShared::NoteLockAttribute>() ) {
        return KIcon( QLatin1String("emblem-locked") );
    } else {
      const Collection col = index.data( CollectionRole ).value<Collection>();
      if ( col.isValid() && col.hasAttribute<NoteShared::NoteLockAttribute>() ) {
        return KIcon(QLatin1String( "emblem-locked") );
      }
    }
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

