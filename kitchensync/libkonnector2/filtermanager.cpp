/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qfile.h>

#include <kdebug.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <kstaticdeleter.h>
#include <ktrader.h>

#include "filtermanager.h"

using namespace KSync;

FilterManager *FilterManager::mSelf = 0;
static KStaticDeleter<FilterManager> filterFactoryDeleter;

FilterManager::FilterManager()
{
  loadFactories();
}

FilterManager::~FilterManager()
{
}

FilterManager *FilterManager::self()
{
  if ( !mSelf )
    filterFactoryDeleter.setObject( mSelf, new FilterManager );

  return mSelf;
}

Filter *FilterManager::create( const QString &type )
{
  FactoryMap::Iterator it = mFactoryMap.find( type );
  if (  it == mFactoryMap.end() ) {
    kdError() << "Asked for undefined filter type '" << type << "'" << endl;
    return 0;
  }

  return it.data()->createFilter( 0 );
}

void FilterManager::loadFactories()
{
  const KTrader::OfferList offers = KTrader::self()->query( "KitchenSync/Filter" );

  KLibFactory *factory = 0;
  KTrader::OfferList::ConstIterator it( offers.begin() );
  for ( ; it != offers.end(); ++it ) {
    KService::Ptr ptr = (*it);

    factory = KLibLoader::self()->factory( QFile::encodeName( ptr->library() ) );
    if ( !factory )
      continue;

    FilterFactory *filterFactory = static_cast<FilterFactory*>( factory );

    if ( filterFactory )
      mFactoryMap.insert( ptr->property( "X-KDE-KitchenSyncFilterType" ).toString(), filterFactory );
  }
}
