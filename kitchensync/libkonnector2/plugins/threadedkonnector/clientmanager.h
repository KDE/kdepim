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

#ifndef CLIENTMANAGER_H
#define CLIENTMANAGER_H

#include <qobject.h>
#include <qapplication.h>
#include <qthread.h>
#include <qmutex.h>
#include <qwaitcondition.h>

#include <synceelist.h>
#include <error.h>
#include <progress.h>

#include "clientthread.h"

namespace Threaded {

  /**
     Client Thread. A QThread for the OBEX client.
     Note that this QObject is only accessed from the main thread. It is used
     to report events from the working thread.
   */
  class ClientManager
    : public QObject, public QThread {
    Q_OBJECT;
  public:
    /**
       Constructor with the usual parent handling.
     */
    ClientManager( QObject* = 0, const char* = 0 );
    /**
       Destructor, make sure that the thread terminated.
     */
    ~ClientManager();
    
    /**
       Returns if the worker is idel at the monment.
       Since we have only one manager this lock free access does not introduce
       a race condition.
     */
    bool isIdle();

    /**
       Starts reading the device and computing the synccees.
       Returns if a backup could be started. If the function returns with true
       the job is terminated by by either a @ref signalError( const KSync::Error& )
       or a @ref signalFinished( const KSync::Progress& ).
     */
    bool readSyncees();
    /**
       Starts writing the syncees to the device.
       Returns if a backup could be started. If the function returns with true
       the job is terminated by by either a @ref signalError( const KSync::Error& )
       or a @ref signalFinished( const KSync::Progress& ).
     */
    bool writeSyncees();

    /**
       Starts connecting the device.
       Returns if a backup could be started. If the function returns with true
       the job is terminated by by either a @ref signalError( const KSync::Error& )
       or a @ref signalFinished( const KSync::Progress& ).
     */
    bool connectDevice();
    /**
       Starts disconnecting the device.
       Returns if a backup could be started. If the function returns with true
       the job is terminated by by either a @ref signalError( const KSync::Error& )
       or a @ref signalFinished( const KSync::Progress& ).
     */
    bool disconnectDevice();

    /**
       Terminates ...
       FIXME, work out if the usual terminate does a better job ...
     */
    bool terminateThread();
    
    /**
       Sets a flag in the client thread. We hope, that the worker thread
       regularly reads that flag ...
     */
    void cancelJob();
    
  signals:
    /**
       Is emitted when the thread running has terminated.
       Then it is save to destroy this class.
     */
    void signalTerminated();
    /**
       Is emitted when the job running has finished.
     */
    void signalFinished();
    /**
       Is emitted when the thread running main gets an error.
     */
    void signalError( const KSync::Error& );
    /**
       Is emitted when the thread running main gets an error.
     */
    void signalProgress( const KSync::Progress& );

  private:
    /**
       Worker function of QThread.
       It runs the requests and posts events.
     */
    void run();

    /**
       Called by QObject to report that an event has arrived.
     */
    void customEvent( QCustomEvent* );

    /**
       The class where the actual work happens.
     */
    ClientThread mWorker;
  };

} // namespace Threaded

#endif
