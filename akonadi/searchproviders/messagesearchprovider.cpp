/*
    Copyright (c) 2006 Volker Krause <vkrause@kde.org>

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

#include "messagesearchprovider.h"
#include "messagesearchproviderthread.h"
#include <QtCore/QCoreApplication>

using namespace Akonadi;

Akonadi::MessageSearchProvider::MessageSearchProvider( const QString &id ) :
  SearchProviderBase( id )
{
}

QList< QByteArray > Akonadi::MessageSearchProvider::supportedMimeTypes() const
{
  QList<QByteArray> mimeTypes;
  mimeTypes << "message/rfc822" << "message/news";
  return mimeTypes;
}

SearchProviderThread* Akonadi::MessageSearchProvider::workerThread()
{
  return new MessageSearchProviderThread( this );
}


int main( int argc, char **argv )
{
  QCoreApplication app( argc, argv );
  Akonadi::SearchProviderBase::init<Akonadi::MessageSearchProvider>( argc, argv, QLatin1String("akonadi_message_searchprovider") );
  return app.exec();
}
