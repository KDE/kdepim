/*
    Copyright (c) 2009 Volker Krause <vkrause@kde.org>

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

#include "messagesearch.h"

#include <akonadi/item.h>
#include <akonadi/linkjob.h>

#include <Nepomuk/ResourceManager>
#include <KDebug>
#include <KUrl>
#include <Soprano/QueryResultIterator>
#include <Soprano/StatementIterator>
#include <Soprano/Model>

MessageSearch::MessageSearch(const QString& query, const Akonadi::Collection& destination, QObject* parent) :
  Task(parent),
  m_query( query ),
  m_destination( destination ),
  m_refCount( 0 )
{
  kDebug() << query << destination;
  ref();
  foreach ( const QByteArray &keyId, listCryptoContainers() )
    searchInContainer( keyId );
  deref();
}

void MessageSearch::searchInContainer(const QByteArray& keyId)
{
  kDebug() << keyId;
  if ( !mountCryptoContainer( keyId ) ) {
    kDebug() << "Unable to mount crypto container for key" << keyId;
    return;
  }

  Q_ASSERT( !hasActiveCryptoModel() );
  Soprano::Model* model = cryptoModel( keyId );

  if ( model ) {
#if 0 // Temporary debugging code, will need this again tomorrow
    {
      Soprano::StatementIterator it = model->listStatements();
      foreach ( Soprano::Statement s, it.allStatements() )
        kDebug() << "###" << s.subject().toString() << s.predicate().toString() << s.object().toString();
    }
#endif

    Soprano::QueryResultIterator it = model->executeQuery( m_query, Soprano::Query::QUERY_LANGUAGE_SPARQL );
    kDebug() << model->lastError().message();
    Akonadi::Item::List result;
    while ( it.next() ) {
      kDebug() << "Found:" << it.binding( 0 ).toString();
      Akonadi::Item item = Akonadi::Item::fromUrl( KUrl( it.binding( 0 ).toString() ) );
      if ( item.isValid() )
        result.append( item );
    }
    it.close();

    ref();
    Akonadi::LinkJob *linkJob = new Akonadi::LinkJob( m_destination, result, this );
    connect( linkJob, SIGNAL(result(KJob*)), SLOT(linkResult(KJob*)) );
  }

  resetModel();
  unmountCryptoContainer();
  Q_ASSERT( !hasActiveCryptoModel() );
}

void MessageSearch::linkResult(KJob* job)
{
  if ( job->error() )
    kWarning() << job->errorText();
  deref();
}

void MessageSearch::ref()
{
  ++m_refCount;
}

void MessageSearch::deref()
{
  --m_refCount;
  if ( m_refCount <= 0 )
    deleteLater();
}

#include "messagesearch.moc"