/* popmail-conduit.cc			KPilot
**
** Copyright (C) 1998-2001 Dan Pilone
** Copyright (C) 1999,2000 Michael Kropfberger
**
** This file is part of the popmail conduit, a conduit for KPilot that
** synchronises the Pilot's email application with the outside world,
** which currently means:
**	-- sendmail or SMTP for outgoing mail
**	-- POP or mbox for incoming mail
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/



static const char *popmail_conduit_id=
	"$Id$";


#include "options.h"

#ifndef HAVE_GETDOMAINNAME
#include <sys/utsname.h>
#endif

#include <sys/types.h>
#include <sys/socket.h> 
#include <ctype.h>
#include <iostream.h>

#ifndef QDIR_H
#include <qdir.h>
#endif

#ifndef QTEXTSTREAM_H
#include <qtextstream.h>
#endif


#ifndef _KAPP_H
#include <kapp.h>
#endif

#ifndef _KCONFIG_H
#include <kconfig.h>
#endif

#ifndef _KMESSAGEBOX_H
#include <kmessagebox.h>
#endif

#ifndef _KSOCK_H
#include <ksock.h>
#endif

#ifndef _KICONLOADER_H
#include <kiconloader.h>
#endif

#ifndef _KDEBUG_H
#include <kdebug.h>
#endif

#ifndef _KABOUTDATA_H
#include <kaboutdata.h>
#endif

#ifndef _KTEMPFILE_H
#include <ktempfile.h>
#endif

#ifndef _DCOPCLIENT_H
#include <dcopclient.h>
#endif

#ifndef _KSIMPLECONFIG_H
#include <ksimpleconfig.h>
#endif

#ifndef _PILOT_SOURCE_H_
#include <pi-source.h>
#endif
#ifndef _PILOT_MAIL_H_
#include <pi-mail.h>
#endif
#ifndef _PILOT_DLP_H_
#include <pi-dlp.h>
#endif

#ifndef _KPILOT_CONDUITAPP_H
#include "conduitApp.h"
#endif

#ifndef _KPILOT_KPILOTCONFIG_H
#include "kpilotConfig.h"
#endif


#include "passworddialog.h"
#include "popmail-conduit.h"
#include "setupDialog.h"

extern "C" {
extern time_t parsedate(char * p);
}


// Just a convienience function [that doesn't
// belong in the class interface]
//
//
void showMessage(const QString &message)
{ 
  KMessageBox::error(0L, message, i18n("Error retrieving mail"));
}


// Errors returned from getXXResponse()
// and interpreted by showResponse():
//
//
#define	TIMEOUT	(-2)
#define PERROR	(-3)

#define BADPOP	(-333)

// This is a convenience function, that displays 
// a message with either
//
//	* an error text describing the error if ret < 0,
//	  errors as described by getXXXResponse
//	* an error text with the contents of the buffer,
//	  if ret>=0
//
// Since we're printing an error message, we're
// going to use someone else's fname, since the error
// doesn't come from us.
//
//
void showResponseResult(int const ret,
	const char *message,
	const char *buffer,
	const char *func)
{
	QString msg(i18n(message));

	if (ret==TIMEOUT)
	{
		msg.append(i18n(" (Timed out)"));
		DEBUGCONDUIT << func
			<< ": " << message
			<< endl;
	}
	if (ret==PERROR)
	{
		kdWarning() << func
			<< ": " << message
			<< perror
			<< endl ;
	}

	if (ret>=0)
	{
		DEBUGCONDUIT << func
			<< ": " << message
			<< endl;

		// Only add the buffer contents if they're interesting
		//
		//
		if (buffer && buffer[0])
		{
			msg.append("\n");
			msg.append(buffer);
			DEBUGCONDUIT << func
				<< ": " << buffer
				<< endl;
		}
	}


	showMessage(msg);
}

// This function waits for a response from a socket
// (with some kind of busy waiting :( ) and returns:
//
//	>=0	The number of bytes read
//	-2	If the response times out (currently
//		unimplemented)
//
//
static int getResponse(KSocket *s,char *buffer,const int bufsiz)
{
	FUNCTIONSETUP;
	int ret;

	// We read one byte less than the buffer
	// size, to account for the fact that we're
	// going to add a 0 byte to the end.
	//
	//
	do
	{
		ret=read(s->socket(), buffer, bufsiz-1);
	}
	while ((ret==-1) && (errno==EAGAIN));

	buffer[ret]=0;

	return ret;
}

// This function waits for a response from the
// POP3 server and then returns. It returns
//
//	BADPOP	If the response doesn't begin(*) with a +
//		(indicating OK in POP3)
//      >=0	If the response begins with a +, the number 
//		returned indicates the offset of the + in
//		the buffer.
//	TIMEOUT	If the POP3 server times out (currently
//		not implemented)
//
//
static int getPOPResponse(KSocket *s,const char *message,
	char *buffer,const int bufsiz)
{
	FUNCTIONSETUP;
	int i,ret;

	ret=getResponse(s,buffer,bufsiz);

	if (ret==TIMEOUT)
	{
		showResponseResult(ret,message,buffer,__FUNCTION__);
		return TIMEOUT;
	}

	// Skip any leading whitespace the POP3
	// server may print before the banner.
	//
	i=0;
	while(i<ret && isspace(buffer[i]) && i<bufsiz)
	{
		i++;
	}

	// If the POP3 server gives us a buffer full of
	// whitespace this test will fail as well.
	// Is that really bad?
	//
	//
	if(buffer[i] != '+')
	{
		showResponseResult(ret,message,buffer+i,__FUNCTION__);
		return BADPOP;
	}

	return i;
}


static void disconnectPOP(KSocket *s)
{
	FUNCTIONSETUP;

	// This buffer is kinda small, but because
	// the POP server's response isn't important
	// *anyway*...
	//
	char buffer[12];
	const char *quitmsg="QUIT\r\n";
	write(s->socket(),quitmsg,strlen(quitmsg));
	getPOPResponse(s,"QUIT command to POP server failed",buffer,12);
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
	return "popmail-conduit " VERSION;
}

void
PopMailConduit::doSync()
{
	FUNCTIONSETUP;

	KConfig& config = KPilotConfig::getConfig(PopMailOptions::PopGroup);
	int mode=0;
	int sent_count=0,received_count=0;

	addSyncLogMessage("Mail ");

	mode=config.readNumEntry("SyncOutgoing");
	if(mode)
	{
#ifdef DEBUG
		if (debug_level)
		{
			kdDebug() << fname << ": Sending pending mail" << endl;
		}
#endif
		sent_count=sendPendingMail(mode);
	}

	mode=config.readNumEntry("SyncIncoming");
	if(mode)
	{
#ifdef DEBUG
		if (debug_level)
		{
			kdDebug() << fname << ": Querying pop server." << endl;
		}
#endif
		received_count=retrieveIncoming(mode);
	}

	// Internationalisation and Qt issues be here.
	// This is an attempt at making a nice log 
	// message on the pilot, but it's obviously very
	// en locale-centric.
	//
	//
	if ((sent_count>0) || (received_count>0))
	{
		char buffer[128];
		if ((sent_count>0) && (received_count>0))
		{
			sprintf(buffer,"[ Sent %d message%c",
				sent_count,(sent_count>1) ? 's' : 0);
			addSyncLogMessage(buffer);
			sprintf(buffer,", Receved %d message%c",
				received_count,(received_count>1) ? 's' : 0);
			addSyncLogMessage(buffer);
		}
		if ((sent_count>0) && !(received_count>0))
		{
			sprintf(buffer,"[ Sent %d message%c",
				sent_count,(sent_count>1) ? 's' : 0);
			addSyncLogMessage(buffer);
		}
		if (!(sent_count>0) && (received_count>0))
		{
			sprintf(buffer,"[ Received %d message%c",
				received_count,(received_count>1) ? 's' : 0);
			addSyncLogMessage(buffer);
		}
		
		addSyncLogMessage(" ] ");
	}
	addSyncLogMessage("OK\n");
}

QWidget*
PopMailConduit::aboutAndSetup()
{
  return new PopMailOptions;
}

// additional changes by Michael Kropfberger
int PopMailConduit::sendPendingMail(int mode)
{
	FUNCTIONSETUP;
	int count=-1;


	if (mode == PopMailConduit::SEND_SMTP)
	{
		count=sendViaSMTP();
	}
	if (mode==PopMailConduit::SEND_SENDMAIL)
	{
		count=sendViaSendmail();
	}
	if (mode==PopMailConduit::SEND_KMAIL)
	{
		count=sendViaKMail();
	}

	if (count < 0)
	{
		kdWarning() << __FUNCTION__
			<< ": Mail was not sent at all!"
			<< endl;
	}
	else
	{
		DEBUGCONDUIT << fname 
			<< ": Sent "
			<< count 
			<< " messages"
			<< endl;
	}

	return count;
}

int PopMailConduit::retrieveIncoming(int mode)
{
	FUNCTIONSETUP;
	int count=0;

	if (mode==RECV_POP)
	{
		count=doPopQuery();
	}
	if (mode==RECV_UNIX)
	{
		count=doUnixStyle();
	}

	return count;
}




// additional changes by Michael Kropfberger
int PopMailConduit::sendViaSMTP() 
{
	FUNCTIONSETUP;
	int count=0;

  int i = 0;
  struct Mail theMail;
  QString smtpSrv;
  int smtpPort=25;
  QString currentDest, msg;
  PilotRecord* pilotRec;
  KSocket* smtpSocket;
  char buffer[0xffff];
  int ret;
  
  KConfig& config = KPilotConfig::getConfig(PopMailOptions::PopGroup);
  smtpSrv = config.readEntry("SMTPServer","localhost");
  smtpPort = config.readNumEntry("SMTPPort",25);
  
#ifdef DEBUG
	if (debug_level & SYNC_MINOR)
	{
		kdDebug() << fname
			<< ": Connecting to SMTP server "
			<< smtpSrv << " on port " << smtpPort
			<< endl;
	}
#endif

	smtpSocket = new KSocket(smtpSrv.latin1(),smtpPort); 
	CHECK_PTR(smtpSocket);
	if(smtpSocket->socket() < 0)
	{
		showResponseResult(PERROR,"Cannot connect to SMTP server",
			0L,__FUNCTION__);
		delete smtpSocket;
		return -1;
	}
	smtpSocket->enableRead(true);
	smtpSocket->enableWrite(true);

	// all do-while loops wait until data is avail
	ret=getResponse(smtpSocket,buffer,1024);

	if ((ret<0) || (strstr(buffer,"220")==0L))
	{
		showResponseResult(ret,"SMTP server failed to announce itself",
			buffer,__FUNCTION__);
		delete smtpSocket;
		return -1;
	}


#ifdef HAVE_GETDOMAINNAME
	// Now we're going to do some yucky things with
	// buffer -- one part will be used to hold the domain 
	// name and the other will be used to hold the
	// SMTP message being constructed.
	//
	//	buffer+1024	Area for domainname
	//	buffer		Area for message
	//
	// Now we're assuming that the message is never
	// longer than 1024 characters.
	//
	//
	ret = getdomainname(buffer+1024,1024);
	if (ret)
	{
		ret=errno;
		kdWarning() << __FUNCTION__
			<< ": getdomainname: "
			<< strerror(ret)
			<< endl;
	}
	else
	{
		DEBUGCONDUIT << fname
			<< ": Got domain name "
			<< buffer+1024
			<< endl;
	}

	sprintf(buffer, "EHLO %s\r\n",buffer+1024);
#else
	{
		struct utsname u;
		(void) uname(&u);

		DEBUGCONDUIT << fname
			<< ": Got uname.nodename "
			<< u.nodename
			<< endl;

		sprintf(buffer,"EHLO %s\r\n",u.nodename);
	}
#endif

#ifdef DEBUG
	if (debug_level & SYNC_MINOR)
	{
		kdDebug() << fname
			<< ": " << buffer
			;
	}
#endif
	write(smtpSocket->socket(), buffer, strlen(buffer));
	ret=getResponse(smtpSocket,buffer,1024);
	if ((ret < 0 ) || (strstr(buffer,"Hello")==0L))
	{
		showResponseResult(ret,"Couldn't EHLO to SMTP server",
			buffer,__FUNCTION__);
		//
		// Should we QUIT from SMTP server?
		//
		//
		delete smtpSocket;
    		return -1;
	}

  // Should probably read the prefs.. 
  // But, let's just get the mail..
	count=0;
  for(i = 0;; i++)
    {
      pilotRec = readNextRecordInCategory(1);
      if(pilotRec == 0L)
	break;
      if((pilotRec->getAttrib() & dlpRecAttrDeleted) 
               || (pilotRec->getAttrib() & dlpRecAttrArchived))
	{
	delete pilotRec;
	}
      else
	{
		count++;
	  unpack_Mail(&theMail, (unsigned char*)pilotRec->getData()
                      , pilotRec->getLen());
	  currentDest = "Mailing: ";
	  currentDest += theMail.to;


          QString fromAddress = config.readEntry("EmailAddress");
        sprintf(buffer,"MAIL FROM: %s\r\n",fromAddress.latin1());
          write(smtpSocket->socket(), buffer, strlen(buffer));

	getResponse(smtpSocket,buffer,1024);

          msg=buffer;
          if (msg.find("250") == -1){
            showMessage(i18n("Couldn't start sending new mail."));
            delete smtpSocket;
            return count;
          }  
             
          sprintf(buffer,"RCPT TO: %s\r\n",theMail.to);
             write(smtpSocket->socket(), buffer, strlen(buffer));

	getResponse(smtpSocket,buffer,1024);

          msg=buffer;
          if (msg.find("25") == -1){  // positively could be 250 or 251
            showMessage(i18n("The recipient doesn't exist!"));
            delete smtpSocket;
            return count;
          }  
          sprintf(buffer,"DATA\r\n");
             write(smtpSocket->socket(), buffer, strlen(buffer));

	getResponse(smtpSocket,buffer,1024);

          msg=buffer;
          if (msg.find("354") == -1){
            msg.prepend("Couldn't start writing mailbody\n");
            showMessage(msg);
            delete smtpSocket;
            return count;
          }  
          sprintf(buffer,"From: %s\r\n",fromAddress.latin1());
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
          if(!config.readEntry("Signature").isEmpty()) {
             QFile f(config.readEntry("Signature"));
             if ( f.open(IO_ReadOnly) ) {    // file opened successfully
                sprintf(buffer,"\r\n-- \r\n");
                write(smtpSocket->socket(), buffer, strlen(buffer));
                QTextStream t( &f );        // use a text stream
                while ( !t.eof() ) {        // until end of file...
                  sprintf(buffer,"%s\r\n",t.readLine().latin1());
                  write(smtpSocket->socket(), buffer, strlen(buffer));
                }
             f.close();
             }
          }
	    
          sprintf(buffer,".\r\n");  //end of mail
          write(smtpSocket->socket(), buffer, strlen(buffer));

	getResponse(smtpSocket,buffer,1024);

          msg=buffer;
          if (msg.find("250") == -1){
            showMessage(i18n("Couldn't send message."));
            delete smtpSocket;
            return -1;
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

	getResponse(smtpSocket,buffer,1024);

  msg=buffer;
  if (msg.find("221") == -1) 
    {
      msg.prepend("QUIT command to SMTP server failed.\n");
      showMessage(msg);
    }
  delete smtpSocket;
//   pilotLink->addSyncLogEntry("OK\n");}

	return count;
}

int PopMailConduit::sendViaSendmail() 
{
	FUNCTIONSETUP;
	int count=0;

  int i = 0;
  struct Mail theMail;
  QString sendmailCmd;
  QString currentDest;
  PilotRecord* pilotRec;
  
  KConfig& config = KPilotConfig::getConfig(PopMailOptions::PopGroup);

  sendmailCmd = config.readEntry("SendmailCmd");
  
//   pilotLink->addSyncLogEntry("Reading outgoing mail...");

  // Should probably read the prefs.. 
  // But, let's just get the mail..
  for(i = 0;i<100; i++)
    {
      FILE* sendf; // for talking to sendmail

#ifdef DEBUG
	if (debug_level & SYNC_TEDIOUS)
	{
		kdDebug() << fname << ": Reading " << i << "th message" << endl;
	}
#endif
      pilotRec = readNextRecordInCategory(1);
	if(pilotRec == 0L)
	{
#ifdef DEBUG
		if (debug_level)
		{
			kdDebug() << fname << ": Got a NULL record from "
				"readNextRecord" << endl;
		}
#endif
		break;
	}
      if((pilotRec->getAttrib() & dlpRecAttrDeleted) 
               || (pilotRec->getAttrib() & dlpRecAttrArchived))
	{
#ifdef DEBUG
		if (debug_level & SYNC_TEDIOUS )
		{
			kdDebug() << fname << ": Skipping deleted record." << endl;
		}
#endif
		delete pilotRec;
	}
      else
	{
	  unpack_Mail(&theMail, (unsigned char*)pilotRec->getData()
                      , pilotRec->getLen());
	  sendf = popen(sendmailCmd.latin1(), "w");
	  if(!sendf)
	    {
 	      KMessageBox::error(0L, "Cannot talk to sendmail!",
				   "Error Sending Mail");
		kdWarning() << __FUNCTION__ 
			<< ": Could not start sendmail.\n"
			<< __FUNCTION__ << ": " << count 
			<< " messages sent OK"
			<< endl ;
	      return -1;
	    }
	  currentDest = "Mailing: ";
	  currentDest += theMail.to;
	  writeMessageToFile(sendf, theMail);
	  pclose(sendf);
	  // Mark it as filed...
	  pilotRec->setCat(3);
	  pilotRec->setAttrib(pilotRec->getAttrib() & ~dlpRecAttrDirty);
	  writeRecord(pilotRec);
	  delete pilotRec;
	  // This is ok since we got the mail with unpack mail..
	  free_Mail(&theMail);
		count++;
	}
    }
//   free_MailAppInfo(&mailAppInfo);

#ifdef DEBUG
	if (debug_level & SYNC_MAJOR)
	{
		kdDebug() << fname << ": Sent " << count << " messages"
			<< endl;
	}
#endif

	return count;
}




QString PopMailConduit::getKMailOutbox() const
{
	FUNCTIONSETUP;
	// Read-only config file. This is code
	// suggested by Don Sanders. It must be
	// kept up-to-date with what KMail does.
	//
	KSimpleConfig c("kmailrc",true);
	c.setGroup("General");
	return c.readEntry("outboxFolder","outbox");
}

/*
 * This function uses KMail's DCOP interface to put all the
 * outgoing mail into the outbox.
 */
