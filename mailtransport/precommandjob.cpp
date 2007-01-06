/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "precommandjob.h"

#include <klocale.h>
#include <kprocess.h>

using namespace KPIM;

PrecommandJob::PrecommandJob(const QString & precommand, QObject * parent) :
    KJob( parent ),
    mProcess( 0 ),
    mPrecommand( precommand )
{
  mProcess = new KProcess( this );
  mProcess->setUseShell( true );
  *mProcess << precommand;
  connect( mProcess, SIGNAL(processExited(KProcess*)), SLOT(processExited(KProcess*)) );
}

PrecommandJob::~ PrecommandJob()
{
  delete mProcess;
}

void PrecommandJob::start()
{
  if ( !mProcess->start( KProcess::NotifyOnExit ) ) {
    setError( UserDefinedError );
    setErrorText( i18n("Could not execute precommand '%1'.") );
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

void PrecommandJob::processExited(KProcess *process)
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
