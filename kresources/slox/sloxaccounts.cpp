/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "sloxaccounts.h"

#include <libkcal/freebusyurlstore.h>

#include <kabc/stdaddressbook.h>

#include <kstaticdeleter.h>
#include <kdebug.h>

QString SloxAccounts::mServer;
QString SloxAccounts::mDomain;

SloxAccounts *SloxAccounts::mSelf = 0;
KStaticDeleter<SloxAccounts> selfDeleter;

SloxAccounts *SloxAccounts::self()
{
  if ( !mSelf ) {
    selfDeleter.setObject( mSelf, new SloxAccounts );
  }
  return mSelf;
}

SloxAccounts::SloxAccounts()
{
  kdDebug() << "SloxAccounts()" << endl;

#if 1
  KABC::AddressBook *ab = KABC::StdAddressBook::self();
  ab->asyncLoad();
#endif
}

void SloxAccounts::setServer( const QString &server )
{
  mServer = server;

  QStringList l = QStringList::split( '.', server );

  if ( l.count() < 2 ) mDomain = server;
  else mDomain = l[ l.count() - 2 ] + "." + l[ l.count() - 1 ];
}

void SloxAccounts::insertUser( const QString &id, const KABC::Addressee &a )
{
  mUsers.replace( id, a.uid() );

  QString email = id + "@" + mDomain;
  
  QString url = "http://" + mServer + "/servlet/webdav.freebusy?username=";
  url += id + "&server=" + mDomain;

  KCal::FreeBusyUrlStore::self()->writeUrl( email, url );
}

KABC::Addressee SloxAccounts::lookupUser( const QString &id )
{
  QMap<QString,QString>::ConstIterator it;
  it = mUsers.find( id );
  if ( it == mUsers.end() ) {
    return KABC::Addressee();
  } else {
    return KABC::StdAddressBook::self()->findByUid( *it );
  }
}
