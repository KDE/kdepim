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
#include <Nepomuk2/Resource>
#include <Nepomuk2/Variant>
#include <Nepomuk2/ResourceManager>
#include <QCache>

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
      Nepomuk2::Resource resource( m_url );
      resource.setWatchEnabled( false );
      foreach ( const QUrl &prop, m_props )
        resource.property( prop ); // loads and caches this property
      QMetaObject::invokeMethod( m_retriever, "resourceRetrievalDone", Qt::QueuedConnection,
                                 Q_ARG(QUrl, m_url), Q_ARG(Nepomuk2::Resource, resource) );
    }

  private:
    QUrl m_url;
    QVector<QUrl> m_props;
    QObject* m_retriever;
};

class AsyncNepomukResourceRetrieverPrivate
{
  public:
    AsyncNepomukResourceRetrieverPrivate( AsyncNepomukResourceRetriever *parent ) : m_parent( parent ), m_running( false ), m_nepomukInitialized(false)
    {
      m_nepomukPool.setMaxThreadCount( 1 );
      m_resourceCache.setMaxCost( 10 );
      qRegisterMetaType<Nepomuk2::Resource>();
    }

    void createRunnable()
    {
      Q_ASSERT( !m_requestedProperties.isEmpty() );
      Q_ASSERT( !m_running );
      m_running = true;
      const QUrl url = m_requestedProperties.begin().key();
      m_nepomukPool.start( new NepomukResourceRetrieverRunnable( url, m_requestedProperties.value( url ), m_parent ) );
    }

    void removeRequest( const QUrl &url )
    {
      m_requestedProperties.remove( url );
    }

    void resourceRetrievalDone( const QUrl &url, const Nepomuk2::Resource &res )
    {
      QMutexLocker locker( &m_mutex );
      m_running = false;
      removeRequest( url );
      if ( !m_requestedProperties.isEmpty() )
        createRunnable();
      locker.unlock();
      m_parent->resourceAvailable( url, res );
    }
    void clearRequests()
    {
      m_requestedProperties.clear();
    }

    AsyncNepomukResourceRetriever *m_parent;
    QThreadPool m_nepomukPool;
    typedef QHash<QUrl, QVector<QUrl> > RequestsHash;
    RequestsHash m_requestedProperties;
    QMutex m_mutex;
    QVector<QUrl> m_properties;
    bool m_running;
    bool m_nepomukInitialized;

    // The last n requested resources are cached so that they do not
    // need to be loaded again and again
    QCache<QUrl, Nepomuk2::Resource> m_resourceCache;
};

}

AsyncNepomukResourceRetriever::AsyncNepomukResourceRetriever(const QVector<QUrl> &properties, QObject* parent) :
  QObject( parent ),
  d( new AsyncNepomukResourceRetrieverPrivate( this ) )
{
  d->m_properties = properties;
  connect( Nepomuk2::ResourceManager::instance(), SIGNAL(nepomukSystemStarted()),
           SLOT(nepomukStarted()) );
  connect( Nepomuk2::ResourceManager::instance(), SIGNAL(nepomukSystemStopped()),
           SLOT(nepomukStopped()) );

  d->m_nepomukInitialized = Nepomuk2::ResourceManager::instance()->initialized();
}

AsyncNepomukResourceRetriever::~AsyncNepomukResourceRetriever()
{
  delete d;
}

void AsyncNepomukResourceRetriever::nepomukStarted()
{
  d->m_nepomukInitialized = true;
}

void AsyncNepomukResourceRetriever::nepomukStopped()
{
  d->m_nepomukInitialized = false;
  QMutexLocker locker( &d->m_mutex );
  d->clearRequests();
}

void AsyncNepomukResourceRetriever::requestResource(const QUrl& url)
{
  if(!d->m_nepomukInitialized)
    return;
  QMutexLocker locker( &d->m_mutex );
  if ( d->m_requestedProperties.contains( url ) )
    return;

  Nepomuk2::Resource* resPtr = d->m_resourceCache.object(url);
  if( resPtr ) {
    emit resourceReceived( url, *resPtr );
    return;
  }
  d->m_requestedProperties.insert( url, d->m_properties );
  if ( !d->m_running )
    d->createRunnable();
}

void AsyncNepomukResourceRetriever::cancelRequest(const QUrl & url)
{
  if(d->m_nepomukInitialized) {
    QMutexLocker locker( &d->m_mutex );
    d->removeRequest( url );
  }
}

void AsyncNepomukResourceRetriever::resourceAvailable(const QUrl& url, const Nepomuk2::Resource& resource)
{
  d->m_resourceCache.insert( url, new Nepomuk2::Resource(resource) );
  emit resourceReceived( url, resource );
}


#include "asyncnepomukresourceretriever.moc"
