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

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <poll.h>
#include <fcntl.h>                                                                                                       

#include <iostream>

#include <qregexp.h>

#include <klocale.h>
#include <kapp.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
 
#include <config.h>

#include "knpgpbase.h"
#include "knpgp.h"

KNPgpBase::KNPgpBase()
{
  //readConfig();
  status = OK;
}

KNPgpBase::~KNPgpBase()
{
  
}

//void 
//KNPgpBase::readConfig()
//{
  //KConfig *config = KNPgp::getConfig();
  //pgpUser = config->readEntry("user");
  //flagEncryptToSelf = config->readBoolEntry("encryptToSelf");
//}

//void
//KNPgpBase::writeConfig(bool sync)
//{
  //KConfig *config = KNPgp::getConfig();
  //config->writeEntry("user",pgpUser);
  //config->writeEntry("encryptToSelf",flagEncryptToSelf);

  //if(sync)
    //config->sync();
//}


bool
KNPgpBase::setMessage(const QString mess)
{
  clear();
  input = mess;
  if(input.find("-----BEGIN PGP") != -1)
    return (decrypt()==OK);
  else
    return false;
}

QString 
KNPgpBase::message() const
{
  // do we have a deciphered text?
  if(!output.isEmpty()) return output;

  // no, then return the original one
  //kdDebug() << "KNPgpBase: No output!" << endl;
  return input;
}

int
KNPgpBase::run(const char *cmd, const char *passphrase)
{
  /* the pipe ppass is used for to pass the password to
   * pgp. passing the password together with the normal input through
   * stdin doesn't seem to work as expected (at least for pgp5.0)
   */
  char str[1024] = "\0";
  int pin[2], pout[2], perr[2], ppass[2];
  int len, status;
  FILE *pass;
  pid_t child_pid, rc;

  if(passphrase)
  {
    kdDebug(5003) << "piping password to pgp" << endl;
    pipe(ppass);

    pass = fdopen(ppass[1], "w");
    fwrite(passphrase, sizeof(char), strlen(passphrase), pass);
    fwrite("\n", sizeof(char), 1, pass);
    fclose(pass);
    close(ppass[1]);

    // tell pgp which fd to use for the passphrase
    QCString tmp;
    tmp.sprintf("%d",ppass[0]);
    setenv("PGPPASSFD",tmp.data(),1);

    //printf("PGPPASSFD = %s\n",tmp.data());
    //printf("pass = %s\n",passphrase);
  }
  else
    unsetenv("PGPPASSFD");

  //printf("cmd = %s\n",cmd);
  //printf("input = %s\nlength = %d\n",input.data(), input.length());

  info = "";
  output = "";

  KConfig *config = KGlobal::config();
  config->setGroup("PRIVACY");
  QString pgpexe = config->readEntry("progPath");

  kdDebug(5003) << "running pgp as: " << pgpexe << " " << cmd << endl;

  pipe(pin);
  pipe(pout);
  pipe(perr);

  // there is a bug in pgp6 which creates a file called "stdin" and ask for 
  // overwriting it in the following calls - not nice in batchmode
  char* tmpname = 0;
  if (!access("stdin", F_OK)) {
    tmpname = tempnam(".","stdin");
    rename("stdin",tmpname);
  }

  QApplication::flushX();
  if(!(child_pid = fork()))
  {
    /*We're the child.*/
    close(pin[1]);
    dup2(pin[0], 0);
    close(pin[0]);

    close(pout[0]);
    dup2(pout[1], 1);
    close(pout[1]);

    close(perr[0]);
    dup2(perr[1], 2);
    close(perr[1]);

    execl("/bin/sh", "sh", "-c", pgpexe.latin1(), cmd,  NULL);
    _exit(127);
  }


  /*Only get here if we're the parent.*/
  close(pin[0]);
  close(pout[1]);
  close(perr[1]);

  if (!input.isEmpty()) {
    kdDebug(5003) << "writing to pgp: " << endl << input << endl;
    write(pin[1], input.latin1(), input.length());
    write(pin[1], "\n", 1);
    // somtimes pgp ask to overwrite a file in batchmode:(
  }
  else {
    kdDebug(5003) << "no input for pgp" << endl;
  }
  close(pin[1]);

#undef POLLING
#ifdef POLLING
  kdDebug(5003) << "polling output from pgp" << endl;
  struct pollfd fds[2];
  fds[0].fd = pout[0];
  fds[0].events =  POLLIN | POLLRDNORM;
  fds[1].fd = perr[0];
  fds[1].events =  POLLIN | POLLRDNORM;
  nfds_t nfds = 2;

  // set the fd's to nonblocking
  if (fds[0].fd >= 0) {
    int val = fcntl(fds[0].fd,F_GETFL,0);
    fcntl(fds[0].fd,F_SETFL, val | O_NONBLOCK);
  }
  if (fds[1].fd >= 0) {
    int val = fcntl(fds[1].fd,F_GETFL,0);
    fcntl(fds[1].fd,F_SETFL, val | O_NONBLOCK);
  }

  int rounds = 0;
  const int rounds_to_timeout = 4;
  while (rounds < rounds_to_timeout) {
    int res = poll(fds,nfds,100);
    kdDebug(5003) << "polling finished with result " << res << endl;
    if (res > 0) {
      kdDebug(5003) << "revents[0]: " << fds[0].revents << " revents[1]: " << fds[1].revents << endl;
      if (fds[0].revents & POLLRDNORM) {
        while ((len=read(pout[0],str,1023))>0)
        {
          str[len] ='\0';
          output += str;
        }
        kdDebug(5003) << "output from pgp: " << endl << output << endl;
      }
      if (fds[1].revents & POLLRDNORM) {
        while ((len=read(perr[0],str,1023))>0)
        {
          str[len] ='\0';
          info += str;
        }
        kdDebug(5003) << "output from pgp: " << endl << info << endl;
      }
      if ( (fds[0].revents & POLLHUP) && (fds[1].revents & POLLHUP) ) break; 
    }
    if (res == 0) ++rounds; // timeout round
  }
#else
  if (pout[0] >= 0) {
    while ((len=read(pout[0],str,1023))>0)
    {
      str[len] ='\0';
      output += str;
    }
    kdDebug(5003) << "output from pgp: " << endl << output << endl;
  }
  if (perr[0] >= 0) {
    while ((len=read(perr[0],str,1023))>0)
    {
      str[len] ='\0';
      info += str;
    }
    kdDebug(5003) << "output from pgp: " << endl << info << endl;
  }
#endif

  unsetenv("PGPPASSFD");
  if(passphrase)
    close(ppass[0]);

  //printf("output = %s\n",output.data());
  //printf("info = %s\n",info.data());

  // we don't want a zombie, do we?  ;-)
  rc = waitpid(0/*child_pid*/, &status, 0);
  if (rc==-1) printf("waitpid: %s\n", strerror(errno));
  if (status) {
    status = ERROR;
    errMsg.sprintf(info.latin1());
  }

  // remove the temporary file from "stdin"
  rename(tmpname,"stdin");

  return status;
}

