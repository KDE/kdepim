/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

    Based on KMail code by:
    Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>
    Copyright (c) 2000-2002 Michael Haeckel <haeckel@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "precommandjob.h"

#include <klocale.h>
#include <k3process.h>

using namespace MailTransport;

PrecommandJob::PrecommandJob(const QString & precommand, QObject * parent) :
    KJob( parent ),
    mProcess( 0 ),
    mPrecommand( precommand )
{
  mProcess = new K3Process( this );
  mProcess->setUseShell( true );
  *mProcess << precommand;
  connect( mProcess, SIGNAL(processExited(K3Process*)), SLOT(processExited(K3Process*)) );
}

PrecommandJob::~ PrecommandJob()
{
  delete mProcess;
}

void PrecommandJob::start()
{
  if ( !mProcess->start( K3Process::NotifyOnExit ) ) {
    setError( UserDefinedError );
    setErrorText( i18n("Could not execute precommand '%1'.", mPrecommand ) );
    emitResult();
  } else {
    emit infoMessage( this, i18n("Executing precommand"),
                      i18n("Executing precommand '%1'.", mPrecommand ) );
  }
}

bool PrecommandJob::doKill()
{
  delete mProcess;
  mProcess = 0;
  return true;
}

void PrecommandJob::processExited(K3Process *process)
{
  Q_ASSERT( mProcess == process );

  if ( mProcess->normalExit() ) {
    if ( mProcess->exitStatus() ) {
      setError( UserDefinedError );
      setErrorText( i18n("The precommand exited with code %1.", mProcess->exitStatus()) );
    }
  }
  if ( mProcess->signalled() ) {
    setError( UserDefinedError );
    setErrorText( i18n("The precommand was terminated by signal %1", mProcess->exitSignal() ) );
  }
  emitResult();
}

#include "precommandjob.moc"
