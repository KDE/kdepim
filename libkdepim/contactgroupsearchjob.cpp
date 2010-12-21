/*
    This file is part of Akonadi Contact.

    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

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

#include "contactgroupsearchjob_p.h"

#include <akonadi/itemfetchscope.h>

using namespace KPIM;

class KPIM::ContactGroupSearchJob::Private
{
  public:
    int mLimit;
};

KPIM::ContactGroupSearchJob::ContactGroupSearchJob( QObject * parent )
  : Akonadi::ItemSearchJob( QString(), parent ), d( new Private )
{
  fetchScope().fetchFullPayload();
  d->mLimit = -1;

  // by default search for all contact groups
  Akonadi::ItemSearchJob::setQuery( QLatin1String( ""
                                          "prefix nco:<http://www.semanticdesktop.org/ontologies/2007/03/22/nco#>"
                                          "SELECT ?r WHERE { ?r a nco:ContactGroup }" ) );
}

KPIM::ContactGroupSearchJob::~ContactGroupSearchJob()
{
  delete d;
}

void KPIM::ContactGroupSearchJob::setQuery( Criterion criterion, const QString &value )
{
  // Exact match was the default in 4.4, so we have to keep it and ContactSearchJob has something
  // else as default
  setQuery( criterion, value, ExactMatch );
}

void KPIM::ContactGroupSearchJob::setQuery( Criterion criterion, const QString &value, Match match )
{
  QString query = QString::fromLatin1(
            "prefix nco:<http://www.semanticdesktop.org/ontologies/2007/03/22/nco#>" );

  QString matchString;
  switch ( match ) {
    case ExactMatch:
      matchString = QString::fromLatin1(
                     " ?group nco:contactGroupName \"%1\"^^<http://www.w3.org/2001/XMLSchema#string>." );
      break;
    case ContainsMatch:
      matchString = QString::fromLatin1(
                     " ?group nco:contactGroupName ?v . "
                     " ?v bif:contains \"'%1'\"" );
      break;
    case StartsWithMatch:
      matchString = QString::fromLatin1(
                     " ?group nco:contactGroupName ?v . "
                     " ?v bif:contains \"'%1*'\"" );
      break;
  }

  if ( criterion == Name ) {
    query += QString::fromLatin1(
        "SELECT DISTINCT ?group "
        "WHERE { "
        "  graph ?g { "
        "    ?group <" + akonadiItemIdUri().toEncoded() + "> ?itemId . " );
    query += matchString;
    query += QString::fromLatin1(
        "  } "
        "}" );
  }

  if ( d->mLimit != -1 )
    query += QString::fromLatin1( " LIMIT %1" ).arg( d->mLimit );

  query = query.arg( value );

  Akonadi::ItemSearchJob::setQuery( query );
}

void KPIM::ContactGroupSearchJob::setLimit( int limit )
{
  d->mLimit = limit;
}

KABC::ContactGroup::List KPIM::ContactGroupSearchJob::contactGroups() const
{
  KABC::ContactGroup::List contactGroups;

  foreach ( const Akonadi::Item &item, items() ) {
    if ( item.hasPayload<KABC::ContactGroup>() )
      contactGroups.append( item.payload<KABC::ContactGroup>() );
  }

  return contactGroups;
}

#include "contactgroupsearchjob_p.moc"
