/** KPGP: Pretty good privacy en-/decryption class
 *        This code is under GPL V2.0
 *
 *
 * @author Lars Knoll <knoll@mpi-hd.mpg.de>
 *
 * GNUPG support
 * @author "J. Nick Koston" <bdraco@the.system.is.halted.net>
 *
 * PGP6 and other enhancements
 * @author Andreas Gungl <a.gungl@gmx.de>
 *
 * code borrowed;) and changed for knode
 * @author Mathias Waack <mathias@atoll-net.de>
 */

#include "knpgpbase.h"
#include "knpgp.h"

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>/** KPGP: Pretty good privacy en-/decryption class
 *        This code is under GPL V2.0
 *
 *
 * @author Lars Knoll <knoll@mpi-hd.mpg.de>
 *
 * GNUPG support
 * @author "J. Nick Koston" <bdraco@the.system.is.halted.net>
 *
 * PGP6 and other enhancements
 * @author Andreas Gungl <a.gungl@gmx.de>
 *
 * code borrowed;) and changed for knode
 * @author Mathias Waack <mathias@atoll-net.de>
 */

#include <qregexp.h>
#include <qcursor.h>
#include <qhbox.h> 
#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlayout.h>

#include <kdebug.h>
#include <kapp.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kconfig.h>


KNPgp *KNPgp::kpgpObject = 0L;

KNPgp::KNPgp()
  : publicKeys()
{
  kpgpObject=this;

  //config = KGlobal::config();
  //config->setGroup("PRIVACY");

  init();
}

KNPgp::~KNPgp()
{
  clear(TRUE);
  //delete config;
}

// ----------------- public methods -------------------------

void
KNPgp::init()
{
  havePassPhrase = FALSE;
  passphrase = QString::null;

  // do we have a pgp executable
  checkForPGP();

  if(havePgp)
  {
    if(havePGP5) {
      kdDebug(5003) << "using PGP5" << endl;
      pgp = new KNPgpBase5();
    }
    else if (haveGpg) {
      kdDebug(5003) << "using GPG" << endl;
      pgp = new KNPgpBaseG();
    }
    else 
    {
      KNPgpBase6 *pgp_v6 = new KNPgpBase6();
      if (!pgp_v6->isVersion6())
      {
        delete pgp_v6;
        pgp = new KNPgpBase2();
        kdDebug(5003) << "using PGP2" << endl;
      }
      else {
        pgp = pgp_v6;
        kdDebug(5003) << "using PGP6" << endl;
      }
    }
  }
  else
  {
    // dummy handler
    pgp = new KNPgpBase();
    return;
  }

  // read kpgp config file entries
  //readConfig();

  // get public keys
  // No! This takes time since pgp takes some ridicules
  // time to start, blocking everything (pressing any key _on_
  // _the_ _machine_ _where_ _pgp_ _runs: helps; ??? )
  // So we will ask for keys when we need them.
  //publicKeys = pgp->pubKeys(); This can return 0!!!

  needPublicKeys = true;
}

//void
//KNPgp::readConfig()
//{
  //KConfig *c = KGlobal::config();
  //c->setGroup("PRIVACY");
  //storePass = c->readBoolEntry("keepPassword");
//}

//void
//KNPgp::writeConfig(bool sync)
//{
  //KConfig *c = KGlobal::config();
  //c->setGroup("PRIVACY");
  //c->writeEntry("keepPassword",storePass);

  //pgp->writeConfig();

  //if(sync)
    //config->sync();
//}

void
KNPgp::setUser(const QString aUser)
{
  pgp->setUser(aUser);
}

const QString
KNPgp::user(void) const
{
  return pgp->user();
}

//void
//KNPgp::setEncryptToSelf(bool flag)
//{
//  pgp->setEncryptToSelf(flag);
//}

//bool
//KNPgp::encryptToSelf(void) const
//{
//  return pgp->encryptToSelf();
//}

bool
KNPgp::storePassPhrase(void) const
{
  return storePass;
}

void
KNPgp::setStorePassPhrase(bool flag)
{
  storePass = flag;
}


