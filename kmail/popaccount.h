// -*- c++ -*-
#ifndef KMAcctExpPop_h
#define KMAcctExpPop_h

#include "networkaccount.h"

#include <tqvaluelist.h>
#include <tqstringlist.h>
#include <tqvaluevector.h>
#include <tqtimer.h>
#include <tqdict.h>

class KMPopHeaders;
class KMMessage;
class QDataStream;
namespace KIO {
  class MetaData;
  class Slave;
  class SimpleJob;
  class Job;
}

/** The namespace where all classes of KMail can be found in. */
namespace KMail {
/**
 * KMail account for pop mail account
 */
class PopAccount: public NetworkAccount {
  Q_OBJECT

public:
  virtual ~PopAccount();
  virtual void init(void);

  virtual KIO::MetaData slaveConfig() const;

  /** A weak assignment operator */
  virtual void pseudoAssign( const KMAccount * a );

  virtual TQString protocol() const;
  virtual unsigned short int defaultPort() const;

  /**
   * Sending of several commands at once
   */
  bool usePipelining(void) const { return mUsePipelining; }
  virtual void setUsePipelining(bool);
 
  /**
   * Shall messages be left on the server upon retreival (TRUE)
   * or deleted (FALSE).
   */
  bool leaveOnServer(void) const { return mLeaveOnServer; }
  virtual void setLeaveOnServer(bool);

  /**
   * If value is positive, leave mail on the server for so many days.
   */
  int leaveOnServerDays(void) const { return mLeaveOnServerDays; }
  virtual void setLeaveOnServerDays(int);

  /**
   * If value is positive, leave so many messages on the server.
   */
  int leaveOnServerCount(void) const { return mLeaveOnServerCount; }
  virtual void setLeaveOnServerCount(int);

  /**
   * If value is positive, leave so many MBs on the server.
   */
  int leaveOnServerSize(void) const { return mLeaveOnServerSize; }
  virtual void setLeaveOnServerSize(int);

  /**
   * Shall messages be filter on the server (TRUE)
   * or not (FALSE).
   */
  bool filterOnServer(void) const { return mFilterOnServer; }
  virtual void setFilterOnServer(bool);

  /**
   * Size of messages which should be check on the
   * pop server before download
   */
  unsigned int filterOnServerCheckSize(void) const { return mFilterOnServerCheckSize; }
  virtual void setFilterOnServerCheckSize(unsigned int);

  /**
   * Inherited methods.
   */
  virtual TQString type(void) const;
  virtual void readConfig(KConfig&);
  virtual void writeConfig(KConfig&);
  virtual void processNewMail(bool _interactive);

  virtual void killAllJobs( bool disconnectSlave=false ); // NOOP currently

protected:
  enum Stage { Idle, List, Uidl, Head, Retr, Dele, Quit };
  friend class ::AccountManager;
  PopAccount(AccountManager* owner, const TQString& accountName, uint id);

  /**
   * Start a KIO Job to get a list of messages on the pop server
   */
  void startJob();

  /**
   * Connect up the standard signals/slots for the KIO Jobs
   */
  void connectJob();

  /**
   * Process any queued messages
   */
  void processRemainingQueuedMessages();

  /**
   * Save the list of seen uids for this user/server
   */
  void saveUidList();

  bool    mUsePipelining;
  bool    mLeaveOnServer;
  int     mLeaveOnServerDays;
  int     mLeaveOnServerCount;
  int     mLeaveOnServerSize;
  bool    gotMsgs;
  bool    mFilterOnServer;
  unsigned int mFilterOnServerCheckSize;

  KIO::SimpleJob *job;
  //Map of ID's vs. sizes of messages which should be downloaded
  TQMap<TQString, int> mMsgsPendingDownload;

  TQPtrList<KMPopHeaders> headersOnServer;
  TQPtrListIterator<KMPopHeaders> headerIt;
  bool headers;

  TQMap<TQString, bool> mHeaderDeleteUids;
  TQMap<TQString, bool> mHeaderDownUids;
  TQMap<TQString, bool> mHeaderLaterUids;

  TQStringList idsOfMsgs; //used for ids and for count
  TQValueList<int> lensOfMsgs;
  TQMap<TQString, TQString> mUidForIdMap; // maps message ID (i.e. index on the server) to UID
  TQDict<int> mUidsOfSeenMsgsDict; // set of UIDs of previously seen messages (for fast lookup)
  TQDict<int> mUidsOfNextSeenMsgsDict; // set of UIDs of seen messages (for the next check)
  TQValueVector<int> mTimeOfSeenMsgsVector; // list of times of previously seen messages
  TQMap<TQString, int> mTimeOfNextSeenMsgsMap; // map of uid to times of seen messages
  TQDict<int> mSizeOfNextSeenMsgsDict;
  TQStringList idsOfMsgsToDelete;
  TQStringList idsOfForcedDeletes;
  int indexOfCurrentMsg;

  TQValueList<KMMessage*> msgsAwaitingProcessing;
  TQStringList msgIdsAwaitingProcessing;
  TQStringList msgUidsAwaitingProcessing;

  TQByteArray curMsgData;
  TQDataStream *curMsgStrm;

  int curMsgLen;
  Stage stage;
  TQTimer processMsgsTimer;
  int processingDelay;
  int numMsgs, numBytes, numBytesToRead, numBytesRead, numMsgBytesRead;
  bool interactive;
  bool mProcessing;
  bool mUidlFinished;
  int dataCounter;

protected slots:
  /**
   * Messages are downloaded in the background and then once every x seconds
   * a batch of messages are processed. Messages are processed in batches to
   * reduce flicker (multiple refreshes of the qlistview of messages headers
   * in a single second causes flicker) when using a fast pop server such as
   * one on a lan.
   *
   * Processing a message means applying KMAccount::processNewMsg to it and
   * adding its UID to the list of seen UIDs
   */
  void slotProcessPendingMsgs();

  /**
   * If there are more messages to be downloaded then start a new kio job
   * to get the message whose id is at the head of the queue
   */
  void slotGetNextMsg();

  /**
   * A messages has been retrieved successfully. The next data belongs to the
   * next message.
   */
  void slotMsgRetrieved(KIO::Job*, const TQString &);

  /**
   * New data has arrived append it to the end of the current message
   */
  void slotData( KIO::Job*, const TQByteArray &);

  /**
   * Finished downloading the current kio job, either due to an error
   * or because the job has been canceled or because the complete message
   * has been downloaded
   */
  void slotResult( KIO::Job* );

  /**
   * Cleans up after a user cancels the current job
   */
  void slotCancel();

  /**
   * Kills the job if still stage == List
   */
  void slotAbortRequested();

  /**
   * Called when a job is finished. Basically a finite state machine for
   * cycling through the Idle, List, Uidl, Retr, Quit stages
   */
  void slotJobFinished();

  /**
   * Slave error handling
   */
  void slotSlaveError(KIO::Slave *, int, const TQString &);

  /**
   * If there are more headers to be downloaded then start a new kio job
   * to get the next header
   */
  void slotGetNextHdr();
};

} // namespace KMail



#endif /*KMAcctExpPop_h*/
