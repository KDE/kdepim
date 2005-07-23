/*
    This file is part of libkcal.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "freebusyurlstore.h"

#include <kstaticdeleter.h>
#include <kconfig.h>
#include <kstandarddirs.h>

using namespace KCal;

static KStaticDeleter<FreeBusyUrlStore> selfDeleter;

FreeBusyUrlStore *FreeBusyUrlStore::mSelf = 0;

FreeBusyUrlStore *FreeBusyUrlStore::self()
{
  if ( !mSelf ) {
    selfDeleter.setObject( mSelf, new FreeBusyUrlStore() );
  }
  return mSelf;
}

FreeBusyUrlStore::FreeBusyUrlStore()
{
  QString configFile = locateLocal( "data", "korganizer/freebusyurls" );
  mConfig = new KConfig( configFile );
}

FreeBusyUrlStore::~FreeBusyUrlStore()
{
  delete mConfig;
}

void FreeBusyUrlStore::writeUrl( const QString &email, const QString &url )
{
  mConfig->setGroup( email );

  mConfig->writeEntry( "url", url );
}

QString FreeBusyUrlStore::readUrl( const QString &email )
{
  mConfig->setGroup( email );
  
  return mConfig->readEntry( "url" );
}

void FreeBusyUrlStore::sync()
{
  mConfig->sync();
}
