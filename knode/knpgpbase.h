/** KPGP: Pretty good privacy en-/decryption class
 *        This code is under GPL V2.0
 *
 * @author Lars Knoll <knoll@mpi-hd.mpg.de>
 *
 * GNUPG support
 * @author "J. Nick Koston" <bdraco@the.system.is.halted.net> 
 *
 * PGP6 and other enhancements
 * @author Andreas Gungl <Andreas.Gungl@osp-dd.de>
 *
 * code borrowed and changed for knode
 * @author Mathias Waack <mathias@atoll-net.de>
 */

#ifndef KNPGPBASE_H
#define KNPGPBASE_H

#include <qstring.h>
#include <qstrlist.h>

class KNPgpBase
{
public:

  enum{ 
    OK = 0,
      CLEARTEXT   =  0,
      RUN_ERR     =  1,
      ERROR       =  1,
      ENCRYPTED   =  2,
      SIGNED      =  4,
      GOODSIG     =  8,
      ERR_SIGNING = 16,
      UNKNOWN_SIG = 32,
      BADPHRASE   = 64,
      BADKEYS     =128,
      NO_SEC_KEY  =256, 
      MISSINGKEY  =512 };
  
  /** virtual class used internally by kpgp */
  KNPgpBase();
  virtual ~KNPgpBase();
  
  //virtual void readConfig();
  //virtual void writeConfig(bool sync = false);

  /** set and retrieve the message to handle */
  virtual bool setMessage(const QString mess);
  virtual QString message() const;

  /** manipulating the message */
  virtual int encrypt(const QStrList *, bool = false) { return OK; };
  virtual int sign(const char *) { return OK; };
  virtual int encsign(const QStrList *, const char * = 0,
		      bool = false) { return OK; };
  virtual int decrypt(const char * = 0) { return OK; };
  virtual QStrList pubKeys() { return OK; };
  virtual QString getAsciiPublicKey(QString) { return QString::null; };
  virtual int signKey(const char *, const char *) { return OK; };

  /** various functions to get the status of a message */
  bool isEncrypted() const;
  bool isSigned() const;
  bool isSigGood() const;
  bool unknownSigner() const;
  int getStatus() const;
  QString signedBy() const;
  QString signedByKey() const;
  const QStrList *receivers() const;

  virtual QString lastErrorMessage() const;

  /** functions that modify the behaviour of kpgp */
  void setUser(QString aUser);
  const QString user() const;
  void setEncryptToSelf(bool flag);
  bool encryptToSelf(void) const;

  /// kpgp needs to access this function
  void clearOutput();

protected:
  virtual int run(const char *cmd, const char *passphrase = 0);
  virtual int run(const QString& cmd, const char *passphrase = 0) {
    return run(cmd.latin1(),passphrase);
  }
  virtual int runGpg(const char *cmd, const char *passphrase = 0);
  virtual int runGpg(const QString& cmd, const char *passphrase = 0) {
    return runGpg(cmd.latin1(),passphrase);
  }
  virtual void clear();

  QString addUserId();

  QString input;
  QString output;
  QString info;
  QString errMsg;
  QString pgpUser;
  QString signature;
  QString signatureID;
  QStrList recipients;

  bool flagEncryptToSelf;
  int status;

};

// ---------------------------------------------------------------------------

class KNPgpBase2 : public KNPgpBase
{

public:
  KNPgpBase2();
  virtual ~KNPgpBase2();

  virtual int encrypt(const QStrList *recipients,
		      bool ignoreUntrusted = false);
  virtual int sign(const char *passphrase);
  virtual int encsign(const QStrList *recipients, const char *passphrase = 0,
		      bool ingoreUntrusted = false);
  virtual int decrypt(const char *passphrase = 0);
  virtual QStrList pubKeys();
  virtual QString getAsciiPublicKey(QString _person);
  virtual int signKey(const char *key, const char *passphrase);
};

class KNPgpBaseG : public KNPgpBase
{

public:
  KNPgpBaseG();
  virtual ~KNPgpBaseG();

  virtual int encrypt(const QStrList *recipients,
		      bool ignoreUntrusted = false);
  virtual int sign(const char *passphrase);
  virtual int encsign(const QStrList *recipients, const char *passphrase = 0,
		      bool ingoreUntrusted = false);
  virtual int decrypt(const char *passphrase = 0);
  virtual QStrList pubKeys();
  virtual QString getAsciiPublicKey(QString _person);
  virtual int signKey(const char *key, const char *passphrase);
};


class KNPgpBase5 : public KNPgpBase
{

public:
  KNPgpBase5();
  virtual ~KNPgpBase5();

  virtual int encrypt(const QStrList *recipients,
		      bool ignoreUntrusted = false);
  virtual int sign(const char *passphrase);
  virtual int encsign(const QStrList *recipients, const char *passphrase = 0,
		      bool ingoreUntrusted = false);
  virtual int decrypt(const char *passphrase = 0);
  virtual QStrList pubKeys();
  virtual QString getAsciiPublicKey(QString _person);
  virtual int signKey(const char *key, const char *passphrase);
};


class KNPgpBase6 : public KNPgpBase2
{

public:
  KNPgpBase6();
  virtual ~KNPgpBase6();

  virtual int decrypt(const char *passphrase = 0);
  virtual QStrList pubKeys();

  virtual int isVersion6();
};

// ---------------------------------------------------------------------------
// inlined functions

inline void
KNPgpBase::setUser(QString aUser)
{
  pgpUser = aUser;
}

inline const QString
KNPgpBase::user() const
{
  return pgpUser;
}

inline void 
KNPgpBase::setEncryptToSelf(bool flag)
{
  flagEncryptToSelf = flag;
}

inline bool 
KNPgpBase::encryptToSelf(void) const
{
  return flagEncryptToSelf;
}

inline bool 
KNPgpBase::isEncrypted() const
{
  if( status & ENCRYPTED )
    return true;
  return false;
}

inline bool 
KNPgpBase::isSigned() const
{
  if( status & SIGNED )
    return true;
  return false;
}

inline bool 
KNPgpBase::isSigGood() const
{
  if( status & GOODSIG )
    return true;
  return false;
}

inline bool 
KNPgpBase::unknownSigner() const
{
  if( status & UNKNOWN_SIG )
    return true;
  return false;
}

inline const QStrList *
KNPgpBase::receivers() const
{
  if(recipients.count() <= 0) return 0;
  return &recipients;
}

inline int
KNPgpBase::getStatus() const
{
  return status;
}

inline QString 
KNPgpBase::signedBy(void) const
{
  return signature;
}

inline QString 
KNPgpBase::signedByKey(void) const
{
  return signatureID;
}

#endif
