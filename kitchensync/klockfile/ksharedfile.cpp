/* This file is part of the KDE libraries
    Copyright (C) 2001 Holger Freyther <freyher@kde.org>
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
		  
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/
#include <sys/stat.h>
#include <unistd.h>


#include <kapplication.h>
#include <kdebug.h>

#include <qfile.h>
#include <qtimer.h>
#include <qtextstream.h>
#include "ksharedfile.h"

KSharedFile::KSharedFile( const QString &filename ) : QObject( 0, "KSharedFile" )
{
  m_locked = false;
  m_FileCheckTimer = new QTimer (this );
  connect( m_FileCheckTimer, SIGNAL( timeout() ), SLOT( checkFile()) );
  setFileName( filename);
}
KSharedFile::KSharedFile( const QFile &file ) : QObject( 0, "KSharedFile" )
{
  m_locked = false;
  m_FileCheckTimer = new QTimer(this  );
  connect( m_FileCheckTimer, SIGNAL( timeout() ), SLOT( checkFile()) );
  setFile( file);
}
KSharedFile::~KSharedFile( )
{
  unlockFile(new Ticket(m_fileName) );
  delete m_FileCheckTimer; // qt would do this too
}

void KSharedFile::setFileName ( const QString &filename )
{
  unlockFile(new Ticket(m_fileName) );
  m_lockedOther = false;
  m_fileName = filename;
  struct stat s;
  int result = stat( QFile::encodeName( m_fileName ), &s );
  if ( result == 0 ) { // file exists 
    m_ChangeTime = s.st_ctime; // last change
    m_FileCheckTimer->start( 500 );
  }
}
void KSharedFile::setFile( const QFile &file )
{
  setFileName( file.name() );
}
QString KSharedFile::fileName( )const 
{
  return m_fileName;
}


KSharedFile::Ticket *KSharedFile::requestSaveTicket( )
{ 
  kdDebug(5910) << "KSharedFile::tryLockFile " << m_fileName << endl; 
  if(!QFile::exists( m_fileName ) ) return 0l;  
  if(QFile::exists( m_fileName+".lock") ) return 0l;
  QString lockFile(m_fileName + ".lock" );

  //create unique file
  m_uniqueName =  m_fileName + kapp->randomString(8) ;
  QFile file( m_uniqueName);
  file.open( IO_WriteOnly );
  file.close( );

  // Create lock file
  int result = link ( QFile::encodeName( m_uniqueName ),
		      QFile::encodeName( m_fileName + ".lock" ));
  if( result == 0 )
  {
    emit fileLocked(m_fileName );
    m_locked = true;
    return new KSharedFile::Ticket(m_fileName );
  }
  return 0l;
}

bool KSharedFile::unlockFile( Ticket* ticket )
{
  if( m_locked && ticket->m_fileName==m_fileName){
    m_locked = false;
    unlink( QFile::encodeName(m_fileName+ ".lock") );
    QFile::remove( m_uniqueName );
    emit fileUnlocked( m_fileName );   
    delete ticket;
    ticket=0;
    return true;
  }else
    return false;
}
bool KSharedFile::isLocked( )
{
  //return m_locked;
  return QFile::exists(m_fileName + ".lock");
}
bool KSharedFile::didILock( )
{
  return m_locked;
}
QString KSharedFile::whoHoldsLock( ) const
{
  return QString();
}
bool KSharedFile::save( Ticket *ticket, const QString &string )
{
  if ( m_locked && ticket->m_fileName == m_fileName ){
    QFile file( m_fileName ); // handles unicode names inernal
    file.open(IO_WriteOnly);
    QTextStream stream( &file );
    //file.writeBlock( string ); //doesn't work
    stream << string;
    file.close();
    return true;
  }
  return false;
}
QString KSharedFile::load( )
{
  QString dummy;
  QFile file( m_fileName );
  file.open(IO_ReadOnly);
  dummy = QString(file.readAll() );
  file.close();
  return dummy;
}
void KSharedFile::checkFile( )
{ 
  // ok somebody holds the lock
  if( !m_lockedOther && !m_locked && QFile::exists( QFile::encodeName( m_fileName+ ".lock" ) ) ) {
    emit fileLocked( m_fileName );
    m_lockedOther= true; // other locked our file
    return; 
  }
  else if( m_lockedOther && !m_locked &&!QFile::exists( QFile::encodeName( m_fileName + ".lock" ) ) ) {
    emit fileUnlocked( m_fileName );
    m_lockedOther = false; // other
   
  }  
  struct stat s;
  int result = stat( QFile::encodeName( m_fileName ), &s );
  if ( result == 0 && (m_ChangeTime != s.st_ctime ) ){
    m_ChangeTime = s.st_ctime;
    emit fileChanged( m_fileName );
  }
  
}

#include "ksharedfile.moc"
