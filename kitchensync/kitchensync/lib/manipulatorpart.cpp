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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <kdebug.h>

#include "manipulatorpart.h"

#include "core.h"

using namespace KSync;

ManipulatorPart::ManipulatorPart( QObject *parent, const char *name )
  : KParts::Part(parent, name )
{
    m_window = 0;

    if ( parent && parent->inherits("KSync::Core") )
        m_window = static_cast<KSync::Core *>( parent );
}

ManipulatorPart::~ManipulatorPart()
{
}

int ManipulatorPart::syncProgress() const
{
    return m_prog;
}

int ManipulatorPart::syncStatus() const
{
    return m_stat;
}

bool ManipulatorPart::hasGui() const
{
    return false;
}

bool ManipulatorPart::configIsVisible() const
{
    return false;
}

bool ManipulatorPart::canSync() const
{
    return false;
}

QWidget *ManipulatorPart::configWidget()
{
    return 0;
}

void ManipulatorPart::sync( const SynceeList& , SynceeList& )
{
    done();
}

Core* ManipulatorPart::core()
{
    return m_window;
}

Core* ManipulatorPart::core() const
{
    return m_window;
}

void ManipulatorPart::progress( int pro )
{
    m_prog = pro;
    emit sig_progress( this, pro );
}

void ManipulatorPart::progress( const Progress& pro )
{
    emit sig_progress( this,pro );
}

void ManipulatorPart::error( const Error& err )
{
    emit sig_error( this, err );
}

void ManipulatorPart::done()
{
    m_stat = SYNC_DONE;
    emit sig_syncStatus( this, m_stat );
}

void ManipulatorPart::slotConfigOk()
{
}

void ManipulatorPart::connectPartChange( const char* slot )
{
    connect( core(), SIGNAL( partChanged( ManipulatorPart * ) ), slot );
}

void ManipulatorPart::connectPartProgress( const char* slot )
{
    connect( core(), SIGNAL(partProgress( ManipulatorPart*, const Progress& ) ),
             slot );
}

void ManipulatorPart::connectPartError( const char* slot )
{
    connect( core(), SIGNAL(partError( ManipulatorPart*, const Error& ) ),
             slot );
}

void ManipulatorPart::connectKonnectorProgress( const char* slot )
{
    connect( core(), SIGNAL(konnectorProgress( Konnector *, const Progress& ) ),
             slot );
}

void ManipulatorPart::connectKonnectorError( const char* slot )
{
    connect( core(), SIGNAL( konnectorError( Konnector *, const Error & ) ),
             slot );
}

void ManipulatorPart::connectSyncProgress( const char* slot )
{
    connect( core(), SIGNAL(syncProgress( ManipulatorPart*, int, int ) ), slot );
}

void ManipulatorPart::connectProfileChanged( const char* slot )
{
    connect( core(), SIGNAL( profileChanged( const Profile & ) ), slot );
}

void ManipulatorPart::connectKonnectorDownloaded( const char* slot )
{
    connect( core(), SIGNAL(konnectorDownloaded( Konnector *, Syncee::PtrList ) ),
             slot );
}

void ManipulatorPart::connectStartSync( const char* slot )
{
    connect( core(), SIGNAL( startSync() ), slot );
}

void ManipulatorPart::connectDoneSync( const char* slot )
{
    connect( core(), SIGNAL( doneSync() ), slot );
}

bool ManipulatorPart::confirmBeforeWriting() const
{
    return core()->currentProfile().confirmSync();
}

void ManipulatorPart::actionSync()
{
}

#include "manipulatorpart.moc"