int
KNPgpBase::runGpg(const char *cmd, const char *passphrase)
{
  /* the pipe ppass is used for to pass the password to
   * pgp. passing the password together with the normal input through
   * stdin doesn't seem to work as expected (at least for pgp5.0)
   */
  char str[1024] = "\0";
  int pin[2], pout[2], perr[2], ppass[2];
  int len, status;
  FILE *pass;
  pid_t child_pid, rc;
  //char gpgcmd[1024] = "\0";

  if(passphrase)
  {
    pipe(ppass);

    pass = fdopen(ppass[1], "w");
    fwrite(passphrase, sizeof(char), strlen(passphrase), pass);
    fwrite("\n", sizeof(char), 1, pass);
    fclose(pass);
    close(ppass[1]);

    //printf("pass = %s\n",passphrase);
  }

  //printf("cmd = %s\n",cmd);
  //printf("input = %s\nlength = %d\n",input.data(), input.length());

  info = "";
  output = "";

  KConfig *config = KGlobal::config();
  config->setGroup("PRIVACY");
  QString pgpexe = config->readEntry("progPath");

  kdDebug(5003) << "running gpg as: " << pgpexe << " " << cmd << endl;

  pipe(pin);
  pipe(pout);
  pipe(perr);


    //if(passphrase) {
      //snprintf(gpgcmd, 1023, "LANG=C; gpg --passphrase-fd %d %s",ppass[0],cmd);
    //} else {
      //snprintf(gpgcmd, 1023, "LANG=C; gpg %s",cmd);
    //}

    QString gpgcmd = pgpexe;
    //#warning FIXME: there has to be a better way to do this
     /* this is nasty nasty nasty (but it works) */
    if(passphrase) {
      //snprintf(gpgcmd, 1023, "LANG=C; gpg --passphrase-fd %d %s",ppass[0],cmd);
      gpgcmd.prepend("LANG=C; ");
      gpgcmd.append(" --passphrase-fd ");
      gpgcmd.append( QString::number(ppass[0]) );
      gpgcmd.append(" ");
      gpgcmd.append(cmd);
    } else {
      //snprintf(gpgcmd, 1023, "LANG=C; gpg %s",cmd);
      gpgcmd.prepend("LANG=C; ");
      gpgcmd.append(" ");
      gpgcmd.append(cmd);
    }
    kdDebug(5003) << "starting gpg: " << gpgcmd << endl;


  QApplication::flushX();
  if(!(child_pid = fork()))
  {
    /*We're the child.*/
    close(pin[1]);
    dup2(pin[0], 0);
    close(pin[0]);
	  
    close(pout[0]);
    dup2(pout[1], 1);
    close(pout[1]);

    close(perr[0]);
    dup2(perr[1], 2);
    close(perr[1]);

    execl("/bin/sh", "sh", "-c", gpgcmd.latin1(), NULL);
    _exit(127);
  }                
     
  /*Only get here if we're the parent.*/
  close(pin[0]);
  close(pout[1]);
  close(perr[1]);

  if (!input.isEmpty()) {
    write(pin[1], input.latin1(), input.length());
    kdDebug(5003) << "write to gpg: " << endl << input << endl;
  }
  else
    write(pin[1], "\n", 1);
  close(pin[1]);
 
  if (pout[0] >= 0)
  {
    while ((len=read(pout[0],str,1023))>0)
    {
      str[len] ='\0';
      output += str;
    }
   close(pout[0]);
   kdDebug(5003) << "output from gpg: " << output << endl;
  }
 
  if (perr[0] >= 0)
  {
    while ((len=read(perr[0],str,1023))>0)
    {
      str[len] ='\0';
      info += str;
    }
    close(perr[0]);
    kdDebug(5003) << "error-output from gpg: " << info << endl;
  }
 
  if(passphrase)
    close(ppass[0]);
     
  //printf("output = %s\n",output.data());
  //printf("info = %s\n",info.data());

  // we don't want a zombie, do we?  ;-)
  rc = waitpid(0/*child_pid*/, &status, 0);
  if (rc==-1) printf("waitpid: %s\n", strerror(errno));
  kdDebug(5003) << "child returned with exit code " << status << endl;
  
  if (status) {
    status = ERROR;
    errMsg.sprintf(info.latin1());
  }
  return status;
}

