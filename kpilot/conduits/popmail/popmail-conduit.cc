// popmail-conduit.cc
//
// Copyright (C) 1998,1999 Dan Pilone
// Copyright (C) 1999 Michael Kropfberger
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
// $Revision$

// This is an old trick so you can determine what revisions
// make up a binary distribution.
//
//
static char *id="$Id$";


#include <sys/types.h>
#include <sys/socket.h> 

#include <qdir.h>
#include <iostream.h>
#include <kapp.h>
#include <kconfig.h>
#include <kmsgbox.h>
#include <ksock.h>
#include "conduitApp.h"
#include "pi-source.h"
#include "pi-mail.h"
#include "passworddialog.h"
#include "popmail-conduit.h"
#include "setupDialog.h"
#include "kpilot.h"

extern "C" {
extern time_t parsedate(char * p);
}





int main(int argc, char* argv[])
{
  ConduitApp a(argc, argv, "popmail-conduit");
  PopMailConduit conduit(a.getMode());
  a.setConduit(&conduit);
  return a.exec();
}


void reset_Mail(struct Mail *t)
{
      t->to = 0;
      t->from = 0;
      t->cc = 0;
      t->bcc = 0;
      t->subject = 0;
      t->replyTo = 0;
      t->sentTo = 0;
      t->body = 0;
      t->dated = 0;
}

PopMailConduit::PopMailConduit(eConduitMode mode)
  : BaseConduit(mode)
{
}

PopMailConduit::~PopMailConduit()
{
}

/* static */ const char *PopMailConduit::version()
{
	return "popmail-conduit 1.2";
}

void
PopMailConduit::doSync()
{
	FUNCTIONSETUP;

	KConfig* config = kapp->getConfig();
	int mode=0;

	config->setGroup(PopMailOptions::configGroup());

	mode=config->readNumEntry("SendOutgoing");
	if(mode)
	{
		if (debug_level)
		{
			cerr << fname << ": Sending pending mail" << endl;
		}
		sendPendingMail(mode);
	}

	mode=config->readNumEntry("SyncIncoming");
	if(mode)
	{
		if (debug_level)
		{
			cerr << fname << ": Querying pop server." << endl;
		}
		retrieveIncoming(mode);
	}
}

QWidget*
PopMailConduit::aboutAndSetup()
{
  return new PopMailOptions;
}

// additional changes by Michael Kropfberger
void PopMailConduit::sendPendingMail(int mode /* unused */)
{
	FUNCTIONSETUP;

	KConfig* config = kapp->getConfig();
	config->setGroup(PopMailOptions::configGroup());

	if (config->readBoolEntry("UseSMTP"))
	{
		sendViaSMTP();
	}
	else
	{
		sendViaSendmail();
	}
}

void PopMailConduit::retrieveIncoming(int mode)
{
	FUNCTIONSETUP;

	if (mode==POP)
	{
		doPopQuery();
	}
	if (mode==UNIXMailbox)
	{
		doUnixStyle();
	}
}



