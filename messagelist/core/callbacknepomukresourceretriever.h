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

#ifndef MESSAGELIST_CALLBACKNEPOMUKRESOURCERETRIEVER_H
#define MESSAGELIST_CALLBACKNEPOMUKRESOURCERETRIEVER_H

#include <messagecore/nepomukutil/asyncnepomukresourceretriever.h>
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
    virtual void resourceAvailable(const QUrl& url, const Nepomuk2::Resource& resource);

  private:
    QHash<QUrl, Core::MessageItemPrivate*> m_pendingCallbacks;
};

}

#endif // MESSAGELIST_CALLBACKNEPOMUKRESOURCERETRIEVER_H
