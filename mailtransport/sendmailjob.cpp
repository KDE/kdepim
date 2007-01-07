/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

    Based on KMail code by:
    Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>

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

#include "sendmailjob.h"
#include "transport.h"

#include <klocale.h>
#include <kprocess.h>

#include <qbuffer.h>

using namespace KPIM;

SendmailJob::SendmailJob(Transport * transport, QObject * parent) :
    TransportJob( transport, parent )
{
  mProcess = new KProcess( this );
  connect( mProcess, SIGNAL(processExited(KProcess*)), SLOT(sendmailExited()) );
  connect( mProcess, SIGNAL(wroteStdin(KProcess*)), SLOT(wroteStdin()) );
  connect( mProcess, SIGNAL(receivedStderr(KProcess*,char*,int)),
           SLOT(receivedStdErr(KProcess*,char*,int)) );
}

SendmailJob::~ SendmailJob()
{
  delete mProcess;
}

void SendmailJob::doStart()
{
  *mProcess << transport()->host() << "-i" << "-f" << sender() << to() << cc() << bcc();
  if ( !mProcess->start( KProcess::NotifyOnExit, KProcess::All ) ) {
    setError( UserDefinedError );
    setErrorText( i18n("Failed to execute mailer program %1", transport()->host()) );
    emitResult();
  }
  setTotalSize( data().length() );
  wroteStdin();
}

void SendmailJob::sendmailExited()
{
  if ( !mProcess->normalExit() || !mProcess->exitStatus() == 0 ) {
    setError( UserDefinedError );
    setErrorText( i18n("Sendmail exited abnormally: %1", mLastError) );
  }
  emitResult();
}

void SendmailJob::wroteStdin()
{
  setProcessedSize( buffer()->pos() );
  if ( buffer()->atEnd() ) {
    mProcess->closeStdin();
  } else {
    QByteArray data = buffer()->read( 1024 );
    mProcess->writeStdin( data.constData(), data.length() );
  }
}

void SendmailJob::receivedStdErr(KProcess * proc, char * data, int len)
{
  Q_ASSERT( proc == mProcess );
  mLastError += QString::fromLocal8Bit( data, len );
}

bool SendmailJob::doKill()
{
  delete mProcess;
  mProcess = 0;
  return true;
}

#include "sendmailjob.moc"
