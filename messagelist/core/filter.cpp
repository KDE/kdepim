/******************************************************************************
 *
 *  Copyright 2008 Szymon Tomasz Stefanek <pragma@kvirc.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *******************************************************************************/

#include "core/filter.h"
#include "core/messageitem.h"

#include <akonadi/itemsearchjob.h>
#include <baloo/pim/emailquery.h>
#include <baloo/pim/resultiterator.h>

using namespace MessageList::Core;

Filter::Filter()
{
}

bool Filter::match( const MessageItem * item ) const
{
  if ( !mStatus.isOfUnknownStatus() )
  {
    if ( !(mStatus & item->status()) )
      return false;
  }

  if ( !mSearchString.isEmpty() )
  {
    if ( mMatchingItemIds.contains( item->itemId() ) )
      return true;
    else
      return false;
  }

  if ( !mTagId.isEmpty() ) {
    const bool tagMatches = item->findTag( mTagId ) != 0;
    if ( !tagMatches )
      return false;
  }

  return true;
}

bool Filter::isEmpty() const
{
  if ( !mStatus.isOfUnknownStatus() )
    return false;

  if ( !mSearchString.isEmpty() )
    return false;

  if ( !mTagId.isEmpty() )
    return false;

  return true;
}

void Filter::clear()
{
  mStatus = Akonadi::MessageStatus();
  mSearchString.clear();
  mTagId.clear();
  mMatchingItemIds.clear();
}

void Filter::setCurrentFolder( const KUrl &url )
{
  mCurrentFolder = url;
}

void Filter::setSearchString( const QString &search )
{
  const QString trimStr = search.trimmed();
  if (mSearchString == trimStr) {
    return;
  }

  mSearchString = trimStr;
  mMatchingItemIds.clear();

  if (mSearchString.isEmpty()) {
    return;
  }

  Baloo::PIM::EmailQuery query;
  query.matches(mSearchString);

  Akonadi::Collection col = Akonadi::Collection::fromUrl(mCurrentFolder);
  if (col.isValid()) {
    query.addCollection(col.id());
  }

  Baloo::PIM::ResultIterator it = query.exec();
  while (it.next())
    mMatchingItemIds << it.id();

  emit finished();
}


#include "moc_filter.cpp"