// additional changes by Michael Kropfberger
void PopMailConduit::sendViaSMTP() 
{
	FUNCTIONSETUP;

  int i = 0;
  struct Mail theMail;
  QString smtpSrv;
  int smtpPort;
  QString currentDest, msg;
  PilotRecord* pilotRec;
  KSocket* smtpSocket;
  char buffer[0xffff];
  int ret;
  
  KConfig* config = kapp->getConfig();
  config->setGroup(PopMailOptions::configGroup());
  smtpSrv = config->readEntry("SMTPServer","localhost");
  smtpPort = config->readNumEntry("SMTPPort",25);
  
//   pilotLink->addSyncLogEntry("Reading outgoing mail...");
  smtpSocket = new KSocket(smtpSrv.data(),smtpPort); CHECK_PTR(smtpSocket);
  if(smtpSocket->socket() < 0)
    {
      showMessage("Cannot connect to SMTP server.");
      cout << "Cannot connect to server." << endl;
      delete smtpSocket;
      return;
    }
  smtpSocket->enableRead(true);
  smtpSocket->enableWrite(true);

  // all do-while loops wait until data is avail
  do
    ret=read(smtpSocket->socket(), buffer, 1024);
  while ((ret==-1) && (errno==EAGAIN));
  msg=buffer;
  if (msg.find("220") == -1) 
    {
      msg.prepend("SMTP server failed to announce itself");
      showMessage(msg.data());
      delete smtpSocket;
      return;
    }  
  getdomainname(buffer,strlen(buffer));
  sprintf(buffer, "EHLO %s\r\n",buffer);
  write(smtpSocket->socket(), buffer, strlen(buffer));
    do
      ret=read(smtpSocket->socket(), buffer, 1024);
    while ((ret==-1) && (errno==EAGAIN));
  msg=buffer;
  if (msg.find("Hello") == -1) 
    {
      msg.prepend("Couldn't EHLO to SMTP Server.\n");
      showMessage(msg.data());
      delete smtpSocket;
      return;
    }  
  // Should probably read the prefs.. 
  // But, let's just get the mail..
  for(i = 0;; i++)
    {
      pilotRec = readNextRecordInCategory(1);
      if(pilotRec == 0L)
	break;
      if((pilotRec->getAttrib() & dlpRecAttrDeleted) 
               || (pilotRec->getAttrib() & dlpRecAttrArchived))
	delete pilotRec;
      else
	{
	  unpack_Mail(&theMail, (unsigned char*)pilotRec->getData()
                      , pilotRec->getLen());
	  currentDest = "Mailing: ";
	  currentDest += theMail.to;


          QString fromAddress = config->readEntry("EmailAddress");
        sprintf(buffer,"MAIL FROM: %s\r\n",fromAddress.data());
          write(smtpSocket->socket(), buffer, strlen(buffer));
          do
            ret=read(smtpSocket->socket(), buffer, 1024);
          while ((ret==-1) && (errno==EAGAIN));
          msg=buffer;
          if (msg.find("250") == -1){
            showMessage("Couldn't start sending new mail.");
            delete smtpSocket;
            return;
          }  
             
          sprintf(buffer,"RCPT TO: %s\r\n",theMail.to);
             write(smtpSocket->socket(), buffer, strlen(buffer));
          do
            ret=read(smtpSocket->socket(), buffer, 1024);
          while ((ret==-1) && (errno==EAGAIN));
          msg=buffer;
          if (msg.find("25") == -1){  // positively could be 250 or 251
            showMessage("The recipient doesn't exist!");
            delete smtpSocket;
            return;
          }  
          sprintf(buffer,"DATA\r\n");
             write(smtpSocket->socket(), buffer, strlen(buffer));
          do
            ret=read(smtpSocket->socket(), buffer, 1024);
          while ((ret==-1) && (errno==EAGAIN));
          msg=buffer;
          if (msg.find("354") == -1){
            msg.prepend("Couldn't start writing mailbody\n");
            showMessage(msg.data());
            delete smtpSocket;
            return;
          }  
          sprintf(buffer,"From: %s\r\n",fromAddress.data());
             write(smtpSocket->socket(), buffer, strlen(buffer));
          sprintf(buffer,"To: %s\r\n",theMail.to);
             write(smtpSocket->socket(), buffer, strlen(buffer));
          if (theMail.cc) {
             sprintf(buffer,"Cc: %s\r\n",theMail.cc);
               write(smtpSocket->socket(), buffer, strlen(buffer));
          }
	  if (theMail.bcc) {
               sprintf(buffer,"Bcc: %s\r\n",theMail.bcc);
               write(smtpSocket->socket(), buffer, strlen(buffer));
	  }
 	  if (theMail.replyTo) {
               sprintf(buffer,"Reply-To: %s\r\n",theMail.replyTo);
               write(smtpSocket->socket(), buffer, strlen(buffer));
	  }
	  if (theMail.subject) {
               sprintf(buffer,"Subject: %s\r\n",theMail.subject);
               write(smtpSocket->socket(), buffer, strlen(buffer));
	  }
          sprintf(buffer,"X-mailer: %s\r\n\r\n",version());
             write(smtpSocket->socket(), buffer, strlen(buffer));
	     
          if(theMail.body) {
               sprintf(buffer,"%s\r\n",theMail.body);
               write(smtpSocket->socket(), buffer, strlen(buffer));
	  }
          //insert the real signature file from disk
          if(config->readEntry("Signature")) {
             QFile f(config->readEntry("Signature"));
             if ( f.open(IO_ReadOnly) ) {    // file opened successfully
                sprintf(buffer,"\r\n-- \r\n");
                write(smtpSocket->socket(), buffer, strlen(buffer));
                QTextStream t( &f );        // use a text stream
                while ( !t.eof() ) {        // until end of file...
                  sprintf(buffer,"%s\r\n",t.readLine().data());
                  write(smtpSocket->socket(), buffer, strlen(buffer));
                }
             f.close();
             }
          }
	    
          sprintf(buffer,".\r\n");  //end of mail
          write(smtpSocket->socket(), buffer, strlen(buffer));
          do
            ret=read(smtpSocket->socket(), buffer, 1024);
          while ((ret==-1) && (errno==EAGAIN));
          msg=buffer;
          if (msg.find("250") == -1){
            showMessage("Couldn't send message.");
            delete smtpSocket;
            return;
          }  

	  // Mark it as filed...
	  pilotRec->setCat(3);
	  pilotRec->setAttrib(pilotRec->getAttrib() & ~dlpRecAttrDirty);
	  writeRecord(pilotRec);
	  delete pilotRec;
	  // This is ok since we got the mail with unpack mail..
	  free_Mail(&theMail);
	}
    }
  sprintf(buffer, "QUIT\r\n");
  write(smtpSocket->socket(), buffer, strlen(buffer));
  do
    ret=read(smtpSocket->socket(), buffer, 1024);
  while ((ret==-1) && (errno==EAGAIN));
  msg=buffer;
  if (msg.find("221") == -1) 
    {
      msg.prepend("QUIT command to SMTP server failed.\n");
      showMessage(msg.data());
    }
  delete smtpSocket;
//   pilotLink->addSyncLogEntry("OK\n");}
}

