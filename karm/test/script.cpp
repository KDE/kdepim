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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#include <qprocess.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtimer.h>

#include <kdebug.h>

#include "script.h"

// Wait for terminate() attempt to return before using kill()
// kill() doesn't let script interpreter try to clean up.
const int NICE_KILL_TIMEOUT_IN_SECS = 5;

Script::Script()
{
  m_status = 0;
  m_stderr = false;
  m_timeoutInSeconds = 5;

  m_proc   = new QProcess( this );

  connect ( m_proc, SIGNAL( readyReadStdout() ), 
            this  , SLOT  ( stdout() ) 
            );
  connect ( m_proc, SIGNAL( readyReadStderr() ), 
            this  , SLOT  ( stderr() ) 
            );
  connect ( m_proc, SIGNAL( processExited() ), 
            this  , SLOT  ( exit() ) 
            );
}

Script::~Script()
{
  delete m_proc;
  m_proc = 0;
}

void Script::addArgument( const QString &arg )
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
  kdDebug() << "Script::running " << m_proc->arguments() << " ..." << endl;
  m_proc->start();
  // This didn't work.  But Ctrl-C does.  :P
  //QTimer::singleShot( m_timeoutInSeconds * 1000, m_proc, SLOT( kill() ) );
  while ( ! m_proc->normalExit() );
  return m_status;
}

void Script::terminate()
{
  kdDebug() << "Script::terminate()" << endl;
  // These both trigger processExited, so exit() will run.
  m_proc->tryTerminate();
  QTimer::singleShot( NICE_KILL_TIMEOUT_IN_SECS*1000, m_proc, SLOT( kill() ) );
}

void Script::exit()
{
  //kdDebug () << "Script::exit()" << endl;
  m_status = m_proc->exitStatus();
  //kdDebug () << "script exit, status = " << m_proc->exitStatus() << endl;
  delete m_proc;
  m_proc = 0;
}

void Script::stderr()
{
  // Treat any output to std err as a script failure
  m_status = 1;
  QString data = QString( m_proc->readStderr() );
  m_stderr= true;
  kdDebug() << "stderr:" << endl;
  kdDebug() << data << endl;
}

void Script::stdout()
{
  QString data = QString( m_proc->readStdout() );
  kdDebug() << "stdout:" << endl;
  kdDebug() << data << endl;
}

#include "script.moc"
