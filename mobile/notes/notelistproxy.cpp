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

#include "notelistproxy.h"

#include <QTextDocument>

#include <AkonadiCore/entitytreemodel.h>
#include <KMime/KMimeMessage>

using namespace Akonadi;

NoteListProxy::NoteListProxy( int customRoleBaseline, QObject* parent )
  : ListProxy( parent ),
    mCustomRoleBaseline( customRoleBaseline )
{ }

QVariant NoteListProxy::data( const QModelIndex& index, int role ) const
{
  const Akonadi::Item item = QSortFilterProxyModel::data( index, Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();

  if ( item.isValid() && item.hasPayload<KMime::Message::Ptr>() ) {
    const KMime::Message::Ptr note = item.payload<KMime::Message::Ptr>();
    switch ( relativeCustomRole( role ) ) {
    case Title:
      return note->subject()->asUnicodeString();
    case Content:
      return note->mainBodyPart()->decodedText();
    case PlainContent:
    {
      QTextDocument doc;
      if ( note->contentType()->asUnicodeString() == QLatin1String( "text/plain" ) )
        doc.setPlainText( note->mainBodyPart()->decodedText() );
      else
        doc.setHtml( note->mainBodyPart()->decodedText() );
      return doc.toPlainText();
    }
    case ShortContent:
    {
      QTextDocument doc;
      if ( note->contentType()->asUnicodeString() == QLatin1String( "text/plain" ) )
        doc.setPlainText( note->mainBodyPart()->decodedText() );
      else
        doc.setHtml( note->mainBodyPart()->decodedText() );
      const QString plain = doc.toPlainText();
      const QStringList list = plain.split( QLatin1Char( '\n' ) );
      if ( list.isEmpty() )
        return QString();
      return list.first();
    }
    }
  }

  return QSortFilterProxyModel::data(index, role);
}

void NoteListProxy::setSourceModel( QAbstractItemModel* sourceModel )
{
  ListProxy::setSourceModel(sourceModel);

  QHash<int, QByteArray> names = roleNames();
  names.insert( EntityTreeModel::ItemIdRole, "itemId" );
  names.insert( absoluteCustomRole( Title ), "title" );
  names.insert( absoluteCustomRole( Content ), "content" );
  names.insert( absoluteCustomRole( PlainContent ), "plainContent" );
  names.insert( absoluteCustomRole( ShortContent ), "shortContent" );
  setRoleNames( names );
}

