/*
  broadcaststatus.h

  This file is part of KDEPIM.

  Copyright (C) 2000 Don Sanders <sanders@kde.org>

  License GPL
*/

#ifndef __kpim_broadcast_status_h
#define __kpim_broadcast_status_h

#include <tqobject.h>
#include <tqmap.h>

#include <kdepimmacros.h>

#undef None

namespace KPIM {

class ProgressItem;

/**
    Provides a singleton which broadcasts status messages by emitting
    signals. Interested mainwindows can connect to the statusMsg()
    signal and update statusBars or whatever they use for showing status.
  */


class KDE_EXPORT BroadcastStatus : public QObject
{

  Q_OBJECT

public:
  virtual ~BroadcastStatus();

  /** Return the instance of the singleton object for this class */
  static BroadcastStatus *instance();

  /** Return the last status message from setStatusMsg() */
  TQString statusMsg() const { return mStatusMsg; }
  /** Sets a status bar message with timestamp */
  void setStatusMsgWithTimestamp( const TQString& message );
  /** Sets a transmission completed status bar message */
  void setStatusMsgTransmissionCompleted( int numMessages,
                                          int numBytes = -1,
                                          int numBytesRead = -1,
                                          int numBytesToRead = -1,
                                          bool mLeaveOnServer = false,
                                          KPIM::ProgressItem* progressItem = 0 ); // set the same status in this progress item
  void setStatusMsgTransmissionCompleted( const TQString& account,
                                          int numMessages,
                                          int numBytes = -1,
                                          int numBytesRead = -1,
                                          int numBytesToRead = -1,
                                          bool mLeaveOnServer = false,
                                          KPIM::ProgressItem* progressItem = 0 ); // set the same status in this progress item

public slots:
  /** Emit an update status bar signal. It's a slot so it can be hooked up
      to other signals. */
  void setStatusMsg( const TQString& message );

  /**
      Set a status message that will go away again with the next call of
      reset().
   */
  void setTransientStatusMsg( const TQString& msg );
  /**
      Reset the status message to what ever non-transient message was last
      active or has since been set.
   */
  void reset();

signals:

  /** Emitted when setStatusMsg is called. */
  void statusMsg( const TQString& );

protected:

  BroadcastStatus();
  TQString mStatusMsg;
  bool mTransientActive;
  static BroadcastStatus* instance_;
};


}
#endif
