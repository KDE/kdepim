/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>

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

#include <qdir.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kstandarddirs.h>

#include "konnectorinfo.h"

#include "konnectormanager.h"

using namespace KSync;

KonnectorManager::KonnectorManager()
  : KRES::Manager<Konnector>( "konnector" )
{
  connectSignals();
}

KonnectorManager::~KonnectorManager()
{
}

void KonnectorManager::connectSignals()
{
  Iterator it;
  for ( it = begin(); it != end(); ++it ) {
    connect( *it, SIGNAL( synceesRead( Konnector * ) ),
             SIGNAL( synceesRead( Konnector * ) ) );
    connect( *it, SIGNAL( synceeReadError( Konnector * ) ),
             SIGNAL( synceeReadError( Konnector * ) ) );
    connect( *it, SIGNAL( synceesWritten( Konnector * ) ),
             SIGNAL( synceesWritten( Konnector * ) ) );
    connect( *it, SIGNAL( synceeWriteError( Konnector * ) ),
             SIGNAL( synceeWriteError( Konnector * ) ) );
  }
}

#include "konnectormanager.moc"
