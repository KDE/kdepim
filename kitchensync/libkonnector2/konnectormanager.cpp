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
#include <kparts/componentfactory.h>
#include <kstandarddirs.h>

#include "konnectorinfo.h"

#include "konnectormanager.h"

using namespace KSync;

static KStaticDeleter<KonnectorManager> deleter;
KonnectorManager* KonnectorManager::m_self = 0;

KonnectorManager::KonnectorManager()
  : KRES::Manager<Konnector>( "konnector" )
{
  readConfig();
  connectSignals();
}

KonnectorManager::~KonnectorManager()
{
}

KonnectorManager* KonnectorManager::self()
{
  if ( !m_self ) deleter.setObject( m_self, new KonnectorManager() );

  return m_self;
}

void KonnectorManager::connectSignals()
{
  Iterator it;
  for ( it = begin(); it != end(); ++it ) {
    connect( *it, SIGNAL(synceesRead(KSync::Konnector*) ),
             SIGNAL( synceesRead(KSync::Konnector*)) );
    connect( *it, SIGNAL(synceeReadError(KSync::Konnector*) ),
             SIGNAL( synceeReadError(KSync::Konnector*)) );
    connect( *it, SIGNAL(synceesWritten(KSync::Konnector*) ),
             SIGNAL( synceesWritten(KSync::Konnector*)) );
    connect( *it, SIGNAL(synceeWriteError(KSync::Konnector*) ),
             SIGNAL(synceeWriteError(KSync::Konnector*)) );
  }
}

#include "konnectormanager.moc"
