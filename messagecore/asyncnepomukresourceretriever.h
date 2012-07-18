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

#ifndef MESSAGECORE_ASYNCNEPOMUKRESOURCERETRIEVER_H
#define MESSAGECORE_ASYNCNEPOMUKRESOURCERETRIEVER_H

#include "messagecore_export.h"

#include <QtCore/QObject>
#include <akonadi/item.h>
#include <Nepomuk2/Resource>

namespace MessageCore {

class AsyncNepomukResourceRetrieverPrivate;

/**
 * Asynchronous retrieval of Nepomuk Resource objects.
 */
class MESSAGECORE_EXPORT AsyncNepomukResourceRetriever : public QObject
{
  Q_OBJECT
  public:
    explicit AsyncNepomukResourceRetriever( const QVector<QUrl> &properties, QObject* parent = 0 );
    ~AsyncNepomukResourceRetriever();

    void requestResource( const QUrl &url );
    void cancelRequest( const QUrl &url );

  Q_SIGNALS:
    void resourceReceived( const QUrl &url, const Nepomuk2::Resource &resource );
  protected Q_SLOTS:
    void nepomukStarted();
    void nepomukStopped();

  protected:
    virtual void resourceAvailable( const QUrl &url, const Nepomuk2::Resource &resource );

  private:
    AsyncNepomukResourceRetrieverPrivate* const d;
    friend class AsyncNepomukResourceRetrieverPrivate;
    Q_PRIVATE_SLOT( d, void resourceRetrievalDone( const QUrl &url, const Nepomuk2::Resource &resource ) )
};

}

#endif // MESSAGECORE_ASYNCNEPOMUKRESOURCERETRIEVER_H