bool
KNPgp::setMessage(const QString mess)
{
  int index;
  int retval = 0;

  clear();

  if(havePgp)
    retval = pgp->setMessage(mess);
  if((index = mess.find("-----BEGIN PGP")) != -1)
  {
    if(!havePgp)
    {
      errMsg = i18n("Couldn't find PGP executable.\n"
			 "Please check your PATH is set correctly.");
      return FALSE;
    }
    if(retval != 0)
      errMsg = pgp->lastErrorMessage();

    // front and backmatter...
    front = mess.left(index);
    index = mess.find("-----END PGP",index);
    index = mess.find("\n",index+1);
    back  = mess.right(mess.length() - index);

    return TRUE;
  }
  //  kdDebug() << "KNPgp: message does not contain PGP parts" << endl;
  return FALSE;
}

const QString
KNPgp::frontmatter(void) const
{
  return front;
}

const QString
KNPgp::backmatter(void) const
{
  return back;
}

const QString
KNPgp::message(void) const
{
  return pgp->message();
}

bool
KNPgp::prepare(bool needPassPhrase)
{
  if(!havePgp)
  {
    errMsg = i18n("Could not find PGP executable.\n"
		       "Please check your PATH is set correctly.");
    return FALSE;
  }

  if((pgp->getStatus() & KNPgpBase::NO_SEC_KEY))
    return FALSE;

  if(needPassPhrase)
  {
    if(!havePassPhrase)
      setPassPhrase(askForPass());
    if(!havePassPhrase)
    {
      errMsg = i18n("The pass phrase is missing.");
      return FALSE;
    }
  }
  return TRUE;
}

void
KNPgp::cleanupPass(void)
{
  if(!storePass)
  {
    passphrase.replace(QRegExp(".")," ");
    passphrase = QString::null;
    havePassPhrase = false;
  }
}

//bool
//KNPgp::decrypt(void)
//{
//  int retval;
//
//  // do we need to do anything?
//  if(!pgp->isEncrypted()) return TRUE;
//  // everything ready
//  if(!prepare(TRUE)) return FALSE;
//  // ok now try to decrypt the message.
//  retval = pgp->decrypt(passphrase.latin1());
//  // erase the passphrase if we do not want to keep it
//  cleanupPass();
//
//  if((retval & KNPgpBase::BADPHRASE))
//  {
//    //kdDebug() << "KNPgp: bad passphrase" << endl;
//    havePassPhrase = false;
//  }
//
//  if(retval & KNPgpBase::ERROR)
//  {
//    errMsg = pgp->lastErrorMessage();
//    return false;
//  }
//  return true;
//}

bool
KNPgp::sign(void)
{
  return encryptFor(0, true);
}

bool
KNPgp::encryptFor(const QStrList& aPers, bool sign)
{
  QStrList persons, noKeyFor;
  char* pers;
  int status = 0;
  errMsg = "";
  int ret;
  QString aStr;

  persons.clear();
  noKeyFor.clear();

  if(!aPers.isEmpty())
  {
    QStrListIterator it(aPers);
    while((pers = it.current()))
    {
      QString aStr = getPublicKey(pers);
      if(!aStr.isEmpty())
        persons.append(aStr.latin1());
      else
	noKeyFor.append(pers);
      ++it;
    }
    if(persons.isEmpty())
    {
      int ret = KMessageBox::warningContinueCancel(0,
			       i18n("Could not find the public keys for the\n"
				    "recipients of this mail.\n"
				    "Message will not be encrypted."),
                               i18n("PGP Warning"), i18n("C&ontinue"));
      if(ret == KMessageBox::Cancel) return false;
    }
    else if(!noKeyFor.isEmpty())
    {
      QString aStr = i18n("Could not find the public keys for\n");
      QStrListIterator it(noKeyFor);
      aStr += it.current();
      ++it;
      while((pers = it.current()))
      {
	aStr += ",\n";
	aStr += pers;
	++it;
      }
      if(it.count() > 1)
	aStr += i18n("These persons will not be able to decrypt the message\n");
      else
	aStr += i18n("This person will not be able to decrypt the message\n");

      int ret = KMessageBox::warningContinueCancel(0, aStr, 
                               i18n("PGP Warning"), 
			       i18n("C&ontinue"));
      if(ret == KMessageBox::Cancel) return false;
    }
  }

  status = doEncSign(persons, sign);

  // check for bad passphrase
  while(status & KNPgpBase::BADPHRASE)
  {
    havePassPhrase = false;
    ret = KMessageBox::warningYesNo(0,
				   i18n("You just entered an invalid passphrase.\n"
					"Do you wan't to try again?"),
                                   i18n("PGP Warning"),
				   i18n("&Retry"), i18n("&Abort"));
    if(ret == KMessageBox::No) return false;
    // ok let's try once again...
    status = doEncSign(persons, sign);
  }
  // check for bad keys
  if( status & KNPgpBase::BADKEYS)
  {
    aStr = pgp->lastErrorMessage();
    aStr += "\n";
    aStr += i18n("Do you wan't to encrypt anyway, leave the\n"
		 "message as is, or cancel the message?");
    ret = KMessageBox::warningYesNoCancel(0,aStr, i18n("PGP Warning"),
                           i18n("&Encrypt"), i18n("&Send unecrypted"));
    if(ret == KMessageBox::Cancel) return false;
    if(ret == KMessageBox::No)
    {
      pgp->clearOutput();
      return true;
    }
    if(ret == KMessageBox::Yes) status = doEncSign(persons, sign, true);
  }
  if( status & KNPgpBase::MISSINGKEY)
  {
    aStr = pgp->lastErrorMessage();
    aStr += "\n";
    aStr += i18n("Do you want to leave the message as is,\n"
		 "or cancel the message?");
    ret = KMessageBox::warningContinueCancel(0,aStr, i18n("PGP Warning"),
				   i18n("&Send as is"));
    if(ret == KMessageBox::Cancel) return false;
    pgp->clearOutput();
    return true;
  }
  if( !(status & KNPgpBase::ERROR) ) return true;

  // in case of other errors we end up here.
  errMsg = pgp->lastErrorMessage();
  return false;

}