void PopMailConduit::sendViaSendmail() 
{
	FUNCTIONSETUP;

  int i = 0;
  struct Mail theMail;
  QString sendmailCmd;
  QString currentDest;
  PilotRecord* pilotRec;
  
  KConfig* config = kapp->getConfig();
  config->setGroup(PopMailOptions::configGroup());
  sendmailCmd = config->readEntry("SendmailCmd");
  
//   pilotLink->addSyncLogEntry("Reading outgoing mail...");

  // Should probably read the prefs.. 
  // But, let's just get the mail..
  for(i = 0;; i++)
    {
      FILE* sendf; // for talking to sendmail
      pilotRec = readNextRecordInCategory(1);
	if(pilotRec == 0L)
	{
		if (debug_level)
		{
			cerr << fname << ": Got a NULL record from "
				"readNextRecord" << endl;
		}
		break;
	}
      if((pilotRec->getAttrib() & dlpRecAttrDeleted) 
               || (pilotRec->getAttrib() & dlpRecAttrArchived))
	{
		if (debug_level > TEDIOUS)
		{
			cerr << fname << ": Skipping deleted record." << endl;
		}
		delete pilotRec;
	}
      else
	{
	  unpack_Mail(&theMail, (unsigned char*)pilotRec->getData()
                      , pilotRec->getLen());
	  sendf = popen(sendmailCmd, "w");
	  if(!sendf)
	    {
 	      KMsgBox::message(0L, "Error Sending Mail"
                      , "Cannot talk to sendmail!", KMsgBox::STOP);
	      return;
	    }
	  currentDest = "Mailing: ";
	  currentDest += theMail.to;
	  sendMessage(sendf, theMail);
	  pclose(sendf);
	  // Mark it as filed...
	  pilotRec->setCat(3);
	  pilotRec->setAttrib(pilotRec->getAttrib() & ~dlpRecAttrDirty);
	  writeRecord(pilotRec);
	  delete pilotRec;
	  // This is ok since we got the mail with unpack mail..
	  free_Mail(&theMail);
	}
    }
//   free_MailAppInfo(&mailAppInfo);
}

