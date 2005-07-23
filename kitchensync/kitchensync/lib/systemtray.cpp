/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>
† † Copyright (c) 2002 Maximilian Reiﬂ <harlekin@handhelds.org>

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

#include <kdebug.h>
#include <kiconloader.h>

#include "systemtray.h"

using namespace KSync;

KSyncSystemTray::KSyncSystemTray( QWidget* parent )
  : KSystemTray( parent, "" )
{
  mIconConnected = KGlobal::iconLoader()->loadIcon( "connect_established", KIcon::Small );
  mIconDisconnected = KGlobal::iconLoader()->loadIcon( "connect_no", KIcon::Small );

  setState( false );
}

KSyncSystemTray::~KSyncSystemTray()
{
}

void setName( QString & )
{
}

void KSyncSystemTray::setState( bool connected )
{
  if ( connected ) {
    setPixmap( mIconConnected );
  } else {
    setPixmap( mIconDisconnected );
  }
}

#include "systemtray.moc"