int
KNPgp::doEncSign(QStrList persons, bool sign, bool ignoreUntrusted)
{
  int retval;

  // to avoid error messages in case pgp is not installed
  if(!havePgp)
    return KNPgpBase::OK;

  if(sign)
  {
    kdDebug(5003) << "KNPgp::doEncSign signs a message" << endl;
    if(!prepare(TRUE)) return KNPgpBase::ERROR;
    retval = pgp->encsign(&persons, passphrase.latin1(), ignoreUntrusted);
  }
  else
  {
    if(!prepare(FALSE)) return KNPgpBase::ERROR;
    retval = pgp->encrypt(&persons, ignoreUntrusted);
  }
  // erase the passphrase if we do not want to keep it
  cleanupPass();

  return retval;
}

bool
KNPgp::signKey(QString _key)
{
  if (!prepare(TRUE)) return FALSE;
  if(pgp->signKey(_key.latin1(), passphrase.latin1()) & KNPgpBase::ERROR)
  {
    errMsg = pgp->lastErrorMessage();
    return false;
  }
  return true;
}


const QStrList*
KNPgp::keys(void)
{
  if (!prepare()) return NULL;

  if(publicKeys.isEmpty()) publicKeys = pgp->pubKeys();
  return &publicKeys;
}

bool
KNPgp::havePublicKey(QString _person)
{
  if(!havePgp) return true;
  if (needPublicKeys)
  {
    publicKeys = pgp->pubKeys();
    needPublicKeys=false;
  }

  // do the checking case insensitive
  QString str;
  _person = _person.lower();
  _person = canonicalAdress(_person);

  for(str=publicKeys.first(); str!=0; str=publicKeys.next())
  {
    str = str.lower();
    if(str.contains(_person)) return TRUE;
  }

  // reread the database, if key wasn't found...
  publicKeys = pgp->pubKeys();
  for(str=publicKeys.first(); str!=0; str=publicKeys.next())
  {
    str = str.lower();
    if(str.contains(_person)) return TRUE;
  }

  return FALSE;
}

QString
KNPgp::getPublicKey(QString _person)
{
  // just to avoid some error messages
  if(!havePgp) return QString::null;
  if (needPublicKeys)
  {
    publicKeys = pgp->pubKeys();
    needPublicKeys=false;
  }

  // do the search case insensitive, but return the correct key.
  QString adress,str,disp_str;
  adress = _person.lower();

  // first try the canonical mail adress.
  adress = canonicalAdress(adress);
  for(str=publicKeys.first(); str!=0; str=publicKeys.next())
    if(str.contains(adress)) return str;

  // now check if the key contains the address
  adress = _person.lower();
  for(str=publicKeys.first(); str!=0; str=publicKeys.next())
    if(str.contains(adress)) return str;

  // reread the database, because key wasn't found...
  publicKeys = pgp->pubKeys();

  // FIXME: let user set the key/ get from keyserver
  // now check if the address contains the key
  adress = _person.lower();
  for(str=publicKeys.first(); str!=0; str=publicKeys.next())
    if(adress.contains(str)) return str;

  // no match until now, let the user choose the key:
  adress = canonicalAdress(adress);
  str= SelectPublicKey(publicKeys, adress.latin1());
  if (!str.isEmpty()) return str;

  return QString::null;
}