// From pilot-link-0.8.7 by Kenneth Albanowski
// additional changes by Michael Kropfberger

void
PopMailConduit::sendMessage(FILE* sendf, struct Mail& theMail)
{
	FUNCTIONSETUP;

  KConfig* config = kapp->getConfig();
  QTextStream mailPipe(sendf, IO_WriteOnly);
  
  config->setGroup(PopMailOptions::configGroup());
  QString fromAddress = config->readEntry("EmailAddress");
  mailPipe << "From: " << fromAddress << "\r\n";
  mailPipe << "To: " << theMail.to << "\r\n";
  if(theMail.cc)
    mailPipe << "Cc: " << theMail.cc << "\r\n";
  if(theMail.bcc)
    mailPipe << "Bcc: " << theMail.bcc << "\r\n";
  if(theMail.replyTo)
    mailPipe << "Reply-To: " << theMail.replyTo << "\r\n";
  if(theMail.subject)
    mailPipe << "Subject: " << theMail.subject << "\r\n";
  mailPipe << "X-mailer: " << version() << "\r\n";
  mailPipe << "\r\n";
  if(theMail.body)
    mailPipe << theMail.body << "\r\n";

  //insert the real signature file from disk
  if(config->readEntry("Signature")) {
      QFile f(config->readEntry("Signature"));
      if ( f.open(IO_ReadOnly) ) {    // file opened successfully
         mailPipe << "-- \r\n";
         QTextStream t( &f );        // use a text stream
         while ( !t.eof() ) {        // until end of file...
           mailPipe << t.readLine() << "\r\n";
         }
         f.close();
      }
   }
    mailPipe << "\r\n";
}

/* static */ char*
PopMailConduit::skipspace(char * c) 
    {
    while (c && ((*c == ' ') || (*c == '\t')))
	c++;
    return c;
    }

int 
PopMailConduit::getpopchar(int socket)
    {
    unsigned char buf;
    int ret;
    do 
	{
      do
        ret=read(socket, &buf, 1);
      while ((ret==-1) && (errno==EAGAIN));
	if (ret < 0)
	    return ret;
	} while ((ret==0) || (buf == '\r'));
    
    return buf;
    }

int 
PopMailConduit::getpopstring(int socket, char * buf)
    {
    int c;
    while ((c = getpopchar(socket)) >= 0) 
	{
	*buf++ = c;
	if (c == '\n')
	    break;
	}
    *buf = '\0';
    return c;
    }

int 
PopMailConduit::getpopresult(int socket, char * buf)
    {
    int c = getpopstring(socket, buf);
    
    if (c<0)
	return c;
    
    if (buf[0] == '+')
	return 0;
    else
	return 1;
    }

/* static */ void 
PopMailConduit::header(struct Mail * m, char * t)
{
	FUNCTIONSETUP;

    static char holding[4096];
    
    if (t && strlen(t) && t[strlen(t)-1] == '\n')
	t[strlen(t)-1] = 0;
    if (t && ((t[0] == ' ') || (t[0] == '\t'))) 
	{
	if ((strlen(t) + strlen(holding)) > 4096)
	    return; /* Just discard approximate overflow */
	strcat(holding, t+1);
	return;
	}
    
    /* Decide on what we do with m->sendTo */
    
    if (strncmp(holding, "From:", 5)==0) 
	{
	m->from = strdup(skipspace(holding+5));
	} 
    else if (strncmp(holding, "To:",3)==0) 
	{
	m->to = strdup(skipspace(holding+3));
	} 
    else if (strncmp(holding, "Subject:",8)==0) 
	{
	m->subject = strdup(skipspace(holding+8));
	} 
    else if (strncmp(holding, "Cc:",3)==0) 
	{
	m->cc = strdup(skipspace(holding+3));
	} 
    else if (strncmp(holding, "Bcc:",4)==0) 
	{
	m->bcc = strdup(skipspace(holding+4));
	} 
    else if (strncmp(holding, "Reply-To:",9)==0) 
	{
	m->replyTo = strdup(skipspace(holding+9));
	} 
    else if (strncmp(holding, "Date:",4)==0) 
	{
	time_t d = parsedate(skipspace(holding+5));
	if (d != -1) 
	    {
	    struct tm * d2;
	    m->dated = 1;
	    d2 = localtime(&d);
	    m->date = *d2;
	    }
	}
    holding[0] = 0;
    if (t)
	strcpy(holding, t);
    }

