/*
† † This file is part of the OPIE Project
† † Copyright (c)  2002 Holger Freyther <zecke@handhelds.org>
† †                2002 Maximilian Reiﬂ <harlekin@handhelds.org>



 †             =.
† † † † † † †.=l.
† † † † † †.>+-=
†_;:, † † .> † †:=|.         This library is free software; you can
.> <`_, † > †. † <=          redistribute it and/or  modify it under
:`=1 )Y*s>-.-- † :           the terms of the GNU Library General Public
.="- .-=="i, † † .._         License as published by the Free Software
†- . † .-<_> † † .<>         Foundation; either version 2 of the License,
† † †._= =} † † † :          or (at your option) any later version.
† † .%`+i> † † † _;_.
† † .i_,=:_. † † †-<s.       This library is distributed in the hope that
† † †+ †. †-:. † † † =       it will be useful,  but WITHOUT ANY WARRANTY;
† † : .. † †.:, † † . . .    without even the implied warranty of
† † =_ † † † †+ † † =;=|`    MERCHANTABILITY or FITNESS FOR A
† _.=:. † † † : † †:=>`:     PARTICULAR PURPOSE. See the GNU
..}^=.= † † † = † † † ;      Library General Public License for more
++= † -. † † .` † † .:       details.
†: † † = †...= . :.=-
†-. † .:....=;==+<;          You should have received a copy of the GNU
† -_. . . † )=. †=           Library General Public License along with
† † -- † † † †:-=`           this library; see the file COPYING.LIB.
                             If not, write to the Free Software Foundation,
                             Inc., 59 Temple Place - Suite 330,
                             Boston, MA 02111-1307, USA.

*/


#include <kdebug.h>

#include "manipulatorpart.h"

#include "mainwindow.h"

using namespace KSync;

ManipulatorPart::ManipulatorPart(QObject *parent, const char *name )
  : KParts::Part(parent, name )
{
    m_window = 0;

    if ( parent && parent->inherits("KSync::KSyncMainWindow") )
        m_window = static_cast<KSyncMainWindow*>(parent);
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

bool ManipulatorPart::partIsVisible() const
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

QWidget* ManipulatorPart::configWidget()
{
    return 0;
}

void ManipulatorPart::sync( const Syncee::PtrList& , Syncee::PtrList& )
{
    done();
}

KSyncMainWindow* ManipulatorPart::core()
{
    return m_window;
}

KSyncMainWindow* ManipulatorPart::core() const
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
    connect( core(), SIGNAL(konnectorProgress(const UDI&, const Progress& ) ),
             slot );
}

void ManipulatorPart::connectKonnectorError( const char* slot )
{
    connect( core(), SIGNAL( konnectorError( const UDI &, const Error & ) ),
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

void ManipulatorPart::connectKonnectorChanged( const char* slot )
{
    connect( core(), SIGNAL( konnectorChanged( const UDI & ) ), slot );
}

void ManipulatorPart::connectKonnectorDownloaded( const char* slot )
{
    connect( core(), SIGNAL(konnectorDownloaded(const UDI&, Syncee::PtrList ) ),
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

#include "manipulatorpart.moc"
