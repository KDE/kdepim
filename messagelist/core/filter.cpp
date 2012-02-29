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

#include <KDE/Nepomuk/Query/AndTerm>
#include <KDE/Nepomuk/Query/ComparisonTerm>
#include <KDE/Nepomuk/Query/LiteralTerm>
#include <KDE/Nepomuk/Query/QueryServiceClient>
#include <KDE/Nepomuk/Query/ResourceTerm>
#include <KDE/Nepomuk/Vocabulary/NIE>

#include <ontologies/nie.h>
#include <ontologies/nmo.h>

using namespace MessageList::Core;

Filter::Filter()
  : mQueryClient( new Nepomuk::Query::QueryServiceClient( this ) )
{
  connect( mQueryClient, SIGNAL(newEntries(QList<Nepomuk::Query::Result>)),
           this, SLOT(newEntries(QList<Nepomuk::Query::Result>)) );
  connect( mQueryClient, SIGNAL(finishedListing()),
           this, SLOT(finishedListing()) );
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
    if ( item->subject().indexOf( mSearchString, 0, Qt::CaseInsensitive ) >= 0 )
      searchMatches = true;
    else if ( item->sender().indexOf( mSearchString, 0, Qt::CaseInsensitive ) >= 0 )
      searchMatches = true;
    else if ( item->receiver().indexOf( mSearchString, 0, Qt::CaseInsensitive ) >= 0 )
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
}

void Filter::setCurrentFolder( const KUrl &url )
{
  mCurrentFolder = url;
}

void Filter::setSearchString( const QString &search )
{
  mSearchString = search;

  emit finished(); // let the view update according to restrictions

  if( mSearchString.isEmpty()) {
    mQueryClient->close();
    return;
  }
  const Nepomuk::Resource parentResource( mCurrentFolder );

  const Nepomuk::Query::ComparisonTerm isChildTerm( Nepomuk::Vocabulary::NIE::isPartOf(), Nepomuk::Query::ResourceTerm( parentResource ) );

  const Nepomuk::Query::ComparisonTerm bodyTerm(
      Vocabulary::NMO::plainTextMessageContent(),
      Nepomuk::Query::LiteralTerm( QString::fromLatin1( "\'%1\'" ).arg( mSearchString ) ),
      Nepomuk::Query::ComparisonTerm::Contains );

  const Nepomuk::Query::AndTerm andTerm( isChildTerm, bodyTerm );

  Nepomuk::Query::Query query( andTerm );
  query.setRequestProperties( QList<Nepomuk::Query::Query::RequestProperty>() << Nepomuk::Types::Property( QUrl( QLatin1String( "http://akonadi-project.org/ontologies/aneo#akonadiItemId" ) ) ) );

  mMatchingItemIds.clear();
  mQueryClient->close();
  bool ok = mQueryClient->query( query );
  if (!ok) {
    qDebug() << "Cannot start query:" << mQueryClient->errorMessage();
  }
}

void Filter::newEntries( const QList<Nepomuk::Query::Result> &entries )
{
  Q_FOREACH( const Nepomuk::Query::Result &result, entries ) {
    const Soprano::Node &property = result.requestProperty( QUrl( QLatin1String( "http://akonadi-project.org/ontologies/aneo#akonadiItemId" ) ) );
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