void PopMailConduit::doPopQuery()
{
  KSocket* popSocket;
  KConfig* config = kapp->getConfig();
  char buffer[0xffff];
  int i, ret;
  
//   pilotLink->addSyncLogEntry("Retrieving incoming mail...");
  config->setGroup(PopMailOptions::configGroup());
  popSocket = new KSocket(config->readEntry("PopServer").data()
            , config->readNumEntry("PopPort")); CHECK_PTR(popSocket);
  if(popSocket->socket() < 0)
    {
      showMessage("Cannot connect to POP server.");
      cout << "Cannot connect to server." << endl;
      delete popSocket;
      return;
    }
  popSocket->enableRead(true);
  popSocket->enableWrite(true);
  // The following code is based _HEAVILY_ :) on pilot-mail.c by Kenneth Albanowski
  // additional changes by Michael Kropfberger
  // all do-while loops wait until data is avail
  do
    ret=read(popSocket->socket(), buffer, 1024);
  while ((ret==-1) && (errno==EAGAIN));
  if(buffer[0] != '+')
    {
      QString msg;
      msg.sprintf("POP server failed to announce itself");
      showMessage(msg.data());
      delete popSocket;
      return;
    }
  sprintf(buffer, "USER %s\r\n", config->readEntry("PopUser").data());
  write(popSocket->socket(), buffer, strlen(buffer));
    do
      ret=read(popSocket->socket(), buffer, 1024);
    while ((ret==-1) && (errno==EAGAIN));
  if (buffer[0] != '+') 
    {
      showMessage("USER command to POP server failed.");
      delete popSocket;
      return;
    }
  if(config->readNumEntry("StorePass", 0))
    sprintf(buffer, "PASS %s\r\n", config->readEntry("PopPass").data());
  else
    {
      PasswordDialog* passDialog = new PasswordDialog("Please Enter your POP password:", 0L, "PopPassword", true);
      passDialog->show();
      sprintf(buffer, "PASS %s\r\n", passDialog->password());
      delete passDialog;
    }
  write(popSocket->socket(), buffer, strlen(buffer));
    do
      ret=read(popSocket->socket(), buffer, 1024);
    while ((ret==-1) && (errno==EAGAIN));
  if (buffer[0] != '+') 
    {
      showMessage("PASS command to POP server failed.");
//       pilotLink->addSyncLogEntry("\n   Invalid POP Password.\n");
      delete popSocket;
      return;
    }
  sprintf(buffer, "STAT\r\n");
  write(popSocket->socket(), buffer, strlen(buffer));
  do
    ret=read(popSocket->socket(), buffer, 1024);
  while ((ret==-1) && (errno==EAGAIN));
  if (ret<0) 
    {
      sprintf(buffer, "Socket error getting mail: %i",errno);
      showMessage(buffer);
      delete popSocket;
      return;
    }
  buffer[ret] = 0;
  if (buffer[0] == '+')
    {
      //sometimes looks like: "+OK ? messages (??? octets)
      //                  or: "+OK <user> has ? message (??? octets)
      QString msg(buffer);
      if (msg.find( config->readEntry("PopUser")) != -1) // with username
          sscanf(buffer, "%*s %*s %*s %d %*s", &i);
      else // normal version
          sscanf(buffer, "%*s %d %*s", &i);
      //msg.sprintf("[%s] so read %i mails...",buffer,i);
      //showMessage(msg.data());
      if(i < 1)
	{
	  // No messages, so bail early..
	  sprintf(buffer, "QUIT\r\n");
	  write(popSocket->socket(), buffer, strlen(buffer));
          do
            ret=read(popSocket->socket(), buffer, 1024);
          while ((ret==-1) && (errno==EAGAIN));
	  if (buffer[0] != '+') { showMessage("QUIT command to POP server failed."); }
	  delete popSocket;
// 	  pilotLink->addSyncLogEntry("OK\n");
	  return;
	}
//       else
// 	pilotLink->createNewProgressBar("Syncing Email", "Retrieving incoming...", 1, i+1, 0);
    }
  for(i=1;;i++) 
    {
      int len;
      char * msg;
      int h;
      struct Mail t;
      PilotRecord* pilotRec;
      
      reset_Mail(&t);
      
      //       pilotLink->updateProgressBar(i);

      sprintf(buffer, "LIST %d\r\n", i);
      write(popSocket->socket(), buffer, strlen(buffer));
      do
        ret=read(popSocket->socket(), buffer, 1024);
      while ((ret==-1) && (errno==EAGAIN));
      if (ret<0) 
	{
          sprintf(buffer, "Socket error getting mail: %i",errno);
	  showMessage(buffer);
	  // 	  pilotLink->destroyProgressBar();
	  delete popSocket;
	  return;
	}
      buffer[ret] = 0;
      if (buffer[0] != '+')
	break;
      
      sscanf(buffer, "%*s %*d %d", &len);
      
      if (len > 16000) 
	continue; 
      
      sprintf(buffer, "RETR %d\r\n", i);
      write(popSocket->socket(), buffer, strlen(buffer));
      ret = getpopstring(popSocket->socket(), buffer);
      if ((ret < 0) || (buffer[0] != '+')) 
	{
	  /* Weird */
	  continue;
	} 
      else
	buffer[ret] = 0;
      
      msg = (char*)buffer;
      h = 1;
      for(;;) 
	{
	  if (getpopstring(popSocket->socket(), msg) < 0) 
	    {
	      showMessage("Error reading message");
	      // 	      pilotLink->destroyProgressBar();
	      delete popSocket;
	      return;
	    }
	  
	  if (h == 1) 
	    { 
	      /* Header mode */
	      if ((msg[0] == '.') && (msg[1] == '\n') && (msg[2] == 0)) 
		{
		  break; /* End of message */
		}
	      if (msg[0] == '\n') 
		{
		  h = 0;
		  header(&t, 0);
		} 
	      else 
		header(&t, msg);
	      continue;
	    }
	  if ((msg[0] == '.') && (msg[1] == '\n') && (msg[2] == 0)) 
	    {
	      msg[0] = 0;
	      break; /* End of message */
	    }
	  if (msg[0] == '.') 
	    {
	      /* Must be escape */
	      memmove(msg, msg+1, strlen(msg));
	    }
	  msg += strlen(msg);
	}
      
      /* Well, we've now got the message. I bet _you_ feel happy with yourself. */
      
      if (h) 
	{
	  /* Oops, incomplete message, still reading headers */
	  // 	  showMessage("Incomplete message");
	  // This is ok since we used strdup's for them all.
	  free_Mail(&t);
	  continue;
	}
      
      // Need to add this support...
      // 	if (strlen(msg) > p.truncate) 
      // 	    {
      // 	    /* We could truncate it, but we won't for now */
      // 	    fprintf(stderr, "Message %d too large (%ld bytes)\n", i, (long)strlen(msg));
      // 	    free_Mail(&t);
      // 	    continue;
      // 	    }
      
      t.body = strdup(buffer);
      
      len = pack_Mail(&t, (unsigned char*)buffer, 0xffff);
      pilotRec = new PilotRecord(buffer, len, 0, 0, 0);
      if (writeRecord(pilotRec) > 0) 
	{
	  if (config->readNumEntry("LeaveMail") == 0)
	    { 
	      sprintf(buffer, "DELE %d\r\n", i);
	      write(popSocket->socket(), buffer, strlen(buffer));
              do
                ret=read(popSocket->socket(), buffer, 1024);
              while ((ret==-1) && (errno==EAGAIN));
	      if (buffer[0] != '+') 
		{
		  showMessage("Error deleting message.");
		}
	    } 
	} 
      else 
	{
	  showMessage("Error writing message to the Pilot.");
	}
      
      delete pilotRec;
      // This is ok since we used strdup's for them all..
      free_Mail(&t);
    }
  
  sprintf(buffer, "QUIT\r\n");
  write(popSocket->socket(), buffer, strlen(buffer));
  do
    ret=read(popSocket->socket(), buffer, 1024);
  while ((ret==-1) && (errno==EAGAIN));
  if (buffer[0] != '+') 
    {
      showMessage("QUIT command to POP server failed.");
    }
  delete popSocket;
//   pilotLink->addSyncLogEntry("OK\n");
}

