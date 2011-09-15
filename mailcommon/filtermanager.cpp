/*
    Copyright (C) 2011 Tobias Koenig <tokoe@kde.org>

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

#include "filtermanager.h"

using namespace MailCommon;

FilterManager* FilterManager::mInstance = 0;

FilterManager* FilterManager::instance()
{
  if ( !mInstance )
    mInstance = new FilterManager;

  return mInstance;
}

FilterManager::FilterManager()
{
  mMailFilterAgentInterface = new org::freedesktop::Akonadi::MailFilterAgent( QLatin1String( "org.freedesktop.Akonadi.MailFilterAgent" ),
                                                                              QLatin1String( "/MailFilterAgent" ),
                                                                              QDBusConnection::sessionBus(), this );
}

bool FilterManager::isValid() const
{
  return mMailFilterAgentInterface->isValid();
}

QString FilterManager::createUniqueFilterName( const QString &name ) const
{
  return mMailFilterAgentInterface->createUniqueName( name );
}

void FilterManager::filter( const Akonadi::Item &item, const QString &identifier ) const
{
}

void FilterManager::filter( const Akonadi::Item &item, FilterSet set, bool account, const QString &resourceId ) const
{
}

void FilterManager::filter( const Akonadi::Item::List &messages, FilterSet set ) const
{
}
