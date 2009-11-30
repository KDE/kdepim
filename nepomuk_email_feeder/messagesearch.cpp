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

#include <KDebug>

MessageSearch::MessageSearch(const QString& query, const Akonadi::Collection& destination, QObject* parent) :
  Task(parent),
  m_query( query ),
  m_destination( destination )
{
  kDebug() << query << destination;
  foreach ( const QByteArray &keyId, listCryptoContainers() )
    searchInContainer( keyId );
  deleteLater();
}

void MessageSearch::searchInContainer(const QByteArray& keyId)
{
  kDebug() << keyId;
  if ( !mountCryptoContainer( keyId ) ) {
    kDebug() << "Unable to mount crypto container for key" << keyId;
    return;
  }

  // TODO

  unmountCryptoContainer();
}

#include "messagesearch.moc"