QString
KNPgpBase::addUserId()
{
  QString cmd;

  if(!pgpUser.isEmpty())
  {
    cmd += " -u \"";
    cmd += pgpUser;
    cmd += "\"";
    return cmd;
  }
  return "";
}

void 
KNPgpBase::clear()
{
  input = QString::null;
  output = QString::null;
  info = QString::null;
  errMsg = QString::null;
  signature = QString::null;
  signatureID = QString::null;
  recipients.clear();
  status = OK;
}

void 
KNPgpBase::clearOutput()
{
  output = QString::null;
}

QString 
KNPgpBase::lastErrorMessage() const
{
  return errMsg;
}

// -------------------------------------------------------------------------

KNPgpBaseG::KNPgpBaseG()
  : KNPgpBase()
{
}

KNPgpBaseG::~KNPgpBaseG()
{
}

int 
KNPgpBaseG::encrypt(const QStrList *_recipients, bool /*ignoreUntrusted*/)
{
  return encsign(_recipients, 0);
}

int 
KNPgpBaseG::sign(const char *passphrase)
{
  return encsign(0, passphrase);
}

int 
KNPgpBaseG::encsign(const QStrList *_recipients, const char *passphrase,
		   bool /*ignoreUntrusted*/)
{
  QString cmd, pers;
  output = "";

  kdDebug(5003) << "KNPgpBaseG::encsign" << endl;
  if(_recipients != 0)
    if(_recipients->count() <= 0)
      _recipients = 0;

  if(_recipients != 0 && passphrase != 0)
    cmd = "--batch --escape-from-lines --armor --always-trust --sign --encrypt ";
  else if( _recipients != 0 )
    cmd = "--batch --escape-from-lines --armor --always-trust --encrypt ";
  else if(passphrase != 0 )
    cmd = "--batch --escape-from-lines --armor --always-trust --clearsign ";
  else 
  {
    kdDebug(5003) << "kpgpbase: Neither recipients nor passphrase specified." << endl;
    return OK;
  }

  if(passphrase != 0)
    cmd += addUserId();

  if(_recipients != 0)
  {
    QStrListIterator it(*_recipients);
    while( (pers=it.current()) != 0 )
    {
      cmd += " --recipient \"";
      cmd += pers;
      cmd += "\" ";
      ++it;
    }
  }
  if(flagEncryptToSelf) {
    cmd += " --recipient \"";
    cmd += user();
    cmd += "\" ";
  }
  cmd += " --set-filename stdin ";

  kdDebug(5003) << "KNPgpBaseG::encsign runs gpg as: " << cmd << endl;

  status = runGpg(cmd.latin1(), passphrase);
  kdDebug(5003) << "runGpg returns to encsign error code " << status << endl;
  if(status != OK) return status;

  if(_recipients != 0)
  {
    int index = 0;
    bool bad = FALSE;
    unsigned int num = 0;
    QString badkeys = "";
    while((index = info.find("Cannot find the public key",index)) 
	  != -1)
    {
      bad = TRUE;
      index = info.find("'",index);
      int index2 = info.find("'",index+1);
      badkeys += info.mid(index, index2-index+1) + ", ";
      num++;
    }
    if(bad)
    {
      badkeys.stripWhiteSpace();
      if(num == _recipients->count())
	errMsg.sprintf("Could not find public keys matching the\n" 
		       "userid(s) %s.\n" 
		       "Message is not encrypted.\n",
		       badkeys.latin1());
      else
	errMsg.sprintf("Could not find public keys matching the\n" 
		       "userid(s) %s. These persons won't be able\n"
		       "to read the message.",
		       badkeys.latin1());
      status |= MISSINGKEY;
      status |= ERROR;
    }
  }
  if(passphrase != 0)
  {
    if(info.find("Pass phrase is good") != -1)
    {
      //kdDebug() << "KNPgpBase: Good Passphrase!" << endl;
      status |= SIGNED;
    }
    if( info.find("bad passphrase") != -1)
    {
      errMsg = i18n("Bad pass Phrase; couldn't sign");
      status |= BADPHRASE;
      status |= ERR_SIGNING;
      status |= ERROR;
    }
  }
  //kdDebug() << "status = " << status << endl;
  return status;
}

