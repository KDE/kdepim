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

#include <error.h>
#include <progress.h>

#include "clientthread.h"
#include "clientmanager.h"

using namespace Threaded;

/**
   Constructor with the usual parent handling.
*/
ClientManager::ClientManager( QObject* parent, const char* name )
  : QObject( parent, name ), mWorker( this )
{
  mWorker.mLock.lock();
  start();
}

/**
   Destructor, make sure that the thread terminated.
*/
ClientManager::~ClientManager()
{
  // Terminate the worker thread if required.
  if ( !finished() ) {
    // Tell the thread to terminate ...
    terminateThread();
    // ... and give it 5 seconds to terminate.
    if ( !wait( 5000 ) ) {
      // If it has not worked, try the hard way ...
      terminate();
      wait();
    }
  }
}

/**
   Worker function of QThread.
   It runs the requests and posts events.
*/
void ClientManager::run()
{
  // Call the real working routine.
  mWorker.run();
}

/**
   Returns if the worker is idel at the monment.
   Since we have only one manager this lock free access does not introduce
   a race condition.
*/
bool ClientManager::isIdle()
{
  return !mWorker.mLock.locked();
}

/**
   Starts reading the device and computing the synccees.
   Returns if a backup could be started. If the function returns with true
   the job is terminated by by either a @ref signalError( const KSync::Error& )
   or a @ref signalFinished( const KSync::Progress& ).
*/
bool ClientManager::readSyncees()
{
  if ( mWorker.mLock.tryLock() ) {
    mWorker.mFilename = QString::null;
    mWorker.mCommand = ClientThread::ReadSyncees;
    mWorker.mLock.unlock();
    mWorker.mWait.wakeOne();
    return true;
  } else
    return false;
}

/**
   Starts writing the syncees to the device.
   Returns if a backup could be started. If the function returns with true
   the job is terminated by by either a @ref signalError( const KSync::Error& )
   or a @ref signalFinished( const KSync::Progress& ).
*/
bool ClientManager::writeSyncees()
{
  if ( mWorker.mLock.tryLock() ) {
    mWorker.mFilename = QString::null;
    mWorker.mCommand = ClientThread::WriteSyncees;
    mWorker.mLock.unlock();
    mWorker.mWait.wakeOne();
    return true;
  } else
    return false;
}

/**
   Starts connecting the device.
   Returns if a backup could be started. If the function returns with true
   the job is terminated by by either a @ref signalError( const KSync::Error& )
   or a @ref signalFinished( const KSync::Progress& ).
*/

bool ClientManager::connectDevice()
{
  if ( mWorker.mLock.tryLock() ) {
    mWorker.mFilename = QString::null;
    mWorker.mCommand = ClientThread::Connect;
    mWorker.mLock.unlock();
    mWorker.mWait.wakeOne();
    return true;
  } else
    return false;
}

/**
   Starts disconnecting the device.
   Returns if a backup could be started. If the function returns with true
   the job is terminated by by either a @ref signalError( const KSync::Error& )
   or a @ref signalFinished( const KSync::Progress& ).
*/

bool ClientManager::disconnectDevice()
{
  if ( mWorker.mLock.tryLock() ) {
    mWorker.mFilename = QString::null;
    mWorker.mCommand = ClientThread::Disconnect;
    mWorker.mLock.unlock();
    mWorker.mWait.wakeOne();
    return true;
  } else
    return false;
}

/**
   Terminates ...
*/
bool ClientManager::terminateThread()
{
  mWorker.mCancel = true;
  if ( mWorker.mLock.tryLock() ) {
    mWorker.mFilename = QString::null;
    mWorker.mCommand = ClientThread::TerminateThread;
    mWorker.mLock.unlock();
    mWorker.mWait.wakeOne();
    return true;
  } else
    return false;
}

/**
   Sets a flag in the client thread. We hope, that the worker thread
   regularly reads that flag ...
*/
void ClientManager::cancelJob()
{
  mWorker.mCancel = true;
}

/**
   Called by QObject to report that an event has arrived.
*/
void ClientManager::customEvent( QCustomEvent* ce )
{
  if ( ce->type() == QEvent::Type( ClientThread::ProgressEvent ) ) {
    // This should occur most often therefore it is the first ...
    // FIXME
//     KSync::Progress* p = dynamic_cast<KSync::Progress *>( ce->data() );
    KSync::Progress* p = static_cast<KSync::Progress *>( ce->data() );
    if ( p )
      emit signalProgress( *p );
    else
      emit signalError( KSync::Error( "Internal error" )  );

  } else if ( ce->type() == QEvent::Type( ClientThread::ErrorEvent ) ) {
    // FIXME
//     KSync::Error* e = dynamic_cast<KSync::Error *>( ce->data() );
    KSync::Error* e = static_cast<KSync::Error *>( ce->data() );
    if ( e )
      emit signalError( *e );
    else
      emit signalError( KSync::Error( "Internal error" )  );

  } else if ( ce->type() == QEvent::Type( ClientThread::FinishedEvent ) ) {
    emit signalFinished();

  } else if ( ce->type() == QEvent::Type( ClientThread::TerminatedEvent ) ) {
    // Make sure that we are really done ...
    wait();
    // ...
    emit signalTerminated();
    mWorker.mLock.unlock();
  }
}
#include "clientmanager.moc"
