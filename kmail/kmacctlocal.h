/* KMail account for local mail folders
 *
 */
#ifndef kmacctlocal_h
#define kmacctlocal_h

#include "kmaccount.h"
#include "kmglobal.h"

class KMAcctLocal: public KMAccount
{
protected:
  friend class ::AccountManager;

  KMAcctLocal(AccountManager* owner, const TQString& accountName, uint id);

public:
  virtual ~KMAcctLocal();
  virtual void init(void);

  virtual void pseudoAssign( const KMAccount * a );

  /** Access to location of local mail file (usually something like
   "/var/spool/mail/joe"). */
  TQString location(void) const { return mLocation; }
  virtual void setLocation(const TQString&);

  /** Acceso to Locking method */
  LockType lockType(void) const { return mLock; }
  void setLockType(LockType lt) { mLock = lt; }

  TQString procmailLockFileName(void) const { return mProcmailLockFileName; }
  void setProcmailLockFileName(const TQString& s);

  virtual TQString type(void) const;
  virtual void processNewMail(bool);
  virtual void readConfig(KConfig&);
  virtual void writeConfig(KConfig&);

private:
  bool preProcess();
  bool fetchMsg();
  void postProcess();

private:
  TQString mLocation;
  TQString mProcmailLockFileName;
  bool mHasNewMail;
  bool mAddedOk;
  LockType mLock;
  int mNumMsgs;
  int mMsgsFetched;
  KMFolder *mMailFolder;
  TQString mStatusMsgStub;
};

#endif /*kmacctlocal_h*/