int 
KNPgpBaseG::decrypt(const char *passphrase)
{
  QString cmd;
  int index, index2;
  output = "";

  cmd = "--batch --set-filename stdin --decrypt ";

  status = runGpg(cmd.latin1(), passphrase);

  if(status != OK)
  {
    //errMsg = i18n("error running gpg");
    return status;
  }

  // pgp2.6 has sometimes problems with the ascii armor pgp5.0 produces
  // this hack can solve parts of the problem
  if(info.find("ASCII armor corrupted.") != -1)
  {
    kdDebug() << "removing ASCII armor header" << endl;
    int index1 = input.find("-----BEGIN PGP SIGNED MESSAGE-----");
    if(index1 != -1)
      index1 = input.find("-----BEGIN PGP SIGNATURE-----", index1);
    else
      index1 = input.find("-----BEGIN PGP MESSAGE-----");
    index1 = input.find("\n", index1);
    index2 = input.find("\n\n", index1);
    input.remove(index1, index2 - index1);
    status = runGpg(cmd.latin1(), passphrase);
  }
 
  if( info.find("File contains key") != -1)
  {
    // FIXME: should do something with it...
  }

  if ((info.find("secret key not available") != -1)
      || ((info.find("key not found") != -1) && (info.find("Can't check signature") == -1)))
  {
    //kdDebug() << "kpgpbase: message is encrypted" << endl;
    status |= ENCRYPTED;
    if( info.find("bad passphrase") != -1)
    {
      if(passphrase != 0)
      {
	errMsg = i18n("Bad pass Phrase; couldn't decrypt");
	kdDebug() << "KNPgpBase: passphrase is bad" << endl;
	status |= BADPHRASE;
	status |= ERROR;
      }
    } 
    else
    {
      // no secret key fitting this message
      status |= NO_SEC_KEY;
      status |= ERROR;
      errMsg = i18n("Do not have the public key for this message");
      kdDebug() << "KNPgpBase: no public key for this message" << endl;
    }
    // check for persons
    index = info.find("can only be read by:");
    if(index != -1) 
    {
      index = info.find("\n",index);
      int end = info.find("\n\n",index);
      
      recipients.clear();
      while( (index2 = info.find("\n",index+1)) <= end )
      {
	QString item = info.mid(index+1,index2-index-1);
	item.stripWhiteSpace();
	recipients.append(item.latin1());
	index = index2;
      }
    }
  }
  if((index = info.find("Signature made")) != -1)
  {
    //kdDebug() << "KNPgpBase: message is signed" << endl;
    status |= SIGNED;
    if ((info.find("Key matching expected") != -1)
        || (info.find("Can't check signature") != -1))
    {
      index = info.find("key ID ",index);
      signatureID = info.mid(index+7,8);
      signature = i18n("unknown key ID ") + signatureID + " ";
      status |= UNKNOWN_SIG;
      status |= GOODSIG;
    }
    else if( info.find("Good signature") != -1 )
    {
      status |= GOODSIG;
      // get signer
      index = info.find("\"",index);
      index2 = info.find("\"", index+1);
      signature = info.mid(index+1, index2-index-1);
      
      // get key ID of signer
      index = info.find("key ID ",index2);
      signatureID = info.mid(index+7,8);
    }
    else if( info.find("CRC error") != -1 )
    {
      //kdDebug() << "BAD signature" << endl;
      status |= SIGNED;
      status |= ERROR;

      // get signer
      index = info.find("\"",index);
      index2 = info.find("\"", index+1);
      signature = info.mid(index+1, index2-index-1);
      
      // get key ID of signer
      index = info.find("key ID ",index2);
      signatureID = info.mid(index+7,8);
    }
    else if( info.find("Can't find the right public key") != -1 )
    {
      status |= UNKNOWN_SIG;
      status |= GOODSIG; // this is a hack...
      signature = i18n("unknown (file ~/.gnupg/pubring.gpg not found)");
      signatureID = "???";
    }
    else
    {
      status |= ERROR;
      signature = "";
      signatureID = "";
    }
  }
  //kdDebug() << "status = " << status << endl;
  return status;
}

QStrList
KNPgpBaseG::pubKeys()
{
  QString cmd;
  int index, index2;

  cmd = "--batch --list-keys";
  status = runGpg(cmd.latin1());
  if(status != OK) return 0;

  // now we need to parse the output
  QStrList publicKeys;
  index = output.find("\n",1)+1; // skip first to "\n"
  while( (index = output.find("\n",index)) != -1)
  {
    //parse line
    QString line;
    if( (index2 = output.find("\n",index+1)) != -1)
    {
      int index3 = output.find("pub ",index);
      int index4 = output.find("uid ",index);
      if ((index4 != -1) && ((index4 < index3) || (index3 == -1)))
        index3 = index4;
		    
      if( (index3 <index2) && (index3 != -1) )
      {
	line = output.mid(index3+31,index2-index3-31);
	line = line.lower();
      }
      if(!line.isEmpty())
      {
	//kdDebug() << "KNPgpBase: found key for " << (const char *)line << "." << endl;
	publicKeys.append(line.latin1());
      }
    }
    else
      break;
    index = index2;
  }
  //kdDebug() << "finished reading keys" << endl;
  return publicKeys;
}

int
KNPgpBaseG::signKey(const char *key, const char *passphrase)
{
  QString cmd;

  cmd = "--set-filename stdin ";
  cmd += addUserId();
  cmd += "--sign-key ";
  cmd += key;

  return runGpg(cmd.latin1(),passphrase);
}


QString KNPgpBaseG::getAsciiPublicKey(QString _person) {

  QString toexec;
  toexec.sprintf("--batch --armor --export \"%s\"", _person.latin1());

  status = runGpg(toexec.latin1());
  if(status != OK) return 0;

  return output;
}


// -------------------------------------------------------------------------

KNPgpBase2::KNPgpBase2()
  : KNPgpBase()
{
}

KNPgpBase2::~KNPgpBase2()
{
}

int 
KNPgpBase2::encrypt(const QStrList *_recipients, bool /*ignoreUntrusted*/)
{
  return encsign(_recipients, 0);
}

int 
KNPgpBase2::sign(const char *passphrase)
{
  return encsign(0, passphrase);
}