QString
KNPgp::getAsciiPublicKey(QString _person)
{
  return pgp->getAsciiPublicKey(_person);
}

//bool
//KNPgp::isEncrypted(void) const
//{
//  return pgp->isEncrypted();
//}

const QStrList*
KNPgp::receivers(void) const
{
  return pgp->receivers();
}

//const QString
//KNPgp::KeyToDecrypt(void) const
//{
//  //FIXME
//  return QString::null;
//}

bool
KNPgp::isSigned(void) const
{
  return pgp->isSigned();
}

QString
KNPgp::signedBy(void) const
{
  return pgp->signedBy();
}

QString
KNPgp::signedByKey(void) const
{
  return pgp->signedByKey();
}

bool
KNPgp::goodSignature(void) const
{
  return pgp->isSigGood();
}

void
KNPgp::setPassPhrase(const QString aPass)
{
  if (!aPass.isEmpty())
  {
    passphrase = aPass;
    havePassPhrase = TRUE;
  }
  else
  {
    if (!passphrase.isEmpty())
      passphrase.replace(QRegExp(".")," ");
    passphrase = QString::null;
    havePassPhrase = FALSE;
  }
}

bool
KNPgp::changePassPhrase(const QString /*oldPass*/,
		       const QString /*newPass*/)
{
  //FIXME...
  KMessageBox::information(0,i18n("Sorry, but this feature\nis still missing"));
  return FALSE;
}

void
KNPgp::clear(bool erasePassPhrase)
{
  if(erasePassPhrase && havePassPhrase && !passphrase.isEmpty())
  {
    passphrase.replace(QRegExp(".")," ");
    passphrase = QString::null;
  }
  front = QString::null;
  back = QString::null;
}

const QString
KNPgp::lastErrorMsg(void) const
{
  return errMsg;
}

bool
KNPgp::havePGP(void) const
{
  return havePgp;
}



// ------------------ static methods ----------------------------
KNPgp *
KNPgp::getKNPgp()
{
  if (!kpgpObject)
  {
    kpgpObject = new KNPgp;
    //kpgpObject->readConfig();
  }
  return kpgpObject;
}

//KConfig *KNPgp::getConfig()
//{
  //return getKNPgp()->config;
//}


const QString
KNPgp::askForPass(QWidget *parent)
{
  return KNPgpPass::getPassphrase(parent);
}

// --------------------- private functions -------------------

// check if pgp 2.6.x or 5.0 is installed
// kpgp will prefer to user pgp 5.0
bool
KNPgp::checkForPGP(void)
{
  // get path
  KConfig *c = KGlobal::config();
  c->setGroup("PRIVACY");
  QString path = c->readEntry("progPath");
  int pgpVersion = c->readNumEntry("pgpVersion");

  // check if path exists
  if ( !access( path.latin1(), X_OK ) ) {
    kdDebug(5003) << "pgp exe is " << path << endl;
    havePgp = true;
    switch (pgpVersion) {
      case 0: // GnuPG
        havePGP5 = false;
        haveGpg = true;
        havePGP6 = false;
        kdDebug(5003) << "KNPgp::checkForPGP() uses GnuPG" << endl;
        break;
      case 1: // PGP 2.6
        havePGP5 = false;
        haveGpg = false;
        havePGP6 = false;
        kdDebug(5003) << "KNPgp::checkForPGP() uses PGP 2.6" << endl;
        break;
      case 2: // PGP6
        havePGP5 = true;
        haveGpg = false;
        havePGP6 = false;
        kdDebug(5003) << "KNPgp::checkForPGP() uses PPG 5" << endl;
        break;
      case 3: // PGP6
        havePGP6 = true;
        havePGP5 = false;
        haveGpg = false;
        kdDebug(5003) << "KNPgp::checkForPGP() uses PPG 6" << endl;
        break;
      default: // oops, this can't happen
        kdDebug(5003) << "KNPgp::checkForPGP() internal error" << endl;
        return false;
    }
    return true;
  }
  havePgp = false;
  havePGP5 = false;
  havePGP6 = false;
  haveGpg = false;
  return false;
}