/* static */ int PopMailConduit::skipBlanks(FILE *f,char *buffer,int buffersize)
{
	FUNCTIONSETUP;

	char *s;
	int count=0;

	while (!feof(f))
	{
		if (fgets(buffer,buffersize,f)==0L) break;
		if (debug_level & SYNC_TEDIOUS)
		{
			cerr <<  fname << ": Got line " << buffer ;
		}

		s=buffer;
		while (isspace(*s)) s++;
		if (*s) return count;
		//
		// Count lines skipped
		//
		count++;
	}

	//
	// EOF found, so erase buffer beginning.
	//	
	*buffer=0;
	return count;
}
#define LINESIZE	(800)
/* static */ int PopMailConduit::readHeaders(FILE *f,
	char *buf,int bufsiz,
	struct Mail *t,
	int expectFrom)
{
	FUNCTIONSETUP;

	char line[LINESIZE];
	int count=0;

	// First line of a message should be a "^From "
	// line, but we'll accept some blank lines first
	// as well.
	//
	//
	if (expectFrom)
	{
		if (debug_level & SYNC_TEDIOUS)
		{
			cerr << fname << ": Looking for From line." << endl;
		}

		skipBlanks(f,line,LINESIZE);
		if (strncmp(line,"From ",5))
		{
			cerr << fname << ": No leading From line." << endl;
			return 0;
		}

		if (debug_level & SYNC_TEDIOUS)
		{
			cerr << fname << ": Found it." << endl;
		}
	}

	while ((skipBlanks(f,line,LINESIZE)==0) && !feof(f))
	{
		if ((line[0]=='.') && (line[1]=='\n') && (line[2] == 0))
		{
			if (debug_level & SYNC_TEDIOUS)
			{
				cerr << fname << ": Found end-of-headers " 
					"and end-of-message."
					<< endl;
			}
			// End of message *and* end-of headers.
			return -count;
		}

		// This if-clause is actually subsumed by
		// skipBlanks, which returns > 0 if lines are
		// skipped because they are blank.
		//
		//
		if (line[0]=='\n')
		{
			if (debug_level & SYNC_TEDIOUS)
			{
				cerr << fname << ": Found end-of-headers" 
					<< endl;
			}
			// End of headers
			header(t,0);
			return count;
		}

		header(t,line);
		count++;
	}

	if (debug_level & SYNC_TEDIOUS)
	{
		cerr << fname << ": Read " << count << " lines." << endl;
	}
	strcpy(buf,line);
	return count;
}


