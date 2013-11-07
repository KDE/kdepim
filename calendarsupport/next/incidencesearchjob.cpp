/*
  Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

  Copyright (c) 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    Author: Sérgio Martins <sergio.martins@kdab.com>

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

#include "incidencesearchjob.h"

#include <Akonadi/ItemFetchScope>

using namespace CalendarSupport;
using namespace Akonadi;

class IncidenceSearchJob::Private
{
  public:
    int mLimit;
};

IncidenceSearchJob::IncidenceSearchJob( QObject *parent )
  : ItemSearchJob( QString(), parent ), d( new Private() )
{
  fetchScope().fetchFullPayload();
  d->mLimit = -1;

  // by default search for all incidences
  ItemSearchJob::setQuery( QLatin1String( ""
#ifdef AKONADI_USE_STRIGI_SEARCH
                                          "<request>"
                                          "  <query>"
                                          "    <equals>"
                                          "      <field name=\"type\"/>"
                                          "      <string>UnionOfEventJournalTodo</string>"
                                          "    </equals>"
                                          "  </query>"
                                          "</request>"
#else
                                          "prefix ncal:<http://www.semanticdesktop.org/ontologies/2007/04/02/ncal#>"
                                          "prefix nao:<http://www.semanticdesktop.org/ontologies/2007/08/15/nao#>"
                                          "SELECT DISTINCT ?r WHERE"
                                          "{"
                                          "?subclasses rdfs:subClassOf ncal:UnionOfEventJournalTodo ."
                                          "?r a ?subclasses ."
                                          "?r nao:hasSymbol \"view-pim-calendar\"^^<http://www.w3.org/2001/XMLSchema#string> ."
                                          "?r <" + akonadiItemIdUri().toEncoded() + "> ?itemId . "
                                          "}"
#endif
                         ) );
}

IncidenceSearchJob::~IncidenceSearchJob()
{
  delete d;
}

void IncidenceSearchJob::setQuery( Criterion criterion, const QString &value, Match match )
{
  if ( match == StartsWithMatch && value.size() < 4 ) {
    match = ExactMatch;
  }

  QString query;
#ifndef AKONADI_USE_STRIGI_SEARCH
  query = QString::fromLatin1( "prefix ncal:<http://www.semanticdesktop.org/ontologies/2007/04/02/ncal#> " );
#endif

  if ( match == ExactMatch ) {
    if ( criterion == IncidenceUid ) {
      query += QString::fromLatin1(
#ifdef AKONADI_USE_STRIGI_SEARCH
        "<request>"
        "  <query>"
        "    <and>"
        "      <equals>"
        "        <field name=\"type\"/>"
        "        <string>UnionOfEventJournalTodo</string>"
        "      </equals>"
        "      <equals>"
        "        <field name=\"uid\"/>"
        "        <string>%1</string>"
        "      </equals>"
        "    </and>"
        "  </query>"
        "</request>"
#else
        "SELECT ?r WHERE {"
        "?subclasses rdfs:subClassOf ncal:UnionOfEventJournalTodo ."
        "?r a ?subclasses ."
        "?r nao:hasSymbol \"view-pim-calendar\"^^<http://www.w3.org/2001/XMLSchema#string> ."
        "?r ncal:uid \"%1\"^^<http://www.w3.org/2001/XMLSchema#string> ."
        "?r <" + akonadiItemIdUri().toEncoded() + "> ?itemId . "
        "}"
#endif
      );
    }
  } else if ( match == StartsWithMatch ) {
    if ( criterion == IncidenceUid ) {
      query += QString::fromLatin1(
#ifdef AKONADI_USE_STRIGI_SEARCH
        "<request>"
        "  <query>"
        "    <and>"
        "      <equals>"
        "        <field name=\"type\"/>"
        "        <string>UnionOfEventJournalTodo</string>"
        "      </equals>"
        "      <startsWith>"
        "        <field name=\"uid\"/>"
        "        <string>%1</string>"
        "      </startsWith>"
        "    </and>"
        "  </query>"
        "</request>"
#else
        "SELECT ?r WHERE"
        "{"
        "?subclasses rdfs:subClassOf ncal:UnionOfEventJournalTodo ."
        "?r a ?subclasses ."
        "?r ncal:uid ?uid ."
        "?r nao:hasSymbol \"view-pim-calendar\"^^<http://www.w3.org/2001/XMLSchema#string> ."
        "?r <" + akonadiItemIdUri().toEncoded() + "> ?itemId . "
        "FILTER REGEX( ?uid, \"^%1\", 'i')"
        "}"
#endif
      );
    }
  } else if ( match == ContainsMatch ) {
    if ( criterion == IncidenceUid ) {
      query += QString::fromLatin1(
#ifdef AKONADI_USE_STRIGI_SEARCH
        "<request>"
        "  <query>"
        "    <and>"
        "      <equals>"
        "        <field name=\"type\"/>"
        "        <string>UnionOfEventJournalTodo</string>"
        "      </equals>"
        "      <contains>"
        "        <field name=\"uid\"/>"
        "        <string>%1</string>"
        "      </contains>"
        "    </and>"
        "  </query>"
        "</request>"
#else
        "SELECT ?r WHERE"
        "{"
        "?subclasses rdfs:subClassOf ncal:UnionOfEventJournalTodo ."
        "?r a ?subclasses ."
        "?r ncal:uid ?uid ."
        "?r nao:hasSymbol \"view-pim-calendar\"^^<http://www.w3.org/2001/XMLSchema#string> ."
        "?r <" + akonadiItemIdUri().toEncoded() + "> ?itemId . "
        "FILTER REGEX( ?uid, \"%1\", 'i')"
        "}"
#endif
      );
    }
  }

  if ( d->mLimit != -1 ) {
#ifndef AKONADI_USE_STRIGI_SEARCH
    query += QString::fromLatin1( " LIMIT %1" ).arg( d->mLimit );
#endif
  }
  query = query.arg( value );
  ItemSearchJob::setQuery( query );
}

void IncidenceSearchJob::setLimit( int limit )
{
  d->mLimit = limit;
}

KCalCore::Incidence::List IncidenceSearchJob::incidences() const
{
  KCalCore::Incidence::List incidences;

  foreach ( const Item &item, items() ) {
    if ( item.hasPayload<KCalCore::Incidence::Ptr>() ) {
      incidences.append( item.payload<KCalCore::Incidence::Ptr>() );
    }
  }

  return incidences;
}