int 
KNPgpBase2::encsign(const QStrList *_recipients, const char *passphrase,
		   bool /*ignoreUntrusted*/)
{
  QString cmd, pers;
  output = "";

  if(_recipients != 0)
    if(_recipients->count() <= 0)
      _recipients = 0;

  if(_recipients != 0 && passphrase != 0)
    cmd = "+batchmode +language=C -seat ";
  else if( _recipients != 0 )
    cmd = "+batchmode +language=C -eat";
  else if(passphrase != 0 )
    cmd = "+batchmode +language=C -sat ";
  else 
  {
    kdDebug(5003) << "kpgpbase: Neither recipients nor passphrase specified." << endl;
    return OK;
  }

  if(passphrase != 0)
    cmd += addUserId();

  if(_recipients != 0)
  {
    QStrListIterator it(*_recipients);
    while( (pers=it.current()) != 0)
    {
      cmd += " \"";
      cmd += pers;
      cmd += "\"";
      ++it;
    }
    if(flagEncryptToSelf)
      cmd += " +EncryptToSelf";
  }
  cmd += " -f";

  status = run(cmd.latin1(), passphrase);
  if(status != OK) return status;

  if(_recipients != 0)
  {
    int index = 0;
    bool bad = FALSE;
    unsigned int num = 0;
    QString badkeys = "";
    if (info.find("Cannot find the public key") != -1)
    {
      index = 0;
      num = 0;
      while((index = info.find("Cannot find the public key",index))
	    != -1)
      {
        bad = TRUE;
        index = info.find("'",index);
        int index2 = info.find("'",index+1);
        if (num++)
          badkeys += ", ";
        badkeys += info.mid(index, index2-index+1);
      }
      if(bad)
      {
        badkeys.stripWhiteSpace();
        if(num == _recipients->count())
	  errMsg.sprintf("Could not find public keys matching the\n"
        	         "userid(s) %s.\n"
		         "Message is not encrypted.\n",
		         badkeys.latin1());
        else
          errMsg.sprintf("Could not find public keys matching the\n"
		         "userid(s) %s. These persons won't be able\n"
		         "to read the message.",
		         badkeys.latin1());
        status |= MISSINGKEY;
        status |= ERROR;
      }
    }
    if (info.find("skipping userid") != -1)
    {
      index = 0;
      num = 0;
      while((index = info.find("skipping userid",index))
	    != -1)
      {
        bad = TRUE;
        int index2 = info.find("\n",index+16);
        if (num++)
          badkeys += ", ";
        badkeys += info.mid(index+16, index2-index-16);
        index = index2;
      }
      if(bad)
      {
        badkeys.stripWhiteSpace();
        if(num == _recipients->count())
	  errMsg.sprintf("Public keys not certified with trusted signature for\n"
	                 "userid(s) %s.\n"
		         "Message is not encrypted.\n",
		         badkeys.latin1());
        else
	  errMsg.sprintf("Public keys not certified with trusted signature for\n"
		         "userid(s) %s. These persons won't be able\n"
		         "to read the message.",
		         badkeys.latin1());
        status |= BADKEYS;
        status |= ERROR;
      }
    }
  }
  if(passphrase != 0)
  {
    if(info.find("Pass phrase is good") != -1)
    {
      //kdDebug() << "KNPgpBase: Good Passphrase!" << endl;
      status |= SIGNED;
    }
    if( info.find("Bad pass phrase") != -1)
    {
      errMsg = i18n("Bad pass Phrase; couldn't sign");
      status |= BADPHRASE;
      status |= ERR_SIGNING;
      status |= ERROR;
    }
  }
  if (info.find("Encryption error") != -1)
  {
    errMsg = i18n("PGP error occured. Please check\nyour PGP setup and key rings.");
    status |= NO_SEC_KEY;
    status |= BADKEYS;
    status |= ERROR;
  }
  //kdDebug() << "status = " << status << endl;
  return status;
}

int 
KNPgpBase2::decrypt(const char *passphrase)
{
  QString cmd;
  int index, index2;
  output = "";


  cmd = "+batchmode +language=C -f";

  status = run(cmd.latin1(), passphrase);

  if(status != OK)
  {
    //errMsg = i18n("error running pgp");
    return status;
  }

  // pgp2.6 has sometimes problems with the ascii armor pgp5.0 produces
  // this hack can solve parts of the problem
  if(info.find("ASCII armor corrupted.") != -1)
  {
    kdDebug() << "removing ASCII armor header" << endl;
    int index1 = input.find("-----BEGIN PGP SIGNED MESSAGE-----");
    if(index1 != -1)
      index1 = input.find("-----BEGIN PGP SIGNATURE-----", index1);
    else
      index1 = input.find("-----BEGIN PGP MESSAGE-----");
    index1 = input.find("\n", index1);
    index2 = input.find("\n\n", index1);
    input.remove(index1, index2 - index1);
    status = run(cmd, passphrase);
  }
 
  if( info.find("File contains key") != -1)
  {
    // FIXME: should do something with it...
  }

  if(info.find("You do not have the secret key") != -1)
  {
    //kdDebug() << "kpgpbase: message is encrypted" << endl;
    status |= ENCRYPTED;
    if( info.find("Bad pass phrase") != -1)
    {
      if(passphrase != 0) 
      {
	errMsg = i18n("Bad pass Phrase; couldn't decrypt");
	kdDebug() << "KNPgpBase: passphrase is bad" << endl;
	status |= BADPHRASE;
	status |= ERROR;
      }
    } 
    else
    {
      // no secret key fitting this message
      status |= NO_SEC_KEY;
      status |= ERROR;
      errMsg = i18n("Do not have the secret key for this message");
      kdDebug() << "KNPgpBase: no secret key for this message" << endl;
    }
    // check for persons
    index = info.find("can only be read by:");
    if(index != -1) 
    {
      index = info.find("\n",index);
      int end = info.find("\n\n",index);
      
      recipients.clear();
      while( (index2 = info.find("\n",index+1)) <= end )
      {
	QString item = info.mid(index+1,index2-index-1);
	item.stripWhiteSpace();
	recipients.append(item.latin1());
	index = index2;
      }
    }
  }
  if((index = info.find("File has signature")) != -1)
  {
    //kdDebug() << "KNPgpBase: message is signed" << endl;
    status |= SIGNED;
    if( info.find("Key matching expected") != -1)
    {
      index = info.find("Key ID ",index);
      signatureID = info.mid(index+7,8);
      signature = i18n("unknown key ID ") + signatureID + " ";
      status |= UNKNOWN_SIG;
      status |= GOODSIG;
    }
    else if( info.find("Good signature") != -1 )
    {
      status |= GOODSIG;
      // get signer
      index = info.find("\"",index);
      index2 = info.find("\"", index+1);
      signature = info.mid(index+1, index2-index-1);
      
      // get key ID of signer
      index = info.find("key ID ",index2);
      signatureID = info.mid(index+7,8);
    }
    else if( info.find("Can't find the right public key") != -1 )
    {
      status |= UNKNOWN_SIG;
      status |= GOODSIG; // this is a hack...
      signature = i18n("unknown (file ~/.pgp/pubring.pgp not found)");
      signatureID = "???";
    }
    else
    {
      status |= ERROR;
      signature = "";
      signatureID = "";
    }
  }
  //kdDebug() << "status = " << status << endl;
  return status;
}

