/* This file is part of the KDE project
   Copyright (C) 2002 Anders Lund <anders@alweb.dk>
   Copyright (C) 2004 Mark Bucciarelli <mark@hubcapconsulting.com>

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
#include <tqdir.h>
#include <tqprocess.h>
#include <tqstring.h>
#include <tqstringlist.h>
#include <tqtimer.h>

#include <kdebug.h>

#include "script.h"

/*
        n.b. Do not use kdDebug statements in this file.

        With qt-copy 3_3_BRANCH, they cause a Valgrind error.
        Ref: KDE bug #95237.
*/

// Wait for terminate() attempt to return before using kill()
// kill() doesn't let script interpreter try to clean up.
const int NICE_KILL_TIMEOUT_IN_SECS = 5;

Script::Script( const TQDir& workingDirectory )
{
  m_status = 0;
  m_stderr = false;
  m_timeoutInSeconds = 5;

  m_proc   = new TQProcess( this );
  m_proc->setWorkingDirectory( workingDirectory );

  connect ( m_proc, TQT_SIGNAL( readyReadStdout() ), 
            this  , TQT_SLOT  ( stdout() ) 
            );
  connect ( m_proc, TQT_SIGNAL( readyReadStderr() ), 
            this  , TQT_SLOT  ( stderr() ) 
            );
  connect ( m_proc, TQT_SIGNAL( processExited() ), 
            this  , TQT_SLOT  ( exit() ) 
            );
}

Script::~Script()
{
  delete m_proc;
  m_proc = 0;
}

void Script::addArgument( const TQString &arg )
{
  m_proc->addArgument( arg );
}

void Script::setTimeout( int seconds )
{
  if ( seconds <= 0 ) return;
  m_timeoutInSeconds = seconds;
}

int Script::run()
{
  m_proc->start();
  // This didn't work.  But Ctrl-C does.  :P
  //TQTimer::singleShot( m_timeoutInSeconds * 1000, m_proc, TQT_SLOT( kill() ) );
  //while ( ! m_proc->normalExit() );
  while ( m_proc->isRunning() );
  return m_status;
}

void Script::terminate()
{
  // These both trigger processExited, so exit() will run.
  m_proc->tryTerminate();
  TQTimer::singleShot( NICE_KILL_TIMEOUT_IN_SECS*1000, m_proc, TQT_SLOT( kill() ) );
}

void Script::exit()
{
  m_status = m_proc->exitStatus();
  delete m_proc;
  m_proc = 0;
}

void Script::stderr()
{
  // Treat any output to std err as a script failure
  m_status = 1;
  TQString data = TQString( m_proc->readStderr() );
  m_stderr= true;
}

void Script::stdout()
{
  TQString data = TQString( m_proc->readStdout() );
}

#include "script.moc"
