/*
  Copyright (c) 2011 Volker Krause <vkrause@kde.org>

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

#include "asyncnepomukresourceretriever.h"

#include <QtCore/QMutex>
#include <QtCore/QRunnable>
#include <QtCore/QThreadPool>

#include <KDebug>
#include <Nepomuk/Resource>
#include <Nepomuk/Variant>

using namespace MessageCore;

namespace MessageCore {

class NepomukResourceRetrieverRunnable : public QRunnable
{
  public:
    NepomukResourceRetrieverRunnable( const QUrl& url, const QVector<QUrl> &props, QObject *retriever ) :
      m_url( url ),
      m_props( props ),
      m_retriever( retriever )
    {}

    void run()
    {
      Nepomuk::Resource resource( m_url );
      foreach ( const QUrl &prop, m_props )
        resource.property( prop ); // loads and caches this property
      QMetaObject::invokeMethod( m_retriever, "resourceRetrievalDone", Qt::QueuedConnection,
                                 Q_ARG(QUrl, m_url), Q_ARG(Nepomuk::Resource, resource) );
    }

  private:
    QUrl m_url;
    QVector<QUrl> m_props;
    QObject* m_retriever;
};

class AsyncNepomukResourceRetrieverPrivate
{
  public:
    AsyncNepomukResourceRetrieverPrivate( AsyncNepomukResourceRetriever *parent ) : m_parent( parent ), m_running( false )
    {
      m_nepomukPool.setMaxThreadCount( 1 );
      qRegisterMetaType<Nepomuk::Resource>();
    }

    void createRunnable()
    {
      Q_ASSERT( !m_pendingRequests.isEmpty() );
      Q_ASSERT( !m_running );
      m_running = true;
      const QUrl url = m_pendingRequests.last();
      m_nepomukPool.start( new NepomukResourceRetrieverRunnable( url, m_requestedProperties.value( url ), m_parent ) );
    }

    void removeRequest( const QUrl &url )
    {
      const int index = m_pendingRequests.lastIndexOf( url );
      if ( index >= 0 )
        m_pendingRequests.remove( index );
      m_requestedProperties.remove( url );
    }

    void resourceRetrievalDone( const QUrl &url, const Nepomuk::Resource &res )
    {
      QMutexLocker locker( &m_mutex );
      m_running = false;
      removeRequest( url );
      if ( !m_pendingRequests.isEmpty() )
        createRunnable();
      locker.unlock();
      m_parent->resourceAvailable( url, res );
    }

    AsyncNepomukResourceRetriever *m_parent;
    QThreadPool m_nepomukPool;
    QVector<QUrl> m_pendingRequests;
    QHash<QUrl, QVector<QUrl> > m_requestedProperties;
    QMutex m_mutex;
    bool m_running;
};

}

AsyncNepomukResourceRetriever::AsyncNepomukResourceRetriever(QObject* parent) :
  QObject( parent ),
  d( new AsyncNepomukResourceRetrieverPrivate( this ) )
{
}

AsyncNepomukResourceRetriever::~AsyncNepomukResourceRetriever()
{
  delete d;
}

void AsyncNepomukResourceRetriever::requestResource(const QUrl& url, const QVector<QUrl> &properties)
{
  QMutexLocker locker( &d->m_mutex );
  if ( d->m_pendingRequests.contains( url ) )
    return;
  d->m_pendingRequests.push_back( url );
  d->m_requestedProperties.insert( url, properties );
  if ( !d->m_running )
    d->createRunnable();
}

void AsyncNepomukResourceRetriever::cancelRequest(const QUrl & url)
{
  QMutexLocker locker( &d->m_mutex );
  d->removeRequest( url );
}

void AsyncNepomukResourceRetriever::resourceAvailable(const QUrl& url, const Nepomuk::Resource& resource)
{
  emit resourceReceived( url, resource );
}


#include "asyncnepomukresourceretriever.moc"
