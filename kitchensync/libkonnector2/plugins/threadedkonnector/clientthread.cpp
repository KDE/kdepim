/*
    This file is part of KitchenSync.

    Copyright (c) 2003 Mathias Froehlich <Mathias.Froehlich@web.de>

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

#include <qobject.h>
#include <qapplication.h>
#include <qthread.h>

#include <kdebug.h>

#include <error.h>
#include <progress.h>

#include "clientthread.h"

using namespace Threaded;

/**
   Constructor.
*/
ClientThread::ClientThread( QObject* reciver )
  : QObject( 0, "Threaded test konnector" )
{
  mReciver = reciver;
  mCancel = false;
}

/**
   Destructor.
*/
ClientThread::~ClientThread()
{
}

/**
   Worker function of QThread.
   It runs the requests and posts events.
*/
void ClientThread::run()
{
  // Do the work here. No further communication here.
  for (;;) {
    mWait.wait( &mLock );
    mCancel = false;
    switch ( mCommand ) {
    case Connect:
      kdDebug() << "################################### Connect" << endl;
      finished();
      break;
    case Disconnect:
      kdDebug() << "################################### Disconnect" << endl;
      finished();
      break;
    case Backup:
      kdDebug() << "################################### Backup" << endl;
      finished();
      break;
    case Restore:
      kdDebug() << "################################### Restore" << endl;
      finished();
      break;
    case ReadSyncees:
      kdDebug() << "################################### ReadSyncee" << endl;
      finished();
      break;
    case WriteSyncees:
      kdDebug() << "################################### WriteSyncee" << endl;
      finished();
      break;
    default:
      // Signal that we are finished ...
      QApplication::postEvent( mReciver, new QCustomEvent( TerminatedEvent ) );
      return;
    }
  }
}

/**
   Call in run if you have to report an error.
*/
void ClientThread::finished()
{
  // Signal an error
  QApplication::postEvent( mReciver, new QCustomEvent( FinishedEvent ) );
}

/**
   Call in run if you have to report an error.
*/
void ClientThread::error( const KSync::Error* e )
{
  // Signal an error
  QCustomEvent* ce = new QCustomEvent( ErrorEvent );
  ce->setData( (void *)e );
  QApplication::postEvent( mReciver, ce );
}

/**
   Call in run if you want to report progress.
*/
void ClientThread::progress( const KSync::Progress* p )
{
  // Signal progress
  QCustomEvent* ce = new QCustomEvent( ProgressEvent );
  ce->setData( (void *)p );
  QApplication::postEvent( mReciver, ce );
}
