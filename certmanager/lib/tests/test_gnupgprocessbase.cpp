/*
    gnupgviewer.cpp

    This file is part of libkleopatra's test suite.
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gnupgviewer.h"

#include <backends/qgpgme/gnupgprocessbase.h>

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include <qstringlist.h>

GnuPGViewer::GnuPGViewer( QWidget * parent, const char * name )
  : QTextEdit( parent, name ), mProcess( 0 )
{
  setTextFormat( LogText );
  setMaxLogLines( 10000 );
}

GnuPGViewer::~GnuPGViewer() {
  if ( mProcess )
    mProcess->kill();
}

void GnuPGViewer::setProcess( Kleo::GnuPGProcessBase * process ) {
  if ( !process )
    return;
  mProcess = process;
  connect( mProcess, SIGNAL(processExited(KProcess*)),
	   SLOT(slotProcessExited(KProcess*)) );
  connect( mProcess, SIGNAL(receivedStdout(KProcess*,char*,int)),
	   SLOT(slotStdout(KProcess*,char*,int)) );
  connect( mProcess, SIGNAL(receivedStderr(KProcess*,char*,int)),
	   SLOT(slotStderr(KProcess*,char*,int)) );
  connect( mProcess, SIGNAL(status(Kleo::GnuPGProcessBase*,const QString&,const QStringList&)),
	   SLOT(slotStatus(Kleo::GnuPGProcessBase*,const QString&,const QStringList&)) );
}

static QStringList split( char * buffer, int buflen, QString & old ) {
  // when done right, this would need to use QTextCodec...
  const QString str = old + QString::fromLocal8Bit( buffer, buflen );
  QStringList l = QStringList::split( '\n', str, true );
  if ( l.empty() )
    return l;
  if ( str.endsWith( "\n" ) ) {
    old = QString::null;
  } else {
    old = l.back();
    l.pop_back();
  }
  return l;
}

static QString escape( QString str ) {
  return str.replace( '&', "&amp" ).replace( '<', "&lt;" ).replace( '>', "&gt;" );
}

void GnuPGViewer::slotStdout( KProcess *, char * buffer, int buflen ) {
  const QStringList l = split( buffer, buflen, mLastStdout );
  for ( QStringList::const_iterator it = l.begin() ; it != l.end() ; ++it )
    append( "stdout: " + escape( *it ) );
}

void GnuPGViewer::slotStderr( KProcess *, char * buffer, int buflen ) {
  const QStringList l = split( buffer, buflen, mLastStderr );
  for ( QStringList::const_iterator it = l.begin() ; it != l.end() ; ++it )
    append( "<b>stderr: " + escape( *it ) + "</b>" );
}
void GnuPGViewer::slotStatus( Kleo::GnuPGProcessBase *, const QString & type, const QStringList & args ) {
  append( "<b><font color=\"red\">status: " + escape( type + ' ' + args.join( " " ) ) + "</font></b>" );
}
void GnuPGViewer::slotProcessExited( KProcess * proc ) {
  if ( !proc )
    return;
  if ( proc->normalExit() )
    append( QString( "<b>Process exit: return code %1</b>" ).arg ( proc->exitStatus() ) );
  else
    append( "<b>Process exit: killed</b>" );
}

int main( int argc, char** argv ) {
  if ( argc < 3 ) {
    kdDebug() << "Need at least two arguments" << endl;
    return 1;
  }
  KAboutData aboutData( "test_gnupgprocessbase", "GnuPGProcessBase Test", "0.1" );
  KCmdLineArgs::init( &aboutData );
  KApplication app;

  Kleo::GnuPGProcessBase gpg;
  for ( int i = 1 ; i < argc ; ++i )
    gpg << argv[i];

  gpg.setUseStatusFD( true );

  GnuPGViewer * gv = new GnuPGViewer();
  gv->setProcess( &gpg );

  app.setMainWidget( gv );
  gv->show();

  gpg.start( KProcess::NotifyOnExit, KProcess::AllOutput );

  return app.exec();
}

#include "gnupgviewer.moc"
