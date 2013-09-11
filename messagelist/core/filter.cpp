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

#include <Nepomuk2/Query/AndTerm>
#include <Nepomuk2/Query/ComparisonTerm>
#include <Nepomuk2/Query/LiteralTerm>
#include <Nepomuk2/Query/QueryServiceClient>
#include <Nepomuk2/Query/ResourceTerm>
#include <Nepomuk2/Vocabulary/NIE>

#include <ontologies/nie.h>
#include <ontologies/nmo.h>

#include <akonadi/itemsearchjob.h>

using namespace MessageList::Core;

Filter::Filter()
  : mQueryClient( new Nepomuk2::Query::QueryServiceClient( this ) )
{
  connect( mQueryClient, SIGNAL(newEntries(QList<Nepomuk2::Query::Result>)),
           this, SLOT(newEntries(QList<Nepomuk2::Query::Result>)) );
  connect( mQueryClient, SIGNAL(finishedListing()),
           this, SLOT(finishedListing()) );
}

bool Filter::containString(const QString& searchInString) const
{
  bool found = false;
  Q_FOREACH(const QString& str, mSearchList) {
    if(searchInString.contains(str,Qt::CaseInsensitive)) {
      found = true;
    } else {
      found = false;
      break;
    }
  }
  return found;
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

    bool searchMatches = false;
    if ( containString(item->subject()) )
      searchMatches = true;
    else if ( containString(item->sender()) )
      searchMatches = true;
    else if ( containString(item->receiver()) )
      searchMatches = true;

    if ( !searchMatches )
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
  mQueryClient->close();
  mSearchList.clear();
}

void Filter::setCurrentFolder( const KUrl &url )
{
  mCurrentFolder = url;
}

void Filter::setSearchString( const QString &search )
{
  mSearchString = search;
  mSearchList = mSearchString.trimmed().split(QLatin1Char(' '));

  emit finished(); // let the view update according to restrictions

  if( mSearchString.isEmpty()) {
    mQueryClient->close();
    return;
  }
  if (mSearchString.count()<4) {
      mQueryClient->close();
      return;
  }
  const Nepomuk2::Resource parentResource( mCurrentFolder );
  if( !parentResource.exists() ) {
     mQueryClient->close();
     return;
  }
  const Nepomuk2::Query::ComparisonTerm isChildTerm( Nepomuk2::Vocabulary::NIE::isPartOf(), Nepomuk2::Query::ResourceTerm( parentResource ) );

  const Nepomuk2::Query::ComparisonTerm bodyTerm(
      Vocabulary::NMO::plainTextMessageContent(),
      Nepomuk2::Query::LiteralTerm( QString::fromLatin1( "\'%1*\'" ).arg( mSearchString ) ),
      Nepomuk2::Query::ComparisonTerm::Contains );

  const Nepomuk2::Query::AndTerm andTerm( isChildTerm, bodyTerm );

  Nepomuk2::Query::Query query( andTerm );
  query.setRequestProperties( QList<Nepomuk2::Query::Query::RequestProperty>() << Nepomuk2::Types::Property( Akonadi::ItemSearchJob::akonadiItemIdUri() ) );

  mMatchingItemIds.clear();
  mQueryClient->close();
  bool ok = mQueryClient->query( query );
  if (!ok) {
    kDebug() << "Cannot start query:" << mQueryClient->errorMessage();
  }
}

void Filter::newEntries( const QList<Nepomuk2::Query::Result> &entries )
{
  Q_FOREACH( const Nepomuk2::Query::Result &result, entries ) {
    const Soprano::Node &property = result.requestProperty( Akonadi::ItemSearchJob::akonadiItemIdUri() );
    if ( !(property.isValid() && property.isLiteral() && property.literal().isString()) ) {
      continue;
    } else {
      mMatchingItemIds.insert( property.literal().toString().toLongLong() );
    }
  }
}

void Filter::finishedListing()
{
  emit finished(); // let the view update according to restrictions _and_ matching full-text search results
}

#include "filter.moc"