QStrList
KNPgpBase2::pubKeys()
{
  QString cmd;
  int index, index2;

  cmd = "+batchmode +language=C -kv -f";
  status = run(cmd);
  if(status != OK) return 0;

  //truncate trailing "\n"
  if (output.length() > 1) output.truncate(output.length()-1);

  QStrList publicKeys;
  index = output.find("\n",1)+1; // skip first to "\n"
  while( (index = output.find("\n",index)) != -1)
  {
    //parse line
    QString line;
    if( (index2 = output.find("\n",index+1)) != -1)
      // skip last line
    {
      int index3 = output.find("pub ",index);
      
      if( (index3 >index2) || (index3 == -1) )
      {
	// second adress for the same key
	line = output.mid(index+1,index2-index-1);
	line = line.stripWhiteSpace();	       
	line = line.lower();
      } else {
	// line with new key
	int index3 = output.find(
	  QRegExp("/[0-9][0-9]/[0-9][0-9] "),
	  index);
	line = output.mid(index3+7,index2-index3-7);
	line = line.lower();
      }
      //kdDebug() << "KNPgpBase: found key for " << (const char *)line << endl;
      publicKeys.append(line.latin1());
    }
    else
      break;
    index = index2;
  }
  
  return publicKeys;
}

int
KNPgpBase2::signKey(const char *key, const char *passphrase)
{
  QString cmd;

  cmd = "+batchmode +language=C -ks -f";
  cmd += addUserId();
  if(passphrase != 0)
  {
    QString aStr;
    aStr.sprintf(" \"-z%s\"",passphrase);
    cmd += aStr;
  }
  cmd += key;

  return run(cmd);
}


QString KNPgpBase2::getAsciiPublicKey(QString _person) {

  QString toexec;
  toexec.sprintf("+language=C -kxaf \"%s\"", _person.latin1());

  status = run(toexec.latin1());
  if(status != OK) return QString::null;

  return output;
}


// -------------------------------------------------------------------------

KNPgpBase5::KNPgpBase5()
  : KNPgpBase()
{
}
KNPgpBase5::~KNPgpBase5()
{
}

int 
KNPgpBase5::encrypt(const QStrList *_recipients, bool ignoreUntrusted)
{
  return encsign(_recipients, 0, ignoreUntrusted);
}

int 
KNPgpBase5::sign(const char *passphrase)
{
  return encsign( 0, passphrase);
}
  
int 
KNPgpBase5::encsign(const QStrList *_recipients, const char *passphrase,
		   bool ignoreUntrusted)
{
  QString in,cmd,pers;
  int index;
  // used to work around a bug in pgp5. pgp5 treats files
  // with non ascii chars (umlauts, etc...) as binary files, but
  // we wan't a clear signature
  bool signonly = false;
  
  output = "";
  
  if(_recipients != 0)
    if(_recipients->isEmpty())
      _recipients = 0;

  if(_recipients != 0 && passphrase != 0)
    cmd = "-ats -f +batchmode=1";
  else if( _recipients != 0 )
    cmd = "-at -f +batchmode=1 ";
  else if(passphrase != 0 )
  {
    cmd = "-bat -f +batchmode=1 ";
    signonly = true;
  }
  else 
  {
    errMsg = "Neither recipients nor passphrase specified.";
    return OK;
  }

  if(ignoreUntrusted) cmd += " +NoBatchInvalidKeys=off";

  if(passphrase != 0)
    cmd += addUserId();

  if(_recipients != 0)
  {
    QStrListIterator it(*_recipients);
    while( (pers=it.current()) != 0)
    {
      cmd += " -r \"";
      cmd += pers;
      cmd += "\"";
      ++it;
    }
    if(flagEncryptToSelf)
      cmd += " +EncryptToSelf";
  }

  if (signonly)
  {
    input.append("\n");
    input.replace(QRegExp("[ \t]+\n"), "\n");   //strip trailing whitespace
  }                                                                        
  //We have to do this otherwise it's all in vain


  status = run(cmd, passphrase);
  if(status != OK) return status;
  
  // now parse the returned info
  if(info.find("Cannot unlock private key") != -1)
  {
    errMsg = i18n("The passphrase you entered is invalid.");
    status |= ERROR;
    status |= BADPHRASE;
  }
  if(!ignoreUntrusted)
  {
    QString aStr;
    index = -1;
    while((index = info.find("WARNING: The above key",index+1)) != -1)
    {
      int index2 = info.find("But you previously",index);
      int index3 = info.find("WARNING: The above key",index+1);
      if(index2 == -1 || (index2 > index3 && index3 != -1))
      {
	// the key wasn't valid, no encryption to this person
	// extract the person
	index2 = info.find("\n",index);
	index3 = info.find("\n",index2+1);
	aStr += info.mid(index2+1, index3-index2-1);
	aStr += ", ";
      }
    }
    if(!aStr.isEmpty())
    {
      aStr.truncate(aStr.length()-2);
      if(info.find("No valid keys found") != -1)
	errMsg = i18n("The key(s) you wan't to encrypt your message\n"
		      "to are not trusted. No encryption done.");
      else
	errMsg = i18n("The following key(s) are not trusted:\n%1\n"
			    "They will not be able to decrypt the message")
		       .arg(aStr);
      status |= ERROR;
      status |= BADKEYS;
    }
  }
  if((index = info.find("No encryption keys found for")) != -1)
  {
    index = info.find(":",index);
    int index2 = info.find("\n",index);

    errMsg.sprintf("Missing encryption key(s) for: %s", 
		   info.mid(index,index2-index).latin1());
    status |= ERROR;
    status |= MISSINGKEY;
  }
  
  if(signonly)
    output = "-----BEGIN PGP SIGNED MESSAGE-----\n\n" + input + "\n" + output;

  return status;
}