/* static */ int PopMailConduit::readBody(FILE *f,char *buf,int bufsize)
{
	FUNCTIONSETUP;
	int count=0;
	int linelen=0;

	if (debug_level & SYNC_TEDIOUS)
	{
		cerr << fname << ": Buffer @" << (int) buf << endl;
	}

	while(!feof(f) && (bufsize > 80))
	{
		if (fgets(buf,bufsize,f)==0)
		{
			// End of file, implies end
			// of message.
			//
			//
			return count;
		}

		if (debug_level & SYNC_TEDIOUS)
		{
			cerr << fname << ": Got line ["  
				<< (int) buf[0] << ',' << (int) buf[1] 
				<< ']'
				<< buf;
		}

		if ((buf[0]=='.') && ((buf[1]=='\n') || (buf[1]=='\r')))
		{
			// Explicit end of message
			//
			//
			return count;
		}

		count++;
		if (buf[0]=='.')
		{
			// Handle . escapes
			//
			//
			memmove(buf+1,buf,strlen(buf));
		}


		linelen=strlen(buf);
		buf+=linelen;
		bufsize-=linelen;
	}

	return count;
}

#undef LINESIZE

/* static */ PilotRecord *PopMailConduit::readMessage(FILE *mailbox,
	char *buffer,int bufferSize)
{
	FUNCTIONSETUP;

	struct Mail t;		// Just like in doPopQuery
	int messageLength=0;
	int len;
	PilotRecord* pilotRec=0L;

	reset_Mail(&t);

	// Don't forget: readHeaders returns the number of lines.
	//
	messageLength=readHeaders(mailbox,buffer,bufferSize,&t,1);
	if (messageLength == 0)
	{
		cerr << fname << ": Bad headers in message." << endl;
		return 0;
	}


	if (messageLength>0)
	{
		messageLength=strlen(buffer);
		if (debug_level & SYNC_TEDIOUS)
		{
			cerr << fname << ": Message so far:" << endl
				<< buffer << endl;
			cerr << fname << ": Length " 
				<< messageLength << endl;
			cerr << fname << ": Buffer @" << (int) buffer 
				<< endl;
		}

		if (readBody(mailbox,
			buffer+messageLength,
			bufferSize-messageLength) < 0)
		{
			cerr << fname << ": Bad body for message." << endl;
			return 0;
		}
	}
	else
	{
		// The message has already ended.
		// Nothing to do.
	}

	t.body = strdup(buffer);

	len = pack_Mail(&t, (unsigned char*)buffer, bufferSize);
	pilotRec = new PilotRecord(buffer, len, 0, 0, 0);
	free_Mail(&t);

	return pilotRec;
}


