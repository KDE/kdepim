/*
    This file is part of libkdepim.

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

#include <kconfig.h>
#include <kglobal.h>
#include <kstaticdeleter.h>

#include <dcopref.h>

#include "networkstatus.h"

using namespace KPIM;

static KStaticDeleter<NetworkStatus> networkStatusDeleter;
NetworkStatus *NetworkStatus::mSelf = 0;

NetworkStatus::NetworkStatus()
  : QObject( 0, "NetworkStatus" ), DCOPObject( "NetworkStatus" )
{
  KConfigGroup group( KGlobal::config(), "NetworkStatus" );
  if ( group.readBoolEntry( "Online", true ) == true )
    mStatus = Online;
  else
    mStatus = Offline;

  connectDCOPSignal( 0, 0, "onlineStatusChanged()", "onlineStatusChanged()", false );
}

NetworkStatus::~NetworkStatus()
{
  KConfigGroup group( KGlobal::config(), "NetworkStatus" );
  group.writeEntry( "Online", mStatus == Online );
}

void NetworkStatus::setStatus( Status status )
{
  mStatus = status;

  emit statusChanged( mStatus );
}

NetworkStatus::Status NetworkStatus::status() const
{
  return mStatus;
}

void NetworkStatus::onlineStatusChanged()
{
  DCOPRef dcopCall( "kded", "networkstatus" );
  DCOPReply reply = dcopCall.call( "onlineStatus()", true );
  if ( reply.isValid() ) {
    int status = reply;
    if ( status == 3 )
      setStatus( Online );
    else {
      if ( mStatus != Offline )
        setStatus( Offline );
    }
  }
}

NetworkStatus *NetworkStatus::self()
{
  if ( !mSelf )
    networkStatusDeleter.setObject( mSelf, new NetworkStatus );

  return mSelf;
}

#include "networkstatus.moc"
