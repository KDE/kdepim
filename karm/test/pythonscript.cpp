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

#include "pythonscript.h"

#include <kdebug.h>
#include <kapplication.h>
#include <dcopclient.h>

#include <kgenericfactory.h>
#include <scriptclientinterface.h>

//typedef KGenericFactory<PythonScript, KScriptClientInterface> PythonScriptFactory;
//K_EXPORT_COMPONENT_FACTORY( libpythonscript, PythonScriptFactory( "PythonScript" ) );

PythonScript::PythonScript
( KScriptClientInterface *parent, const char *name, const QStringList &args )
{
  ScriptClientInterface = parent;
  m_proc =  new KProcess();
  connect ( m_proc, 
            SIGNAL( processExited( KProcess * ) ), 
            SLOT  ( Exit         ( KProcess * ) ) );
  connect ( m_proc, 
            SIGNAL( receivedStdout( KProcess *, char *, int ) ), 
            SLOT  ( stdOut        ( KProcess *, char *, int ) ) );
  connect ( m_proc, 
            SIGNAL( receivedStderr( KProcess *, char *, int ) ), 
            SLOT  ( stdErr        ( KProcess *, char *, int ) ) );

  // Connect feedback signals and slots
  kdDebug() << "Building new script engine (PythonScript)" << endl;
}

PythonScript::~PythonScript()
{
  kdDebug() << "Destroying script engine (PythonScript)" << endl;
}

QString PythonScript::script() const
{
//  return m_proc;
  kdDebug() << "return script path" << endl;
  return QString::null;
}

void PythonScript::setScript( const QString &scriptFile  )
{
  kdDebug() << "set script " << kapp->dcopClient()->appId() << " " << scriptFile << endl;
  *m_proc << "python" << scriptFile << kapp->dcopClient()->appId();
}

void PythonScript::setScript( const QString &, const QString & )
{
  // ### what is this?
}

void PythonScript::run(QObject * /*context*/, const QVariant &/*arg*/)
{
  kdDebug() << "running the script" << endl;
  m_proc->start(KProcess::NotifyOnExit,KProcess::All);
}
void PythonScript::kill()
{
  if ( !m_proc->kill() )    // Kill the process
    m_proc->kill( 9 );  // Kill it harder
}

void PythonScript::Exit(KProcess * /*proc*/)
{
  kdDebug () << "Done processing..." << endl;
//  ScriptClientInteface->done(proc->exitStatus());
}

void PythonScript::stdErr(KProcess * /*proc*/, char *buffer, int buflen)
{
  QString s( buffer );
  s.truncate( buflen );
  kdDebug()<<"PythonScript: got error: "<<s<<endl;
}
void PythonScript::stdOut(KProcess * /*proc*/, char *buffer, int buflen)
{
  kdDebug() << "Feedback" << endl;
  char *data = (char *) malloc(buflen);
  //kdDebug() << data << endl;
  ScriptClientInterface->output("data");
  free(data);
}

#include "pythonscript.moc"
