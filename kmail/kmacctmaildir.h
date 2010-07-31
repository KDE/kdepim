/* KMail account for maildir mail folders
 *
 */
#ifndef kmacctmaildir_h
#define kmacctmaildir_h

#include "kmaccount.h"
#include "kmglobal.h"

class KMAcctMaildir: public KMAccount
{
protected:
  friend class ::AccountManager;

  KMAcctMaildir(AccountManager* owner, const TQString& accountName, uint id);

public:
  virtual ~KMAcctMaildir();
  virtual void init(void);
  virtual void pseudoAssign( const KMAccount * a );

  /** Access to location of maildir mail file (usually something like
   "/home/joe/Maildir"). */
  const TQString& location(void) const { return mLocation; }
  virtual void setLocation(const TQString&);

  virtual TQString type(void) const;
  virtual void processNewMail(bool);
  virtual void readConfig(KConfig&);
  virtual void writeConfig(KConfig&);

protected:
  TQString mLocation;
  bool hasNewMail;
};

#endif /*kmacctmaildir_h*/