#define BUFFERSIZE	(12000)
int PopMailConduit::doUnixStyle()
{
	FUNCTIONSETUP;
	QString filename;
	FILE *mailbox;
	// A buffer to hold the body and headers
	// of each message. 12000 isn't very big, but
	// since the mail application truncates at
	// 8000 the buffer is way larger than
	// the largest possible message actually
	// passed to the pilot.
	//
	//
	char *buffer=new char[BUFFERSIZE];
	int messageCount=0;

	PilotRecord *pilotRec=0L;

	{
		KConfig *config=kapp->getConfig();
		config->setGroup(PopMailOptions::configGroup());
	
		filename=config->readEntry("UNIX Mailbox");
		if (filename.isEmpty()) return 0;

		if (debug_level & SYNC_MINOR)
		{
			cerr << fname << ": Trying to read mailbox "
				<< filename << endl;
		}

		QFileInfo info(filename);
		if (!info.exists()) 
		{
			cerr << fname << ": Mailbox doesn't exist."
				<< endl;
			return -1;
		}

		if (debug_level & SYNC_MINOR)
		{
			cerr << fname << ": Mailbox found." << endl;
		}
	}

	mailbox=fopen(filename,"r");
	if (mailbox==0L)
	{
		cerr << fname << ": Can't open mailbox." << endl;
		perror(fname);
		return -1;
	}

	while (!feof(mailbox))
	{
		pilotRec=readMessage(mailbox,buffer,BUFFERSIZE);
		if  (pilotRec && writeRecord(pilotRec)>0)
		{
			messageCount++;
			if (debug_level & SYNC_MAJOR)
			{
				cerr << fname << ": Read message "
					<< messageCount << " from mailbox." 
					<< endl;
			}
		}
		else
		{
			cerr << fname << ": Message "
				<< messageCount << " couldn't be written."
				<< endl;
			showMessage("Error writing mail message to Pilot");
		}
		delete pilotRec;
	}

	if (debug_level & SYNC_MAJOR)
	{
		cerr << fname << ": Wrote "
			<< messageCount
			<< " messages to pilot." 
			<< endl;
	}

	return messageCount;
}
#undef BUFFERSIZE

