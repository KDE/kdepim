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

#ifndef CLIENTTHREAD_H
#define CLIENTTHREAD_H

#include <qobject.h>
#include <qapplication.h>
#include <qthread.h>
#include <qmutex.h>
#include <qwaitcondition.h>

#include <synceelist.h>
#include <error.h>
#include <progress.h>

namespace Threaded {

  /**
     Client Thread. The Worker OBEX client thread.
     Note that this QObject is only accessed from the client thread. It is used
     to report events from the working thread. This QObject will be the parent of
     all QObjects in the working thread. It is in no way connected to the QObject
     in the main thread.
   */
  class ClientThread
    : public QObject {
    Q_OBJECT;
  public:
    /**
       Event numbers for the custom event.
     */
    enum { TerminatedEvent = 46666,
	   FinishedEvent = TerminatedEvent+1,
	   ErrorEvent = TerminatedEvent+2,
	   ProgressEvent = TerminatedEvent+3
    };
    /**
       Possible commands for the worker.
     */
    enum Command {
      Connect,
      Disconnect,
      Backup,
      Restore,
      ReadSyncees,
      WriteSyncees,
      TerminateThread
    };

    /**
       Constructor.
     */
    ClientThread( QObject* );
    /**
       Destructor.
     */
    ~ClientThread();
    
    /**
       Worker function of QThread.
       It runs the requests and posts events.
     */
    void run();

  private:
    /**
       Call in run if you have to report an error.
     */
    void finished();
    /**
       Call in run if you have to report an error.
     */
    void error( const KSync::Error* );
    /**
       Call in run if you want to report progress.
     */
    void progress( const KSync::Progress* );

    /**
       The object where signals schould be sent to.
     */
    QObject* mReciver;
    /**
       The command to process;
     */
    Command mCommand;
    /**
       The file where to write/read backups/restores.
     */
    QString mFilename;
    /**
       The synceelist to write/read to/from.
     */
    KSync::SynceeList mSynceeList;

    /**
       The three variables above are guarded by that lock.
     */
    QMutex mLock;
    /**
       Wait for a job.
     */
    QWaitCondition mWait;
    /**
       Set if the running request should be stopped.
     */
    bool mCancel;

    /**
       The class has to access several private variables.
     */
    friend class ClientManager;
  };
  
} // namespace Threaded

#endif
