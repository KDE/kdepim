/* This file is part of the KDE libraries
    Copyright (C) 2002 Holger Freyther <freyher@kde.org>

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

#include <kdebug.h>
#include <kapplication.h>
#include <qdatastream.h>
#include <qcstring.h>
#include <dcopclient.h>

#include "ksharedfiledevice.h"


KSharedFileDevice::KSharedFileDevice() : QFile()
{

}
KSharedFileDevice::KSharedFileDevice(const QString &name ) : QFile(name )
{

}
KSharedFileDevice::~KSharedFileDevice()
{

}
/** First look if the user want's to open ReadOnly
 *  and WriteOnly/WriteRead. Then we'll
 */
bool KSharedFileDevice::open( int mode )
{
  //init();
  setMode( mode );
  qWarning("KSharedfdileDevice::open" );
  //if( mode & IO_WriteOnly || mode & IO_ReadWrite ){ qt is insane
  if( isWritable() || isReadWrite() ) {
    qWarning("ReadWrite" );
    QByteArray data;
    QByteArray replyData;
    QCString replyType;
    QDataStream arg(data, IO_WriteOnly );
    arg << name();
    if(kapp->dcopClient()->call( "kded", "ksharedfile",
				 "writeLockFile(QString)",
				 data,
				 replyType, replyData ) ) {
      QDataStream res( replyData, IO_ReadOnly );
      bool ok;
      res >> ok;
      if( ok ){
	if( QFile::open(mode ) ){
	  qWarning("open worked" );
	  return true;
	}else{
	  qWarning("open failed" );
	  return false;
	}
      }else{
	kapp->dcopClient()->call( "kded", "ksharedfile",
				  "writeUnlockFile(Qtring)",
				  data, replyType, replyData);
	return false;
      }
    }else {
      return false;
    }
  }else if( isReadable() ) {
    qWarning("ReadOnly" );
    QByteArray data;
    QByteArray replyData;
    QCString replyType;
    QDataStream arg(data, IO_WriteOnly );
    arg << name();
    if(kapp->dcopClient()->call( "kded", "ksharedfile",
				 "readShareFile(QString)",
				 data,
				 replyType, replyData ) ) {
      QDataStream res( replyData, IO_ReadOnly );
      bool ok=false;
      res >> ok;
      if( ok ){
	if( QFile::open(mode ) ){
	  qWarning("open worked" );
	  return true;
	}else{
	  qWarning("hmm couldn't open" );
	  return false;
	}
      }else{
	kapp->dcopClient()->call( "kded", "ksharedfile",
				  "readUnshareFile(Qtring)",
				  data, replyType, replyData);
	return false;
      }
    }else {
      return false;
    }
  }
  return false;
}
void KSharedFileDevice::close(  )
{
  if(!isOpen() )
    return;
  qWarning("KSharedDevice::close %s", name().latin1() );
  if( isWritable() || isReadWrite() ){
    QByteArray data;
    QByteArray replyData;
    QCString replyType;
    QDataStream arg(data, IO_WriteOnly );
    arg << name();
    kapp->dcopClient()->call( "kded", "ksharedfile",
				 "writeUnlockFile(QString)",
				 data,
				 replyType, replyData );
  }else{
    QByteArray data;
    QByteArray replyData;
    QCString replyType;
    QDataStream arg(data, IO_WriteOnly );
    arg << name();
    kapp->dcopClient()->call( "kded", "ksharedfile",
				 "readUnshareFile(QString)",
				 data,
				 replyType, replyData );
  }
}


