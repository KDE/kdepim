/*
    Copyright (c) 2010 Tobias Koenig <tokoe@kde.org>

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

#include "emailsfilterproxymodel.h"

#include <AkonadiCore/entitytreemodel.h>
#include <KMime/Message>

static bool emailMatchesFilter( const KMime::Message::Ptr &message, const QString &filterString );

using namespace Akonadi;

class EmailsFilterProxyModel::Private
{
  public:
    QString mFilter;
};

EmailsFilterProxyModel::EmailsFilterProxyModel( QObject *parent )
  : QSortFilterProxyModel( parent ), d( new Private )
{
  setSortLocaleAware( true );
  setDynamicSortFilter( true );
}

EmailsFilterProxyModel::~EmailsFilterProxyModel()
{
  delete d;
}

void EmailsFilterProxyModel::setFilterString( const QString &filter )
{
  d->mFilter = filter;
  invalidateFilter();
}

bool EmailsFilterProxyModel::filterAcceptsRow( int row, const QModelIndex &parent ) const
{
  if ( d->mFilter.isEmpty() )
    return true;

  const QModelIndex index = sourceModel()->index( row, 0, parent );

  const Akonadi::Item item = index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();

  if ( item.hasPayload<KMime::Message::Ptr>() ) {
    const KMime::Message::Ptr message = item.payload<KMime::Message::Ptr>();
    return emailMatchesFilter( message, d->mFilter );
  }

  return true;
}

static bool emailMatchesFilter( const KMime::Message::Ptr &message, const QString &filterString )
{
  if ( message->subject()->asUnicodeString().contains( filterString, Qt::CaseInsensitive ) )
    return true;

  foreach ( const KMime::Types::Mailbox &mailbox, message->from()->mailboxes() ) {
    if ( mailbox.hasName() ) {
      if ( mailbox.name().contains( filterString, Qt::CaseInsensitive ) )
        return true;
    } else {
      if ( mailbox.addrSpec().asPrettyString().contains( filterString, Qt::CaseInsensitive ) )
        return true;
    }
  }

  return false;
}