QString
KNPgp::canonicalAdress(QString _adress)
{
  int index,index2;

  _adress = _adress.simplifyWhiteSpace();
  _adress = _adress.stripWhiteSpace();

  // just leave pure e-mail adress.
  if((index = _adress.find("<")) != -1)
    if((index2 = _adress.find("@",index+1)) != -1)
      if((index2 = _adress.find(">",index2+1)) != -1)
	return _adress.mid(index,index2-index);

  if((index = _adress.find("@")) == -1)
  {
    // local address
    char hostname[1024];
    gethostname(hostname,1024);
    QString adress = "<";
    adress += _adress;
    adress += '@';
    adress += hostname;
    adress += '>';
    return adress;
  }
  else
  {
    int index1 = _adress.findRev(" ",index);
    int index2 = _adress.find(" ",index);
    if(index2 == -1) index2 = _adress.length();
    QString adress = "<";
    adress += _adress.mid(index1+1 ,index2-index1-1);
    adress += ">";
    return adress;
  }
}

QString
KNPgp::SelectPublicKey(QStrList pbkeys, const char *caption)
{
  KNPgpSelDlg dlg(pbkeys,caption);
  QString txt ="";

  if (dlg.exec()==QDialog::Rejected) return 0;
  txt = dlg.key();
  if (!txt.isEmpty())
  {
    return txt;
  }
  return 0;
}


//----------------------------------------------------------------------
//  widgets needed by kpgp
//----------------------------------------------------------------------

KNPgpPass::KNPgpPass(QWidget *parent, const QString &name, bool modal )
  :KDialogBase( parent, name.latin1(), modal, i18n("OpenPGP Security Check"),
                Ok|Cancel )
{
  QHBox *hbox = makeHBoxMainWidget();
  hbox->setSpacing( spacingHint() );
  hbox->setMargin( marginHint() );

  QLabel *label = new QLabel(hbox);
  label->setPixmap( BarIcon("pgp-keys") );

  QWidget *rightArea = new QWidget( hbox );
  QVBoxLayout *vlay = new QVBoxLayout( rightArea, 0, spacingHint() );
  
  label = new QLabel(i18n("Please enter your OpenPGP passphrase"),rightArea);
  lineedit = new QLineEdit( rightArea );
  lineedit->setEchoMode(QLineEdit::Password);
  lineedit->setMinimumWidth( fontMetrics().maxWidth()*20 );
  lineedit->setFocus();
  connect( lineedit, SIGNAL(returnPressed()), this, SLOT(slotOk()) );

  vlay->addWidget( label );
  vlay->addWidget( lineedit );

  disableResize();
}


KNPgpPass::~KNPgpPass()
{
}

QString
KNPgpPass::getPassphrase(QWidget *parent)
{
  KNPgpPass kpgppass(parent, i18n("OpenPGP Security Check"));
  if (kpgppass.exec())
    return kpgppass.getPhrase().copy();
  else
    return QString::null;
}

QString
KNPgpPass::getPhrase()
{
  return lineedit->text();
}



// ------------------------------------------------------------------------

KNPgpKey::KNPgpKey( QStrList *keys, QWidget *parent, const char *name, 
		  bool modal ) 
  :KDialogBase( i18n("Select key"), Yes, Yes, Yes, parent, name, modal, 
		true, i18n("&Insert") )
{
  QHBox *hbox = new QHBox( this );
  setMainWidget( hbox );
  hbox->setSpacing( spacingHint() );
  hbox->setMargin( marginHint() );

  QLabel *label = new QLabel(hbox);
  label->setPixmap( BarIcon("pgp-keys") );

  QWidget *rightArea = new QWidget( hbox );
  QVBoxLayout *vlay = new QVBoxLayout( rightArea, 0, spacingHint() );
  
  label = new QLabel(i18n("Please select the public key to insert"),rightArea);
  combobox = new QComboBox( FALSE, rightArea, "combo" );
  combobox->setFocus();
  if( keys != 0 )
  {
    combobox->insertStrList(keys);
  }
  vlay->addWidget( label );
  vlay->addWidget( combobox );


  setCursor( QCursor(arrowCursor) );
  cursor = kapp->overrideCursor();
  if( cursor != 0 )
    kapp->setOverrideCursor( QCursor(arrowCursor) );

  disableResize();
}


