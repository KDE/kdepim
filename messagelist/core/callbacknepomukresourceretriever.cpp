#include "callbacknepomukresourceretriever.h"

using namespace MessageList;

CallbackNepomukResourceRetriever::CallbackNepomukResourceRetriever(QObject* parent): AsyncNepomukResourceRetriever(parent)
{
}

void CallbackNepomukResourceRetriever::requestResource(Core::MessageItemPrivate* item, const QUrl& url)
{
  m_pendingCallbacks.insert( url, item );
  AsyncNepomukResourceRetriever::requestResource( url );
}

void CallbackNepomukResourceRetriever::cancelCallbackRequest(const QUrl& url)
{
  m_pendingCallbacks.remove( url );
  AsyncNepomukResourceRetriever::cancelRequest( url );
}

void CallbackNepomukResourceRetriever::resourceAvailable(const QUrl& url, const Nepomuk::Resource& resource)
{
  Core::MessageItemPrivate* item = m_pendingCallbacks.take( url );
  if ( item )
    item->resourceReceived( resource );
}