int PopMailConduit::sendViaKMail() 
{
	FUNCTIONSETUP;
	int count=0;
	bool sendImmediate = true;
	QString kmailOutboxName = getKMailOutbox();

	{
	KConfig& config = KPilotConfig::getConfig(PopMailOptions::PopGroup);
	sendImmediate = config.readBoolEntry("SendImmediate",true);
	}

	DCOPClient *dcopptr = KApplication::kApplication()->
		dcopClient();
	if (!dcopptr)
	{
		kdWarning() << __FUNCTION__
			<< ": Can't get DCOP client."
			<< endl;
		KMessageBox::error(0L, 
			i18n("Couldn't connect to DCOP server for\n"
				"the KMail connection."),
			i18n("Error Sending Mail"));
		return -1;
	}

	while (PilotRecord *pilotRec = readNextRecordInCategory(1))
	{
		DEBUGCONDUIT << fname 
			<< ": Reading " 
			<< count + 1
			<< "th message" 
			<< endl;

		if (pilotRec->isDeleted() || pilotRec->isArchived())
		{
			DEBUGCONDUIT << fname
				<< ": Skipping record."
				<< endl;
			continue;
		}

		struct Mail theMail;
		KTempFile t;
		t.setAutoDelete(true);

		if (t.status())
		{
			kdWarning() << __FUNCTION__
				<< ": Can't open temp file."
				<< endl;
			KMessageBox::error(0L, 
				i18n("Cannot open temporary file to store\n"
					"mail from Pilot in."),
				i18n("Error Sending Mail"));
			continue;
		}

		FILE *sendf = t.fstream();

		if (!sendf)
		{
			kdWarning() << __FUNCTION__
				<< ": Can't open temporary file for writing!"
				<< endl;
			KMessageBox::error(0L, 
				i18n("Cannot open temporary file to store\n"
					"mail from Pilot in."),
				i18n("Error Sending Mail"));
			continue;
		}

		unpack_Mail(&theMail, 
			(unsigned char*)pilotRec->getData(),
			pilotRec->getLen());
		writeMessageToFile(sendf, theMail);


		QByteArray data,returnValue;
		QCString returnType;
		QDataStream arg(data,IO_WriteOnly);

		arg << kmailOutboxName
			<< t.name();

		if (dcopptr->call("kmail",
			"KMailIface",
			"dcopAddMessage",
			data,
			returnType,
			returnValue,
			true))
		{
			kdWarning() << __FUNCTION__
				<< ": DCOP call failed."
				<< endl;

			KMessageBox::error(0L, 
				i18n("DCOP connection with KMail failed."),
				i18n("Error Sending Mail"));
			continue;
		}

		DEBUGCONDUIT << fname
			<< ": DCOP call returned "
			<< returnType
			<< " of "
			<< (const char *)returnValue
			<< endl;

		// Mark it as filed...
		pilotRec->setCat(3);
		pilotRec->setAttrib(pilotRec->getAttrib() & ~dlpRecAttrDirty);
		writeRecord(pilotRec);
		delete pilotRec;
		// This is ok since we got the mail with unpack mail..
		free_Mail(&theMail);

		count++;
	}

	if ((count > 0)  && sendImmediate)
	{
		QByteArray data;
		if (dcopptr->send("kmail","KMailIface","sendQueued",data))
		{
			kdWarning() << __FUNCTION__
				<< ": Couldn't flush queue."
				<< endl;
		}
	}

	return count;
}

