/* This file is part of the KDE libraries
    Copyright (C) 2001, 2002 Holger Freyther <freyher@kde.org>
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

#include <dcopclient.h>
#include <kapplication.h>
#include <kdebug.h>


#include <qfile.h>
#include <qtimer.h>
#include <qtextstream.h>
#include "ksharedfile.h"


#include "ksharedfile.moc"

KSharedFile::KSharedFile( const QString &filename ) : QObject( 0, "KSharedFile" ),  DCOPObject("KSharedFileManager" )
{
  // dcop bits
  readLock = false;
  writeLock = false;
  setFileName( filename);
  connectDCOPSignal( "kded", "ksharedfile", // makes it even load
		     "fileLocked(QString)",
		     "slotFileLocked(QString)", false );
  connectDCOPSignal( "kded", "ksharedfile",
		     "fileUnlocked(QString)",
		     "slotFileUnlocked(QString)", false );
  connectDCOPSignal( "kded", "ksharedfile",
		     "fileChanged(QString)",
		     "slotFileChanged(QString)",  false );
}
KSharedFile::KSharedFile( const QFile &file ) : QObject( 0, "KSharedFile" )
{
  // dcop bits
  readLock = false;
  writeLock = false;
  setFile( file);
  connectDCOPSignal( "kded", "ksharedfile",
		     "fileLocked(QString)",
		     "slotFileLocked(QString)", false );
  connectDCOPSignal( "kded", "ksharedfile",
		     "fileUnlocked(QString)",
		     "slotFileUnlocked(QString)", false );
  connectDCOPSignal( "kded", "ksharedfile",
		     "fileChanged(QString)",
		     "slotFileChanged(QString)",  false );
  kdDebug(5910) << file.name() << endl;
}
KSharedFile::~KSharedFile( )
{ // remove all locks and interests
  updateLocks();
  kdDebug(5910) << "~KSharedFile" << endl;
}

void KSharedFile::setFileName ( const QString &filename )
{
  updateLocks();
 ////// interested in a new File
   m_fileName = filename;
  QByteArray data;
  QDataStream stream2(data, IO_WriteOnly );
  stream2 << m_fileName;
  kapp->dcopClient()->send("kded", "ksharedfile","interestedIn(QString)", data  );
}
void KSharedFile::setFile( const QFile &file )
{
  setFileName( file.name() );
}
QString KSharedFile::fileName( )const 
{
  return m_fileName;
}


KSharedFile::Ticket *KSharedFile::requestWriteTicket( )
{ 
  kdDebug(5910) << "KSharedFile::requestSaveTicket " << m_fileName << endl; 
  Ticket *ticket=0l;
  if(writeLock)
    return 0l; 
  // Create lock file
  // dcop bits
  QByteArray data;
  QByteArray replyData;
  QCString replyType;
  QDataStream arg(data, IO_WriteOnly );
  arg << m_fileName;
  if(kapp->dcopClient()->call( "kded", "ksharedfile",
			       "writeLockFile(QString)",
			       data,
			       replyType, replyData ) )
{
    qWarning("If" );
    if( replyType == "bool"){
      kdDebug(5910) << "reply" << endl;
      QDataStream result(replyData, IO_ReadOnly ); 
      bool ok;
      result >> ok;
      if( ok ){
	kdDebug(5910 ) << "request Worked" << endl;
	ticket = new Ticket(m_fileName );
	writeLock = true;
      }else{
	kdDebug(5910) << "failed to lock" << endl;
      }
    }
  }
  return ticket;
}

KSharedFile::Ticket *KSharedFile::requestReadTicket( )
{
  Ticket *ticket = 0l;
  kdDebug(5910) << "requestReadTicket" << endl;
  if(readLock)
    return ticket;
  kdDebug(5910) << "DCOP" << endl;
  QByteArray data;
  QByteArray replyData;
  QCString replyType;
  QDataStream arg(data, IO_WriteOnly );
  arg << m_fileName;
  if(kapp->dcopClient()->call( "kded", "ksharedfile",
			       "readShareFile(QString)",
			       data,
			       replyType, replyData ) )
    {
    qWarning("If" );
    if( replyType == "bool"){
      kdDebug(5910) << "bool" << endl;
      QDataStream result(replyData, IO_ReadOnly ); 
      bool ok;
      result >> ok;
      if( ok ){
	ticket = new Ticket(m_fileName );
	readLock = true;
	kdDebug(5910) <<"readLockFileOk" <<endl;
      }
      kdDebug(5910) << "bool end" << endl;
    }
  }
  return ticket;
}
bool KSharedFile::unlockWriteFile( Ticket* ticket )
{
  if( writeLock && ticket->m_fileName==m_fileName){
    QByteArray data;
    QByteArray replyData;
    QCString replyType;
    QDataStream arg(data, IO_WriteOnly );
    arg << m_fileName;
    if(kapp->dcopClient()->call( "kded", "ksharedfile",
				 "writeUnlockFile(QString)",
				 data,
				 replyType, replyData ) ){
      qWarning("If" );
      if( replyType == "bool"){
	QDataStream result(replyData, IO_ReadOnly );
	bool ok;
	result >> ok;
	if( ok ){
	  delete ticket;
	  writeLock = false;
	  if(m_file != 0 ){
	    delete m_file;
	  }
	  return true;
	}
      }
    }
  }
  return false;
}
bool KSharedFile::unlockReadFile( Ticket *ticket )
{
  kdDebug(5910) << "unlockReadFile" << endl; 
  if(ticket == 0l)
    return false;
  if (readLock && ticket->m_fileName == m_fileName ) {
    kdDebug(5910) << "going to unlock" << endl;
    readLock = false;
    QByteArray data;
    QByteArray replyData;
    QCString replyType;
    QDataStream arg(data, IO_WriteOnly );
    arg << m_fileName;
    if(kapp->dcopClient()->call( "kded", "ksharedfile",
				 "readUnshareFile(QString)",
				 data,
				 replyType, replyData ) ){
      qWarning("If" );
      if( replyType == "bool"){
        kdDebug(5910) << "unlock bool type" << endl;
	QDataStream result(replyData, IO_ReadOnly );
	bool ok;
	result >> ok;
	if( ok ){
	  delete ticket;
	  readLock = false;
	  kdDebug(5910) << "bool" <<endl;
	  return true;
	}
      }
    }
  }
  return false;
}
bool KSharedFile::canReadLock( )
{
  //return m_locked; dcop bits
  return true;
}
bool KSharedFile::canWriteLock()
{
  return true;
}
//QString KSharedFile::whoHoldsLock( ) const
//{
//  return QString();
//}
bool KSharedFile::save( Ticket *ticket, const QString &string )
{
  if ( writeLock && ticket->m_fileName == m_fileName ){
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
bool KSharedFile::save( Ticket *ticket, const QByteArray &array )
{
  if ( writeLock && ticket->m_fileName == m_fileName ){
    QFile file( m_fileName ); // handles unicode names inernal
    file.open(IO_WriteOnly);
    file.writeBlock(array );
    file.close();
    return true;
  }
  return false;

}
QFile* KSharedFile::save( Ticket *ticket )
{
  if ( writeLock && ticket->m_fileName == m_fileName ){
    m_file = new QFile(ticket->m_fileName );
    m_file->open(IO_WriteOnly );
    return m_file;
  }
  return 0l;
}
QString KSharedFile::readAsString( bool &ok, Ticket *ticket )
{
  if( readLock && ticket->m_fileName == m_fileName ){
    QString dummy;
    QFile file( m_fileName );
    file.open(IO_ReadOnly);
    dummy = QString(file.readAll() );
    file.close();
    ok=true;
    return dummy;
  }else {
    ok=false;
    return QString();
  }
}
QByteArray KSharedFile::readAsByteArray(bool &ok, Ticket *ticket )
{
  if( readLock && ticket->m_fileName == m_fileName ){
    QByteArray dummy;
    QFile file( m_fileName );
    file.open(IO_ReadOnly);
    dummy = file.readAll() ;
    file.close();
    ok=true;
    return dummy;
  }else {
    ok=false;
    return QByteArray();
  }
}
QFile* KSharedFile::readAsFile(Ticket *ticket)
{
  if ( readLock && ticket->m_fileName == m_fileName ){
    m_file = new QFile(ticket->m_fileName );
    m_file->open(IO_ReadOnly );
    return m_file;
  }
  return 0l;
}
bool KSharedFile::didIReadLock() 
{
  return readLock;
}
bool KSharedFile::didIWriteLock()
{
  return writeLock;
}
void KSharedFile::slotFileChanged(QString) {

}
void KSharedFile::updateLocks()
{
  if(m_fileName.isEmpty() )
    return;
  if( readLock ){
    QByteArray data;
    QByteArray replyData;
    QCString replyType;
    QDataStream arg(data, IO_WriteOnly );
    arg << m_fileName;
    kapp->dcopClient()->call( "kded", "ksharedfile",
				 "readUnlockFile(QString)",
				 data,
				 replyType, replyData );
    readLock=false;
  }else if(writeLock ){
    QByteArray data;
    QByteArray replyData;
    QCString replyType;
    QDataStream arg(data, IO_WriteOnly );
    arg << m_fileName;
    kapp->dcopClient()->call( "kded", "ksharedfile",
			      "readUnlockFile(QString)",
			      data,
			      replyType, replyData );
    writeLock=false; 
  }
  QByteArray data;
  QDataStream stream(data, IO_WriteOnly );
  stream << m_fileName;
  kapp->dcopClient()->send("kded", "ksharedfile","removeInterestIn(QString)", data  );
  if(m_file != 0 )
    delete m_file;
}









