/* KMail Mail Sender
 *
 * Author: Stefan Taferner <taferner@alpin.or.at>
 */
#ifndef __KMAIL_SENDER_P_H__
#define __KMAIL_SENDER_P_H__
#include "kmsender.h"

#include <tqstring.h>
#include <tqstringlist.h>
#include <tqobject.h>
#include <kio/global.h>
#include <kdeversion.h>

class KProcess;

namespace KIO {
  class Job;
  class TransferJob;
  class Slave;
}

class KMSendProc: public QObject
{
  Q_OBJECT

public:
  KMSendProc(KMSender*);
  virtual ~KMSendProc() {}

  /** Initialize sending of one or more messages. */
  void start() { emit started( doStart() ); }

  /** Send given message. May return before message is sent. */
  bool send( const TQString & sender, const TQStringList & to, const TQStringList & cc, const TQStringList & bcc, const TQByteArray & message ) {
    reset(); return doSend( sender, to, cc, bcc, message );
  }

  /** Cleanup after sending messages. */
  void finish() { doFinish(); deleteLater(); }

  /** Abort sending the current message. Sets mSending to false */
  virtual void abort() = 0;

  /** Returns TRUE if send was successful, and FALSE otherwise.
      Returns FALSE if sending is still in progress. */
  bool sendOk() const { return mSendOk; }

  /** Returns error message of last call of failed(). */
  TQString lastErrorMessage() const { return mLastErrorMessage; }

signals:
  /** Emitted when the current message is sent or an error occurred. */
  void idle();

  /** Emitted when the initialization sequence has finished */
  void started(bool);


protected:
  /** Called to signal a transmission error. The sender then
    calls finish() and terminates sending of messages.
    Sets mSending to FALSE. */
  void failed(const TQString &msg);

  /** Informs the user about what is going on. */
  void statusMsg(const TQString&);

private:
  void reset();

private:
  virtual void doFinish() = 0;
  virtual bool doSend( const TQString & sender, const TQStringList & to, const TQStringList & cc, const TQStringList & bcc, const TQByteArray & message ) = 0;
  virtual bool doStart() = 0;

protected:
  KMSender* mSender;
  TQString mLastErrorMessage;
  bool mSendOk : 1;
  bool mSending : 1;
};


//-----------------------------------------------------------------------------
class KMSendSendmail: public KMSendProc
{
  Q_OBJECT
public:
  KMSendSendmail(KMSender*);
  ~KMSendSendmail();
  void start();
  void abort();

protected slots:
  void receivedStderr(KProcess*,char*,int);
  void wroteStdin(KProcess*);
  void sendmailExited(KProcess*);

private:
  /** implemented from KMSendProc */
  void doFinish();
  /** implemented from KMSendProc */
  bool doSend( const TQString & sender, const TQStringList & to, const TQStringList & cc, const TQStringList & bcc, const TQByteArray & message );
  /** implemented from KMSendProc */
  bool doStart();

private:
  TQByteArray mMsgStr;
  char* mMsgPos;
  int mMsgRest;
  KProcess* mMailerProc;
};

//-----------------------------------------------------------------------------
class KMSendSMTP : public KMSendProc
{
Q_OBJECT
public:
  KMSendSMTP(KMSender *sender);
  ~KMSendSMTP();

  void abort();

private slots:
  void dataReq(KIO::Job *, TQByteArray &);
  void result(KIO::Job *);
  void slaveError(KIO::Slave *, int, const TQString &);

private:
  /** implemented from KMSendProc */
  void doFinish();
  /** implemented from KMSendProc */
  bool doSend( const TQString & sender, const TQStringList & to, const TQStringList & cc, const TQStringList & bcc, const TQByteArray & message );
  /** implemented from KMSendProc */
  bool doStart() { return true; }

  void cleanup();

private:
  TQByteArray mMessage;
  uint mMessageLength;
  uint mMessageOffset;

  bool mInProcess;

  KIO::TransferJob *mJob;
  KIO::Slave *mSlave;
};

#endif /* __KMAIL_SENDER_P_H__ */