// From pilot-link-0.8.7 by Kenneth Albanowski
// additional changes by Michael Kropfberger

void
PopMailConduit::writeMessageToFile(FILE* sendf, struct Mail& theMail)
{
	FUNCTIONSETUP;

	KConfig& config=KPilotConfig::getConfig(PopMailOptions::PopGroup); 
  QTextStream mailPipe(sendf, IO_WriteOnly);
  
  QString fromAddress = config.readEntry("EmailAddress");
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


#ifdef DEBUG
	if (debug_level & SYNC_MINOR)
	{
		kdDebug() << fname << ": To: " << theMail.to << endl;
	}
#endif


	if(theMail.body)
	{
#ifdef DEBUG
		if (debug_level & SYNC_MINOR)
		{
			kdDebug() << fname << ": Sent body." << endl;
		}
#endif
		mailPipe << theMail.body << "\r\n";
	}

  //insert the real signature file from disk
  if(!config.readEntry("Signature").isEmpty()) {
#ifdef DEBUG
	if (debug_level & SYNC_MINOR)
	{
		kdDebug() << fname << ": Reading signature" << endl;
	}
#endif

      QFile f(config.readEntry("Signature"));
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

#ifdef DEBUG
	if (debug_level & SYNC_MINOR)
	{
		kdDebug() << fname << ": Done" << endl;
	}
#endif
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

void PopMailConduit::retrievePOPMessages(KSocket *popSocket,int const msgcount,
	int const flags,
	char *buffer,int const bufsiz)
{
	FUNCTIONSETUP;
	int i,ret;

	for(i=1;i<(msgcount+1);i++) 
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
		ret=getPOPResponse(popSocket,"LIST command failed",
			buffer,bufsiz);
		if (ret<0) return;

		sscanf(buffer+ret, "%*s %*d %d", &len);

#ifdef DEBUG
		if (debug_level & SYNC_TEDIOUS)
		{
			kdDebug() << fname
				<< ": Message " << i
				<< " is " << len << " bytes long"
				<< endl;
		}
#endif

		if (len > 16000) 
		{
			kdWarning() << __FUNCTION__
				<< ": Skipped long message " << i
				<< endl;
			continue; 
		}

		sprintf(buffer, "RETR %d\r\n", i);
		write(popSocket->socket(), buffer, strlen(buffer));
		ret = getpopstring(popSocket->socket(), buffer);
		if ((ret < 0) || (buffer[0] != '+')) 
		{
			/* Weird */
			continue;
		} 
		else
		{
			buffer[ret] = 0;
		}

		msg = (char*)buffer;
		h = 1;
		for(;;) 
		{
			if (getpopstring(popSocket->socket(), msg) < 0) 
			{
				showMessage(i18n("Error reading message"));
				return;
			}

			if (h == 1) 
			{ 
				/* Header mode */
				if ((msg[0] == '.') && 
					(msg[1] == '\n') && (msg[2] == 0)) 
				{
					break; /* End of message */
				}
				if (msg[0] == '\n') 
				{
					h = 0;
					header(&t, 0);
				} 
				else 
				{
					header(&t, msg);
				}
				continue;
			}
			if ((msg[0] == '.') && 
				(msg[1] == '\n') && (msg[2] == 0)) 
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

		// Well, we've now got the message. 
		// I bet _you_ feel happy with yourself. 

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
			if (flags & POP_DELE)
			{ 
				sprintf(buffer, "DELE %d\r\n", i);
				write(popSocket->socket(), 
					buffer, strlen(buffer));
				getPOPResponse(popSocket,
					"Error deleting message",
					buffer,bufsiz);

			} 
		} 
		else 
		{
			showMessage(
				i18n("Error writing message to the Pilot."));
		}

		delete pilotRec;
		// This is ok since we used strdup's for them all..
		free_Mail(&t);
	}

}



int PopMailConduit::doPopQuery()
{
	FUNCTIONSETUP;

	KSocket* popSocket;
	KConfig& config = KPilotConfig::getConfig(PopMailOptions::PopGroup); 
	char buffer[0xffff];
	int offset;
	int flags=0;
	int msgcount;
  

	// Setup the flags to reflect the settings in
	// the config file.
	//
	//
	if (config.readNumEntry("LeaveMail") == 0)
	{
		flags |= POP_DELE ;
	}

	popSocket = new KSocket(config.readEntry("PopServer").latin1(),
		config.readNumEntry("PopPort")); 
	CHECK_PTR(popSocket);

#ifdef DEBUG
	if (debug_level & SYNC_MAJOR)
	{
		kdDebug() << fname 
			<< ": Attempted to connect to POP3 server "
			<< config.readEntry("PopServer")
			<< endl;
	}
#endif

	if(popSocket->socket() < 0)
	{
		showResponseResult(PERROR,
			"Cannot connect to POP server -- no socket",
			0L,__FUNCTION__);
		delete popSocket;
		return -1;
	}



	popSocket->enableRead(true);
	popSocket->enableWrite(true);

#ifdef DEBUG
	if (debug_level & SYNC_TEDIOUS)
	{
		kdDebug() << fname 
			<< ": Connected to POP3 server socket "
			<< popSocket->socket()
			<< endl ;
	}
#endif

	// The following code is based _HEAVILY_ :) 
	// on pilot-mail.c by Kenneth Albanowski
	// additional changes by Michael Kropfberger
	// all do-while loops wait until data is avail

	if (getPOPResponse(popSocket,"POP server failed to announce itself",
		buffer,1024)<0)
	{
		delete popSocket;
		return -1;
	}


	sprintf(buffer, "USER %s\r\n", config.readEntry("PopUser").latin1());
	write(popSocket->socket(), buffer, strlen(buffer));
	if (getPOPResponse(popSocket,"USER command to POP server failed",
		buffer,1024)<0)
	{
		delete popSocket;
		return -1;
	}

	if(config.readNumEntry("StorePass", 0))
	{
#ifdef DEBUG
		if (debug_level & SYNC_TEDIOUS)
		{
			kdDebug() << fname 
				<< ": Reading password from config."
				<< endl;
		}
#endif

		sprintf(buffer, "PASS %s\r\n", 
			config.readEntry("PopPass").latin1());
	}
	else
	{
		// Create a modal password dialog.
		//
		//
		PasswordDialog* passDialog = new PasswordDialog(
			i18n("Please Enter your POP password:"), 
			0L, "PopPassword", true);
		passDialog->show();
		if (passDialog->result()==QDialog::Accepted)
		{
			sprintf(buffer, "PASS %s\r\n", passDialog->password());
			delete passDialog;
		}
		else
		{
#ifdef DEBUG
			kdDebug() << fname
				<< ": Password dialog was canceled." 
				<< endl;
#endif
			delete passDialog;
			disconnectPOP(popSocket);
			delete popSocket;
			return -1;
		}
	}



	write(popSocket->socket(), buffer, strlen(buffer));
	if (getPOPResponse(popSocket,"PASS command to POP server failed",
		buffer,1024)<0)
	{
		disconnectPOP(popSocket);
		delete popSocket;
		return -1;
	}


	sprintf(buffer, "STAT\r\n");
	write(popSocket->socket(), buffer, strlen(buffer));
	if ((offset=getPOPResponse(popSocket,
		"STAT command to POP server failed",
		buffer,1024))<0)
	{
		disconnectPOP(popSocket);
		delete popSocket;
		return -1;
	}
		
	//sometimes looks like: "+OK ? messages (??? octets)
	//                  or: "+OK <user> has ? message (??? octets)
	//
	// [ The standard says otherwise ]
	//
	QString msg(buffer+offset);
	if (msg.find( config.readEntry("PopUser")) != -1) // with username
	{
		sscanf(buffer+offset, "%*s %*s %*s %d %*s", &msgcount);
	}
	else // normal version
	{
		sscanf(buffer+offset, "%*s %d %*s", &msgcount);
	}

#ifdef DEBUG
	if (debug_level & SYNC_TEDIOUS)
	{
		kdDebug() << fname
			<< ": POP STAT is "
			<< buffer+offset
			<< endl;
		kdDebug() << fname
			<< ": Will retrieve "
			<< msgcount << " messages."
			<< endl;
	}
#endif

	if(msgcount < 1)
	{
		// No messages, so bail early..
		disconnectPOP(popSocket);
		delete popSocket;
		return 0;
	}



	retrievePOPMessages(popSocket,flags,msgcount,buffer,1024);

	disconnectPOP(popSocket);
	delete popSocket;

	return msgcount;
}



/* static */ int PopMailConduit::skipBlanks(FILE *f,char *buffer,int buffersize)
{
	FUNCTIONSETUP;

	char *s;
	int count=0;

	while (!feof(f))
	{
		if (fgets(buffer,buffersize,f)==0L) break;
#ifdef DEBUG
		if (debug_level & SYNC_TEDIOUS)
		{
			kdDebug() <<  fname << ": Got line " << buffer ;
		}
#endif

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
#ifdef DEBUG
		if (debug_level & SYNC_TEDIOUS)
		{
			kdDebug() << fname << ": Looking for From line." << endl;
		}
#endif

		skipBlanks(f,line,LINESIZE);
		if (strncmp(line,"From ",5))
		{
			kdWarning() << __FUNCTION__
				<< ": No leading From line." << endl;
			return 0;
		}

#ifdef DEBUG
		if (debug_level & SYNC_TEDIOUS)
		{
			kdDebug() << fname << ": Found it." << endl;
		}
#endif
	}

	while ((skipBlanks(f,line,LINESIZE)==0) && !feof(f))
	{
		if ((line[0]=='.') && (line[1]=='\n') && (line[2] == 0))
		{
#ifdef DEBUG
			if (debug_level & SYNC_TEDIOUS)
			{
				kdDebug() << fname << ": Found end-of-headers " 
					"and end-of-message."
					<< endl;
			}
#endif
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
#ifdef DEBUG
			if (debug_level & SYNC_TEDIOUS)
			{
				kdDebug() << fname << ": Found end-of-headers" 
					<< endl;
			}
#endif
			// End of headers
			header(t,0);
			return count;
		}

		header(t,line);
		count++;
	}

#ifdef DEBUG
	if (debug_level & SYNC_TEDIOUS)
	{
		kdDebug() << fname << ": Read " << count << " lines." << endl;
	}
#endif
	strncpy(buf,line,bufsiz);
	return count;
}


/* static */ int PopMailConduit::readBody(FILE *f,char *buf,int bufsize)
{
	FUNCTIONSETUP;
	int count=0;
	int linelen=0;

#ifdef DEBUG
	if (debug_level & SYNC_TEDIOUS)
	{
		kdDebug() << fname << ": Buffer @" << (int) buf << endl;
	}
#endif

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

#ifdef DEBUG
		if (debug_level & SYNC_TEDIOUS)
		{
			kdDebug() << fname << ": Got line ["  
				<< (int) buf[0] << ',' << (int) buf[1] 
				<< ']'
				<< buf;
		}
#endif

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
		kdWarning() << __FUNCTION__ 
			<< ": Bad headers in message." << endl;
		return 0;
	}


	if (messageLength>0)
	{
		messageLength=strlen(buffer);
#ifdef DEBUG
		if (debug_level & SYNC_TEDIOUS)
		{
			kdDebug() << fname << ": Message so far:" << endl
				<< buffer << endl;
			kdDebug() << fname << ": Length " 
				<< messageLength << endl;
			kdDebug() << fname << ": Buffer @" << (int) buffer 
				<< endl;
		}
#endif

		if (readBody(mailbox,
			buffer+messageLength,
			bufferSize-messageLength) < 0)
		{
			kdWarning() << __FUNCTION__
				<< ": Bad body for message." << endl;
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
		KConfig& config=KPilotConfig::getConfig(PopMailOptions::PopGroup); 
	
		filename=config.readEntry("UNIX Mailbox");
		if (filename.isEmpty()) return 0;

#ifdef DEBUG
		if (debug_level & SYNC_MINOR)
		{
			kdDebug() << fname << ": Trying to read mailbox "
				<< filename << endl;
		}
#endif

		QFileInfo info(filename);
		if (!info.exists()) 
		{
			kdWarning() << __FUNCTION__ << ": Mailbox doesn't exist."
				<< endl;
			return -1;
		}

#ifdef DEBUG
		if (debug_level & SYNC_MINOR)
		{
			kdDebug() << fname << ": Mailbox found." << endl;
		}
#endif

	}

	mailbox=fopen(filename.latin1(),"r");
	if (mailbox==0L)
	{
		kdWarning() << __FUNCTION__ << ": Can't open mailbox:" 
			<< perror
			<< endl;
		return -1;
	}

	while (!feof(mailbox))
	{
		pilotRec=readMessage(mailbox,buffer,BUFFERSIZE);
		if  (pilotRec && writeRecord(pilotRec)>0)
		{
			messageCount++;
#ifdef DEBUG
			if (debug_level & SYNC_MAJOR)
			{
				kdDebug() << fname << ": Read message "
					<< messageCount << " from mailbox." 
					<< endl;
			}
#endif
		}
		else
		{
			kdWarning() << __FUNCTION__ << ": Message "
				<< messageCount << " couldn't be written."
				<< endl;
			showMessage(i18n("Error writing mail message to Pilot"));
		}
		delete pilotRec;
	}

#ifdef DEBUG
	if (debug_level & SYNC_MAJOR)
	{
		kdDebug() << fname << ": Wrote "
			<< messageCount
			<< " messages to pilot." 
			<< endl;
	}
#endif

	return messageCount;
}
#undef BUFFERSIZE

/* virtual */ QPixmap PopMailConduit::icon() const
{
	FUNCTIONSETUP;

	QPixmap p=KGlobal::iconLoader()->loadIcon("kmail", KIcon::Desktop);
	return p;
}

/* virtual */ void PopMailConduit::doTest()
{
	FUNCTIONSETUP;


	QString outbox = getKMailOutbox();

	DEBUGCONDUIT << fname
		<< ": KMail's outbox is "
		<< outbox
		<< endl;
}


int main(int argc, char* argv[])
{
	ConduitApp a(argc,argv,"popmail-conduit",
		I18N_NOOP("POP Mail Conduit"),
		KPILOT_VERSION);
	a.addAuthor("Michael Kropfberger","POP3 code");
	a.addAuthor("Adriaan de Groot","KDE2 port");

	PopMailConduit conduit(a.getMode());
	a.setConduit(&conduit);

	return a.exec();

	/* NOTREACHED */
	(void) popmail_conduit_id;
}



// $Log$
// Revision 1.20  2001/04/23 21:08:42  adridg
// Extra debugging for bug #24522
//
// Revision 1.19  2001/03/09 09:46:14  adridg
// Large-scale #include cleanup
//
// Revision 1.18  2001/03/06 12:13:32  adridg
// Fixed Solaris compilation problems (again?)
//
// Revision 1.17  2001/03/05 23:57:53  adridg
// Added KPILOT_VERSION
//
// Revision 1.16  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
// Revision 1.15  2001/02/09 15:59:28  habenich
// replaced "char *id" with "char *<filename>_id", because of --enable-final in configure
//
// Revision 1.14  2001/02/07 15:46:31  adridg
// Updated copyright headers for source release. Added CVS log. No code change.
//