KNPgpKey::~KNPgpKey()
{
  if(cursor != 0)
    kapp->restoreOverrideCursor();
}


QString
KNPgpKey::getKeyName(QWidget *parent, const QStrList *keys)
{
  KNPgpKey pgpkey( (QStrList*)keys, parent );
  pgpkey.exec();
  return pgpkey.getKey().copy();
}


QString
KNPgpKey::getKey()
{
  return combobox->currentText();
}


// ------------------------------------------------------------------------
KNPgpConfig::KNPgpConfig(QWidget *parent, const char *name)
  : QWidget(parent, name), pgp( KNPgp::getKNPgp() )
{
  QVBoxLayout *topLayout = new QVBoxLayout( this, 0, KDialog::spacingHint() );
  
  QGroupBox *group = new QGroupBox( i18n("Identity"), this );
  topLayout->addWidget( group );
  QGridLayout *glay = new QGridLayout( group, 2, 2,  KDialog::spacingHint() );
  glay->addRowSpacing( 0, fontMetrics().lineSpacing() );  

  QLabel *label = new QLabel( i18n("PGP User Identity:"), group );
  pgpUserEdit = new QLineEdit( group );
  pgpUserEdit->setText( pgp->user() );
  glay->addWidget( label, 1, 0 );
  glay->addWidget( pgpUserEdit, 1, 1 );

  group = new QGroupBox( i18n("Options"), this );
  topLayout->addWidget( group );
  QVBoxLayout *vlay = new QVBoxLayout( group, KDialog::spacingHint() );
  vlay->addSpacing( fontMetrics().lineSpacing() );  

  storePass = new QCheckBox( i18n("Keep passphrase in memory"), group );
  encToSelf = new QCheckBox( i18n("Always encrypt to self"), group );
  vlay->addWidget( storePass );
  vlay->addWidget( encToSelf );

  setValues();
}


KNPgpConfig::~KNPgpConfig()
{
}

void
KNPgpConfig::setValues()
{
  // set default values
  pgpUserEdit->setText( pgp->user() );
  storePass->setChecked( pgp->storePassPhrase() );
  //encToSelf->setChecked( pgp->encryptToSelf() );
}

void
KNPgpConfig::applySettings()
{
  pgp->setUser(pgpUserEdit->text());
  pgp->setStorePassPhrase(storePass->isChecked());
  //pgp->setEncryptToSelf(encToSelf->isChecked());

  //pgp->writeConfig(true);
}



// ------------------------------------------------------------------------
KNPgpSelDlg::KNPgpSelDlg( const QStrList &aKeyList, const QString &recipent,
			QWidget *parent, const char *name, bool modal )
  :KDialogBase( parent, name, modal, i18n("PGP Key Selection"), Ok|Cancel, Ok)
{
  QFrame *page = makeMainWidget();
  QVBoxLayout *topLayout = new QVBoxLayout( page, 0, spacingHint() );
  
  QLabel *label = new QLabel( page );
  label->setText(i18n("Select public key for recipient \"%1\"").arg(recipent));
  topLayout->addWidget( label );

  mListBox = new QListBox( page );
  mListBox->setMinimumHeight( fontMetrics().lineSpacing() * 7 );
  mListBox->setMinimumWidth( fontMetrics().maxWidth() * 25 );
  topLayout->addWidget( mListBox, 10 );

  mKeyList = aKeyList;
  mkey = "";

  for( const char *key = mKeyList.first(); key!=0; key = mKeyList.next() )
  {
    //insert only real keys:
    //if (!(QString)key.contains("matching key"))
    mListBox->insertItem(key);
  }
  if( mListBox->count() > 0 )
  {
    mListBox->setCurrentItem(0);
  } 
}


void KNPgpSelDlg::slotOk()
{
  int idx = mListBox->currentItem();
  if (idx>=0) mkey = mListBox->text(idx);
  else mkey = "";

  accept();
}


void KNPgpSelDlg::slotCancel()
{
  mkey = "";
  reject();
}


#include "knpgp.moc"
