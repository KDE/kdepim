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

#include "actionpart.h"

#include "core.h"

using namespace KSync;

ActionPart::ActionPart( QObject *parent, const char *name )
  : KParts::Part( parent, name )
{
    m_window = 0;

    if ( parent && parent->inherits("KSync::Core") )
        m_window = static_cast<KSync::Core *>( parent );
}

ActionPart::~ActionPart()
{
}

int ActionPart::syncProgress() const
{
    return m_prog;
}

int ActionPart::syncStatus() const
{
    return m_stat;
}

bool ActionPart::hasGui() const
{
    return false;
}

bool ActionPart::configIsVisible() const
{
    return false;
}

bool ActionPart::canSync() const
{
    return false;
}

QWidget *ActionPart::configWidget()
{
    return 0;
}

void ActionPart::sync( const SynceeList& , SynceeList& )
{
    done();
}

Core* ActionPart::core()
{
    return m_window;
}

Core* ActionPart::core() const
{
    return m_window;
}

void ActionPart::done()
{
    m_stat = SYNC_DONE;
}

void ActionPart::slotConfigOk()
{
}

void ActionPart::connectPartChange( const char *slot )
{
    connect( core(), SIGNAL( partChanged( ActionPart * ) ), slot );
}

void ActionPart::connectSyncProgress( const char *slot )
{
    connect( core(), SIGNAL( syncProgress( ActionPart *, int, int ) ), slot );
}

void ActionPart::connectProfileChanged( const char *slot )
{
    connect( core(), SIGNAL( profileChanged( const Profile & ) ), slot );
}

void ActionPart::connectKonnectorDownloaded( const char *slot )
{
    connect( core(),
             SIGNAL( konnectorDownloaded( Konnector *, Syncee::PtrList ) ),
             slot );
}

void ActionPart::connectStartSync( const char *slot )
{
    connect( core(), SIGNAL( startSync() ), slot );
}

void ActionPart::connectDoneSync( const char *slot )
{
    connect( core(), SIGNAL( doneSync() ), slot );
}

bool ActionPart::confirmBeforeWriting() const
{
    return core()->currentProfile().confirmSync();
}

#include "actionpart.moc"