int 
KNPgpBase5::decrypt(const char *passphrase)
{  
  QString in = "";
  output = "";

  status = run("-f +batchmode=1", passphrase);
  if(status != OK) return status;

  // lets parse the returned information.
  int index;

  index = info.find("Cannot decrypt message");
  if(index != -1)
  {
    //kdDebug() << "message is encrypted" << endl;
    status |= ENCRYPTED;

    // ok. we have an encrypted message. Is the passphrase bad,
    // or do we not have the secret key?
    if(info.find("Need a pass phrase") != -1)
    {
      if(passphrase != 0) 
      {
	errMsg = i18n("Bad pass Phrase; couldn't decrypt");
	kdDebug() << "KNPgpBase: passphrase is bad" << endl;
	status |= BADPHRASE;
	status |= ERROR;
      }
    }
    else
    {
      // we don't have the secret key
      status |= NO_SEC_KEY;
      status |= ERROR;
      errMsg = i18n("Do not have the secret key for this message");
      kdDebug() << "KNPgpBase: no secret key for this message" << endl;
    }
    // check for persons
    index = info.find("can only be decrypted by:");
    if(index != -1) 
    {
      index = info.find("\n",index);
      int end = info.find("\n\n",index);
	 
      recipients.clear();
      int index2;
      while( (index2 = info.find("\n",index+1)) <= end )
      {
	QString item = info.mid(index+1,index2-index-1);
	item.stripWhiteSpace();
	recipients.append(item.latin1());
	index = index2;
      }
    }
  } 
  index = info.find("Good signature");
  if(index != -1)
  {
    //kdDebug() << "good signature" << endl;
    status |= SIGNED;
    status |= GOODSIG;
 
    // get key ID of signer
    index = info.find("Key ID ");
    int index2 = info.find(",",index);
    signatureID = info.mid(index+7,index2-index-8);

    // get signer
    index = info.find("\"",index);
    index2 = info.find("\"", index+1);
    signature = info.mid(index+1, index2-index-1);
  }
  index = info.find("BAD signature");
  if(index != -1)
  {
    //kdDebug() << "BAD signature" << endl;
    status |= SIGNED;
    status |= ERROR;

    // get key ID of signer
    index = info.find("Key ID ");
    int index2 = info.find(",",index);
    signatureID = info.mid(index+7,index2-index-8);

    // get signer
    index = info.find("\"",index);
    index2 = info.find("\"", index+1);
    signature = info.mid(index+1, index2-index-1);
  }
  index = info.find("Signature by unknown key");
  if(index != -1)
  {
    index = info.find("keyid: ",index);
    int index2 = info.find("\n",index);
    signatureID = info.mid(index+7,index2-index-7);
    signature = "unknown key ID " + signatureID + " ";
    // FIXME: not a very good solution...
    status |= SIGNED;
    status |= GOODSIG;
  }

  //kdDebug() << "status = " << status << endl;
  return status;
}

QStrList
KNPgpBase5::pubKeys()
{
  int index,index2;
	
  status = run("-l");
  if(status != OK) return 0;

  // now we need to parse the output
  QStrList publicKeys;
  index = output.find("\n",1)+1; // skip first to "\n"
  while( (index = output.find("\n",index)) != -1)
  {
    //parse line
    QString line;
    if( (index2 = output.find("\n",index+1)) != -1)
    {
      int index3 = output.find("uid ",index);
		    
      if( (index3 <index2) && (index3 != -1) )
      {
	line = output.mid(index3+5,index2-index3-5);
	line = line.lower();
      }
      if(!line.isEmpty())
      {
	//kdDebug() << "KNPgpBase: found key for " << (const char *)line << "." << endl;
	publicKeys.append(line.latin1());
      }
    }
    else
      break;
    index = index2;
  }
  //kdDebug() << "finished reading keys" << endl;
  return publicKeys;
}     

QString KNPgpBase5::getAsciiPublicKey(QString _person) {
  QString toexec;
  toexec.sprintf("-xa \"%s\"", _person.latin1());

  status = run(toexec.latin1());
  if(status != OK) return QString::null;

  return output;
}

int
KNPgpBase5::signKey(const char *key, const char *passphrase)
{
  QString cmd;
  
  if(passphrase == 0) return false;
  
  cmd = "-f +batchmode=1";
  cmd += key;
  cmd += addUserId();
  status = run(cmd, passphrase);

  return status;
}

// -------------------------------------------------------------------------

KNPgpBase6::KNPgpBase6()
  : KNPgpBase2()
{
}

