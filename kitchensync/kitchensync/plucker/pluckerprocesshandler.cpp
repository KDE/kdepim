/*
    This file is part of KitchenSync.

    Copyright (c) 2004 Holger Hans Peter Freyther <freyther@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/


#include "pluckerprocesshandler.h"
#include "pluckerconfig.h"

#include <kdebug.h>

namespace KSPlucker {
PluckerProcessHandler::PluckerProcessHandler( enum Mode m, bool forget,
                                const QString& file, QObject* p )
  : QObject( p ), m_mode( m ), m_forget( forget ),
    m_file( file ), m_useList( false )
{}

PluckerProcessHandler::PluckerProcessHandler( enum Mode m, bool forget,
                                const QStringList& file, const QString& dest,
                                QObject* p )
  : QObject( p ), m_mode( m ), m_forget( forget ),  m_dir( dest ),
    m_files( file ), m_useList( true )
{}

PluckerProcessHandler::~PluckerProcessHandler()
{}

void PluckerProcessHandler::runConfig( KProcess* proc )
{
  PluckerConfig *conf = PluckerConfig::self();
  *proc << conf->javaPath();
  *proc << "-jar" << conf->pluckerPath()+"/jpluckx.jar" << m_file;
}

void PluckerProcessHandler::runConvert( KProcess* proc )
{
  PluckerConfig *conf = PluckerConfig::self();
  *proc << conf->javaPath();
  *proc << "-jar" << conf->pluckerPath()+"/jpluckc.jar"
        << "-destination" << m_dir << m_file;
}

void PluckerProcessHandler::run()
{
  if ( m_useList )
    popFirst();

  KProcess *proc = new KProcess( this );

  /*
   * Set the Configuration
   */
  if ( m_mode == Configure )
    runConfig( proc );
  else
    runConvert( proc );

  connect(proc, SIGNAL(processExited(KProcess*)),
          this, SLOT(slotExited(KProcess*)) );
  connect(proc, SIGNAL(receivedStdout(KProcess*,char*,int)),
          this, SLOT(slotStdOutput(KProcess*,char*,int)) );


  if ( !proc->start(m_forget ? KProcess::DontCare        : KProcess::NotifyOnExit,
                    m_forget ? KProcess::NoCommunication : KProcess::AllOutput ) ) {
    kdDebug() << "Failed To Execute" << endl;
    emit sigFinished( this );
  }

}

void PluckerProcessHandler::slotExited( KProcess* proc )
{
  proc->deleteLater();

  if ( !m_useList || m_files.isEmpty() )
    emit sigFinished(this);
  else
    run();

}

void PluckerProcessHandler::slotStdOutput( KProcess* ,
                                    char* buffer, int buflen ) {
  QString str = QString::fromLatin1( buffer, buflen );
  emit sigProgress( str );
}

/*
 * For converting we need to be able to convert
 * a series of documents. So once one
 * Document is Finished we will continue with the next one
 */
void PluckerProcessHandler::popFirst()
{
  m_file = m_files.first();
  m_files.remove( m_file );
}

}

#include "pluckerprocesshandler.moc"
