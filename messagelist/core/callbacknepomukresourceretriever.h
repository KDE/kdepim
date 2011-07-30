#ifndef MESSAGELIST_CALLBACKNEPOMUKRESOURCERETRIEVER_H
#define MESSAGELIST_CALLBACKNEPOMUKRESOURCERETRIEVER_H

#include <messagecore/asyncnepomukresourceretriever.h>
#include "messageitem_p.h"

namespace MessageList {

/**
 * Specialized version of the AsyncNepomukResourceRetriever that can deal with non-QObject receivers.
 * @internal
 */
class CallbackNepomukResourceRetriever : public MessageCore::AsyncNepomukResourceRetriever
{
  Q_OBJECT
  public:
    explicit CallbackNepomukResourceRetriever(QObject* parent = 0);

    void requestResource( Core::MessageItemPrivate *item, const QUrl &url );
    void cancelCallbackRequest( const QUrl &url );

  protected:
    virtual void resourceAvailable(const QUrl& url, const Nepomuk::Resource& resource);

  private:
    QHash<QUrl, Core::MessageItemPrivate*> m_pendingCallbacks;
};

}

#endif // MESSAGELIST_CALLBACKNEPOMUKRESOURCERETRIEVER_H