KNPgpBase6::~KNPgpBase6()
{
}

int
KNPgpBase6::decrypt(const char *passphrase)
{
  QString cmd;
  int index, index2;
  output = "";

  cmd = "+batchmode +language=C -f";

  kdDebug(5003) << "running pgp6" << endl;
  status = run(cmd, passphrase);

  if(status != OK)
  {
    //errMsg = i18n("error running pgp");
    return status;
  }

  // encrypted message
  if( info.find("File is encrypted.") != -1)
  {
    //kdDebug() << "kpgpbase: message is encrypted" << endl;
    status |= ENCRYPTED;
    if((index = info.find("Key for user ID")) != -1)
    {
      // Find out the key for which the phrase is needed
      index  = info.find(":", index) + 2;
      index2 = info.find("\n", index);
      QString keyId = info.mid(index, index2 - index);
      //kdDebug() << "KNPgpBase: key needed is \"" << keyId << "\"!" << endl;
      //kdDebug() << "KNPgpBase: pgp user is \"" << pgpUser << "\"." << endl;

      // Test output length to find out, if the passphrase is
      // bad. If someone knows a better way, please fix this.
      if (!passphrase || !output.length())
      {
	errMsg = i18n("Bad pass Phrase; couldn't decrypt");
	//kdDebug() << "KNPgpBase: passphrase is bad" << endl;
        status |= BADPHRASE;
        status |= ERROR;
      }
    }
    else if( info.find("Secret key is required to read it.") != -1)
    {
      errMsg = i18n("Do not have the secret key for this message");
      //kdDebug() << "KNPgpBase: no secret key for this message" << endl;
      status |= NO_SEC_KEY;
      status |= ERROR;
    }
  }

  // signed message
  if(((index = info.find("File is signed.")) != -1)
    || (info.find("Good signature") != -1 ))
  {
    //kdDebug() << "KNPgpBase: message is signed" << endl;
    status |= SIGNED;
    if( info.find("signature not checked") != -1)
    {
      index = info.find("KeyID:",index);
      signatureID = info.mid(index+7,8);
      signature = i18n("unknown key ID ");
      signature += " " +signatureID;
      status |= UNKNOWN_SIG;
      status |= GOODSIG;
    }
    else if((index = info.find("Good signature")) != -1 )
    {
      status |= GOODSIG;
      // get signer
      index = info.find("\"",index);
      index2 = info.find("\"", index+1);
      signature = info.mid(index+1, index2-index-1);

      // get key ID of signer
      index = info.find("KeyID:",index2);
      if (index == -1)
        signatureID = "???";
      else
        signatureID = info.mid(index+7,8);
    }
    else if( info.find("Can't find the right public key") != -1 )
    {
      status |= UNKNOWN_SIG;
      status |= GOODSIG; // this is a hack...
      signature = i18n("unknown (file ~/.pgp/pubring.pkr not found)");
      signatureID = "???";
    }
    else
    {
      status |= ERROR;
      signature = "";
      signatureID = "";
    }
  }
  //kdDebug() << "status = " << status << endl;
  return status;
}

QStrList
KNPgpBase6::pubKeys()
{
  QString cmd;
  int index, index2;
  int compatibleMode = 1;

  cmd = "+batchmode +language=C -kv -f";
  status = run(cmd);
  if(status != OK) return 0;

  //truncate trailing "\n"
  if (info.length() > 1) info.truncate(info.length()-1);

  QStrList publicKeys;
  index = info.find("bits/keyID",1); // skip first to "\n"
  if (index ==-1)
  {
    index = info.find("Type bits",1); // skip first to "\n"
    if (index == -1)
      return 0;
    else
      compatibleMode = 0;
  }

  while( (index = info.find("\n",index)) != -1)
  {
    //parse line
    QString line;
    if( (index2 = info.find("\n",index+1)) != -1)
      // skip last line
    {
      int index3;
      if (compatibleMode)
      {
        int index_pub = info.find("pub ",index);
        int index_sec = info.find("sec ",index);
        if (index_pub < 0)
          index3 = index_sec;
        else if (index_sec < 0)
          index3 = index_pub;
        else
          index3 = (index_pub < index_sec ? index_pub : index_sec);
      }
      else
      {
        int index_rsa = info.find("RSA ",index);
        int index_dss = info.find("DSS ",index);
        if (index_rsa < 0)
          index3 = index_dss;
        else if (index_dss < 0)
          index3 = index_rsa;
        else
          index3 = (index_rsa < index_dss ? index_rsa : index_dss);
      }

      if( (index3 >index2) || (index3 == -1) )
      {
	// second adress for the same key
	line = info.mid(index+1,index2-index-1);
	line = line.stripWhiteSpace();	
	line = line.lower();
      } else {
	// line with new key
	int index4 = info.find(
	  QRegExp("/[0-9][0-9]/[0-9][0-9] "),
	  index);
	line = info.mid(index4+7,index2-index4-7);
	line = line.lower();
      }
      //kdDebug() << "KNPgpBase: found key for " << (const char *)line << endl;
      publicKeys.append(line.latin1());
    }
    else
      break;
    index = index2;
  }
  return publicKeys;
}

int
KNPgpBase6::isVersion6()
{
  QString cmd;
  QString empty;

  cmd = "";

  status = run(cmd, empty.latin1());

  if(status != OK)
  {
    //errMsg = i18n("error running pgp");
    return 0;
  }

  if( info.find("Version 6") != -1)
  {
    //kdDebug() << "kpgpbase: pgp version 6.x detected" << endl;
    return 1;
  }

  //kdDebug() << "kpgpbase: not pgp version 6.x" << endl;
  return 0;
}
