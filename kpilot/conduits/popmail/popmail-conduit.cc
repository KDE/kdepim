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
** Bug reports and questions can be sent to kde-pim@kde.org
*/

static const char *popmail_conduit_id=
	"$Id$";

#include "options.h"
#include <config.h>

#include <qsocket.h>
#include <qregexp.h>


#include <sys/types.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <ctype.h>

#include <unistd.h>
#include <errno.h>

#include <time.h>  // Needed by pilot-link include
#include <pi-version.h>
#if PILOT_LINK_MAJOR < 10
#include <pi-config.h>
#endif
#include <pi-mail.h>

#include <qdir.h>
#include <qtextstream.h>
#include <qtextcodec.h>

#include <kapplication.h>
#include <kmessagebox.h>
#include <ksock.h>
#include <kconfig.h>
#include <ksimpleconfig.h>
#include <dcopclient.h>
#include <ktempfile.h>

#include "pilotAppCategory.h"
#include "pilotSerialDatabase.h"

#include "passworddialog.h"
#include "popmail-factory.h"
#include "popmail-conduit.h"


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
#ifdef DEBUG
		DEBUGCONDUIT << func
			<< ": " << message
			<< endl;
#endif
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
#ifdef DEBUG
		DEBUGCONDUIT << func
			<< ": " << message
			<< endl;
#endif

		// Only add the buffer contents if they're interesting
		//
		//
		if (buffer && buffer[0])
		{
			msg.append(CSL1("\n"));
			msg.append(QString::fromLocal8Bit(buffer));
#ifdef DEBUG
			DEBUGCONDUIT << func
				<< ": " << buffer
				<< endl;
#endif
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
		showResponseResult(ret,message,buffer,"getPOPResponse");
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
		showResponseResult(ret,message,buffer+i,"getPOPResponse");
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

PopMailConduit::PopMailConduit(KPilotDeviceLink *d,
	const char *n,
	const QStringList &l) :
	ConduitAction(d,n,l)
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGCONDUIT<<popmail_conduit_id<<endl;
#endif
}

PopMailConduit::~PopMailConduit()
{
	FUNCTIONSETUP;
}

void
PopMailConduit::doSync()
{
	FUNCTIONSETUP;

	int mode=0;
	int sent_count=0,received_count=0;

	addSyncLogEntry(CSL1("Mail "));

	mode=fConfig->readNumEntry(PopmailConduitFactory::syncOutgoing);
#ifdef DEBUG
	DEBUGCONDUIT << fname 
		<< ": Outgoing mail mail disposition " 
		<< mode << endl;
#endif

	if(mode)
	{
		sent_count=sendPendingMail(mode);
	}

	mode=fConfig->readNumEntry(PopmailConduitFactory::syncIncoming);
#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Sending mail mode " << mode << endl;
#endif

	if(mode)
	{
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
		QString msg = CSL1("[ ");
		if (sent_count>0)
		{
			msg.append(i18n("Sent one message",
				"Sent %n messages",sent_count));
			if (received_count>0)
			{
				msg.append(CSL1(" ; "));
			}
		}
		if (received_count>0)
		{
			msg.append(i18n("Received one message",
				"Received %n messages",received_count));
		}
		msg.append(CSL1(" ] "));
		addSyncLogEntry(msg);
	}
	addSyncLogEntry(CSL1("OK\n"));
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
		kdWarning() << k_funcinfo
			<< ": Mail was not sent at all!"
			<< endl;
		emit logError(TODO_I18N("[ No mail could be sent. ]"));
	}
	else
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname 
			<< ": Sent "
			<< count 
			<< " messages"
			<< endl;
#endif
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



///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    ---- |   | ----- ----  -----                                           //
//   (     |\ /|   |   |   )   |        ___    _    ____  --            |    //
//    ---  | V |   |   |---    |   |/\  ___| |/ \  (     |  )  __  |/\ -+-   //
//       ) | | |   |   |       |   |   (   | |   |  \__  |--  /  \ |    |    //
//   ___/  |   |   |   |       |   |    \__| |   | ____) |    \__/ |     \   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
//
// SMTP Transfer Method (only sending)
//
// Additional changes by Michael Kropfberger
// Cleanup and fixing by Marko Grnroos <magi@iki.fi>, 2001
//

// Helper function to get the Fully Qualified Domain Name
QString getFQDomainName (const KConfig& config)
{
	FUNCTIONSETUP;

	QString fqDomainName;

	// Has the user given an explicit domain name?
	int useExplicitDomainName = 0;
	if (!config.readEntry("explicitDomainName", QString::null).isEmpty())
		useExplicitDomainName = 1;

	// Or was it given in the MAILDOMAIN environment variable?
	if (!useExplicitDomainName && getenv ("MAILDOMAIN"))
		useExplicitDomainName = 2;

#ifdef DEBUG
	DEBUGCONDUIT << fname << ": EDN=" << config.readEntry("explicitDomainName", QString::null) << endl;
	DEBUGCONDUIT << fname << ": useEDN=" << useExplicitDomainName << endl;
#endif

	if (useExplicitDomainName > 0) {
		// User has provided the FQDN either in config or in environment.

		if (useExplicitDomainName == 2) {
			fqDomainName = "$MAILDOMAIN";
		} else {
			// Use explicitly configured FQDN.
			// The domain name can also be the name of an environment variable.
			fqDomainName = config.readEntry("explicitDomainName", CSL1("$MAILDOMAIN"));
#ifdef DEBUG
			DEBUGCONDUIT << fname << ": got from config" << endl;
#endif
		}

		// Get FQDN from environment, from given variable.
		if (fqDomainName.left(1) == CSL1("$")) {
			QString envVar = fqDomainName.mid (1);
			char* envDomain = getenv (envVar.latin1());
			if (envDomain) {
				fqDomainName = envDomain;
#ifdef DEBUG
				DEBUGCONDUIT << fname << ": got from env" << endl;
#endif
			} else {
				// Ummh... It didn't exist, fall back to using system domain name
				useExplicitDomainName = false;

#ifdef DEBUG
				DEBUGCONDUIT << fname << ": Promised domain name environment variable "
							 << fqDomainName << " wasn't available." << endl;
#endif
			}
		}
	}

	if (useExplicitDomainName == 0) {
		// We trust in the system FQDN domain name

		struct utsname u;
		uname (&u);
		fqDomainName = u.nodename;

#ifdef DEBUG
		DEBUGCONDUIT << fname 
			<< ": Got uname.nodename " 
			<< u.nodename << endl;
#endif
	}

	return fqDomainName;
}

// Extracts email address from: "Firstname Lastname <mailbox@domain.tld>"
QString extractAddress (const QString& address) {
	int pos = address.find (QRegExp (CSL1("<.+>")));
	if (pos != -1) {
		return address.mid (pos+1, address.find (CSL1(">"), pos)-pos-1);
	} else
		return address;
}

QString buildRFC822Headers (const QString& sender,
	const struct Mail& theMail,
	const PopMailConduit&)
{
	FUNCTIONSETUP;

	QString buffer;
	QTextOStream bufs (&buffer);

	bufs << "From: " << sender << "\r\n";
	bufs << "To: " << theMail.to << "\r\n";
	if (theMail.cc)
		bufs << "Cc: " << theMail.cc << "\r\n";
	if (theMail.bcc)
		bufs << "Bcc: " << theMail.bcc << "\r\n";
	if (theMail.replyTo)
		bufs << "Reply-To: " << theMail.replyTo << "\r\n";
	if (theMail.subject)
		bufs << "Subject: " << theMail.subject << "\r\n";
	bufs << "X-mailer: " << "Popmail-Conduit " << KPILOT_VERSION << "\r\n\r\n";

	return buffer;
}

int sendSMTPCommand (KSocket& kSocket,
	const QString& sendBuffer,   // Buffer to send
	QTextOStream& logStream,     // For SMTP conversation logging
	const QString& logBuffer,    // Entire SMTP conversation log
	const QRegExp& expect,       // What do we expect as response (regexp)
	const QString& errormsg)     // Error message for error dialog
{
	FUNCTIONSETUP;

	// Send
	logStream << ">>> " << sendBuffer;
	write (kSocket.socket(), sendBuffer.latin1(), sendBuffer.length());

	// Receive confirmation
	QByteArray response (1024);
	int ret;
	ret = getResponse (&kSocket, response.data(), response.size());
	logStream << "<<< " << (const char*) response;

	// Check if the confirmation was correct
	if (QString(response).find (expect) == -1) {
		QString msg;
		msg = errormsg +
			i18n("\n\nPOPMail conduit sent to SMTP server:\n") +
			sendBuffer +
			i18n("\nSMTP server responded with:\n") +
			QString(response);

		showMessage (msg);

		kdWarning() << k_funcinfo << ": SMTP error: " << msg << endl;
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": SMTP error: " << logBuffer << endl;
#endif

		return -1;
	}

	return 0;
}

// Send
int PopMailConduit::sendViaSMTP ()
{
	FUNCTIONSETUP;
	QString			smtpSrv;					// Hostname of the SMTP server
	int				smtpPort = 25;
	int				handledCount = 0;			// Number of messages handled
	int				current = 0;				// Current message
	PilotRecord*	pilotRec;					// Message in Pilot format
	struct Mail		theMail;					// Message in internal format
	QCString		currentDest, msg;
	QString			sendBuffer;					// Output buffer
	int				ret;						// Return value from socket functions
	QByteArray		recvBuffer (1024);			// Input buffer, size is always 1024 bytes
	QString			domainName;					// The domain name of local host
	QString			logBuffer;					// SMTP conversation log
	QTextOStream	logStream (&logBuffer);		// Log stream, use with: log << stuff;

	// Read user-defined parameters
	smtpSrv = fConfig->readEntry ("SMTPServer", CSL1("localhost"));
	smtpPort = fConfig->readNumEntry ("SMTPPort", 25);

	//
	// Determine "domain name"
	// (FQDN, Fully Qualified Domain Name, ie., hostname+domainname)
	//

	// If we are behind a masquerading firewall, we can't trust in our
	// host- and domainname or even the IP number, so we have to fake them.
	// Some systems also don't set the domainname properly.

	domainName = getFQDomainName (*fConfig);

#ifdef DEBUG
	DEBUGCONDUIT << fname << ": " << domainName << endl;
#endif


	//
	//     Create socket connection to SMTP server
	//

#ifdef DEBUG
		DEBUGCONDUIT << fname << ": Connecting to SMTP server "
				  << smtpSrv << " on port " << smtpPort << endl;
#endif

	//
	//     Connect to SMTP server
	//

	// We open the socket with KSocket, because it's blocking, which
	// is much easier for us.
	KSocket kSocket (smtpSrv.latin1(), smtpPort); // Socket to SMTP server
	if (kSocket.socket() < 0) {
		showMessage (i18n("Cannot connect to SMTP server"));
		return -1;
	}
	kSocket.enableRead (true);
	kSocket.enableWrite (true);

	//
	//     SMTP Handshaking
	//

	// all do-while loops wait until data is avail
	ret = getResponse (&kSocket, recvBuffer.data(), recvBuffer.size());

	// Receive server handshake initiation
	if (ret<0 || QString(recvBuffer).find(CSL1("220")) == -1) {
		showMessage (i18n("SMTP server failed to announce itself")+
					 CSL1("\n\n")+logBuffer);
		return -1;
	}

	// Send EHLO, expect "250- ... Hello"
	sendBuffer.sprintf ("EHLO %s\r\n", domainName.latin1());
	if (sendSMTPCommand (kSocket, sendBuffer, logStream, logBuffer,
						 QRegExp(CSL1("^250")),
						 i18n("Couldn't EHLO to SMTP server")))
		return -1;

	//
	//     Should probably read the prefs..
	//     But, let's just get the mail..
	//

	// Handle each message in queue
	for (current=0, handledCount=0; ; current++) {

		// Get the Pilot message record
		pilotRec = fDatabase->readNextRecInCategory (1);
		if (pilotRec == 0L)
			break;

		// Do not handle the message if it is deleted or archived
		if ((pilotRec->getAttrib() & dlpRecAttrDeleted)
		   || (pilotRec->getAttrib() & dlpRecAttrArchived)) {
			delete pilotRec;
			continue; // Jumps to end of the for loop
		}

		// Ok, we shall send the message
		handledCount++;

		// Get the message data
		unpack_Mail (&theMail, (unsigned char*)pilotRec->getData(),
					 pilotRec->getLen());
		currentDest = "Mailing: ";
		currentDest += theMail.to;

		// Send "MAIL FROM: <...>", with the user-defined sender address
		QString sender = fConfig->readEntry("EmailAddress");
		QString fromAddress = extractAddress (sender);
		fromAddress.replace (QRegExp(CSL1("\\s")), QString::null); // Remove whitespaces

		// Send MAIL and receive response, expecting 250
        sendBuffer.sprintf ("MAIL FROM: <%s>\r\n", fromAddress.latin1());
		if (sendSMTPCommand (kSocket, sendBuffer, logStream, logBuffer,
							 QRegExp(CSL1("^250")),
							 i18n("Couldn't start sending new mail.")))
		{
			return handledCount;
		}

		//
		//     List recipients
		//

		// Get recipient(s) and clean up any whitespaces
		QCString recipients = theMail.to;
		if (QCString(theMail.cc).length()>1)
			recipients += QCString(",") + QCString (theMail.cc);
		if (QCString(theMail.bcc).length()>1)
			recipients += QCString(",") + QCString (theMail.bcc);
		recipients.replace (QRegExp(CSL1("\\s")), ""); // Remove whitespaces

		// Send to all recipients
		int rpos=0;
		int nextComma=0;
		for (rpos=0; rpos<int(recipients.length());) {
			QCString recipient;

			nextComma = recipients.find (',', rpos);
			if (nextComma > rpos) {
				recipient = recipients.mid (rpos, nextComma-rpos);
				rpos = nextComma+1;
			} else {
				recipient = recipients.mid (rpos);
				rpos = recipients.length(); // Will exit
			}

			// Send "RCPT TO: <...>", expect 25*
			sendBuffer.sprintf ("RCPT TO: <%s>\r\n", recipient.data());
			if (sendSMTPCommand (kSocket, sendBuffer, logStream, logBuffer,
								 QRegExp(CSL1("^25")),
								 i18n("The recipient doesn't exist!")))
				return handledCount;
		}

		// Send "DATA",
		sendBuffer.sprintf("DATA\r\n");
		if (sendSMTPCommand (kSocket, sendBuffer, logStream, logBuffer,
							 QRegExp(CSL1("^354")),
							 i18n("Unable to start writing mail body\n")))
            return handledCount;

		// Send RFC822 mail headers
		sendBuffer = buildRFC822Headers (sender, theMail, *this);
		write (kSocket.socket(), sendBuffer.latin1(), sendBuffer.length());

		// Send message body
		if (theMail.body) {
			sendBuffer = QString::fromLatin1 (theMail.body)+CSL1("\r\n");
			write (kSocket.socket(), sendBuffer.latin1(), sendBuffer.length());
		}

		//insert the real signature file from disk
		if (!fConfig->readEntry ("Signature").isEmpty()) {
			QFile f (fConfig->readEntry ("Signature"));
			if ( f.open (IO_ReadOnly) ) {    // file opened successfully
				sendBuffer.sprintf ("\r\n-- \r\n");
				write (kSocket.socket(), sendBuffer.latin1(), sendBuffer.length());

				// Read signature file with a text stream
                QTextStream t ( &f );
                while ( !t.eof() ) {        // until end of file...
					sendBuffer.sprintf ("%s\r\n", t.readLine().latin1());
					write (kSocket.socket(), sendBuffer.latin1(), sendBuffer.length());
                }
				f.close ();
			}
		}

	    // Send end-of-mail
		sendBuffer.sprintf(".\r\n");
		if (sendSMTPCommand (kSocket, sendBuffer, logStream, logBuffer,
							 QRegExp(CSL1("^250")),
							 i18n("Unable to send message")))
            return -1;

		// Mark it as filed...
		pilotRec->setCat (3);
		pilotRec->setAttrib (pilotRec->getAttrib() & ~dlpRecAttrDirty);
		fDatabase->writeRecord (pilotRec);
		delete pilotRec;

		// This is ok since we got the mail with unpack mail..
		free_Mail (&theMail);
	}

	sendBuffer.sprintf("QUIT\r\n");
	sendSMTPCommand (kSocket, sendBuffer, logStream, logBuffer,
					 QRegExp(CSL1("^221")),
					 i18n("QUIT command to SMTP server failed.\n"));

	return handledCount;
}



///////////////////////////////////////////////////////////////////////////////
//                   ----                 |             o |                  //
//                  (      ___    _       |        ___    |                  //
//                   ---  /   ) |/ \   ---| |/|/|  ___| | |                  //
//                      ) |---  |   | (   | | | | (   | | |                  //
//                  ___/   \__  |   |  ---| | | |  \__| | |                  //
///////////////////////////////////////////////////////////////////////////////

int PopMailConduit::sendViaSendmail() 
{
	FUNCTIONSETUP;
	int count=0;

  int i = 0;
  struct Mail theMail;
  QString sendmailCmd;
  QString currentDest;
  PilotRecord* pilotRec;

  sendmailCmd = fConfig->readEntry("SendmailCmd");

  // Should probably read the prefs..
  // But, let's just get the mail..
  for(i = 0;i<100; i++)
    {
      FILE* sendf; // for talking to sendmail

#ifdef DEBUG
	{
		DEBUGCONDUIT << fname << ": Reading " << i << "th message" << endl;
	}
#endif
      pilotRec = fDatabase->readNextRecInCategory(1);
	if(pilotRec == 0L)
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": Got a NULL record from "
			"readNextRecord" << endl;
#endif
		break;
	}
      if((pilotRec->getAttrib() & dlpRecAttrDeleted)
               || (pilotRec->getAttrib() & dlpRecAttrArchived))
	{
#ifdef DEBUG
		{
			DEBUGCONDUIT << fname << ": Skipping deleted record." << endl;
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
 	      KMessageBox::error(0L, TODO_I18N("Cannot talk to sendmail!"),
				   TODO_I18N("Error Sending Mail"));
		kdWarning() << k_funcinfo
			<< ": Could not start sendmail." << endl;
		kdWarning() << k_funcinfo << ": " << count
			<< " messages sent OK"
			<< endl ;
	      return -1;
	    }
	  // TODO: Is currentDest used at all?
	  currentDest = CSL1("Mailing: ");
	  currentDest += PilotAppCategory::codec()->toUnicode(theMail.to);
	  writeMessageToFile(sendf, theMail);
	  pclose(sendf);
	  // Mark it as filed...
	  pilotRec->setCat(3);
	  pilotRec->setAttrib(pilotRec->getAttrib() & ~dlpRecAttrDirty);
	 fDatabase->writeRecord(pilotRec);
	  delete pilotRec;
	  // This is ok since we got the mail with unpack mail..
	  free_Mail(&theMail);
		count++;
	}
    }
//   free_MailAppInfo(&mailAppInfo);

#ifdef DEBUG
	{
		DEBUGCONDUIT << fname << ": Sent " << count << " messages"
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
	// TODO: Completely broken since KMail disposed of this
	// setting in KDE 3.0. No idea how to fix short of i18n("outbox").
	KSimpleConfig c(CSL1("kmailrc"),true);
	c.setGroup("General");

	QString outbox = c.readEntry("outboxFolder",QString::null);
	if (outbox.isEmpty())
	{
		KConfigGroupSaver gs(fConfig,PopmailConduitFactory::group);
		outbox = fConfig->readEntry("outboxFolder");
	}
	return outbox;
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

	sendImmediate = fConfig->readBoolEntry("SendImmediate",true);

	DCOPClient *dcopptr = KApplication::kApplication()->
		dcopClient();
	if (!dcopptr)
	{
		kdWarning() << k_funcinfo
			<< ": Can't get DCOP client."
			<< endl;
		KMessageBox::error(0L,
			i18n("Couldn't connect to DCOP server for "
				"the KMail connection."),
			i18n("Error Sending Mail"));
		return -1;
	}

	dcopptr->attach();
	while (PilotRecord *pilotRec = fDatabase->readNextRecInCategory(1))
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname 
			<< ": Reading " 
			<< count + 1
			<< "th message" 
			<< endl;
#endif

		if (pilotRec->isDeleted() || pilotRec->isArchived())
		{
#ifdef DEBUG
			DEBUGCONDUIT << fname
				<< ": Skipping record."
				<< endl;
#endif
			continue;
		}

		struct Mail theMail;
		KTempFile t;
		t.setAutoDelete(true);

		if (t.status())
		{
			kdWarning() << k_funcinfo
				<< ": Can't open temp file."
				<< endl;
			KMessageBox::error(0L,
				i18n("Cannot open temporary file to store "
					"mail from Pilot in."),
				i18n("Error Sending Mail"));
			continue;
		}

		FILE *sendf = t.fstream();

		if (!sendf)
		{
			kdWarning() << k_funcinfo
				<< ": Can't open temporary file for writing!"
				<< endl;
			KMessageBox::error(0L, 
				i18n("Cannot open temporary file to store "
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

		if (!dcopptr->call("kmail",
			"KMailIface",
			"dcopAddMessage(QString,QString)",
			data,
			returnType,
			returnValue,
			true))
		{
			kdWarning() << k_funcinfo
				<< ": DCOP call failed."
				<< endl;

			KMessageBox::error(0L,
				i18n("DCOP connection with KMail failed."),
				i18n("Error Sending Mail"));
			continue;
		}

#ifdef DEBUG
		DEBUGCONDUIT << fname
			<< ": DCOP call returned "
			<< returnType
			<< " of "
			<< (const char *)returnValue
			<< endl;
#endif

		// Mark it as filed...
		pilotRec->setCat(3);
		pilotRec->setAttrib(pilotRec->getAttrib() & ~dlpRecAttrDirty);
		fDatabase->writeRecord(pilotRec);
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
			kdWarning() << k_funcinfo
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

  QTextStream mailPipe(sendf, IO_WriteOnly);

  QString fromAddress = fConfig->readEntry("EmailAddress");
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
  mailPipe << "X-mailer: " << "Popmail-Conduit " << KPILOT_VERSION << "\r\n";
  mailPipe << "\r\n";


#ifdef DEBUG
	{
		DEBUGCONDUIT << fname << ": To: " << theMail.to << endl;
	}
#endif


	if(theMail.body)
	{
#ifdef DEBUG
		{
			DEBUGCONDUIT << fname << ": Sent body." << endl;
		}
#endif
		mailPipe << theMail.body << "\r\n";
	}

  //insert the real signature file from disk
  if(!fConfig->readEntry("Signature").isEmpty()) {
#ifdef DEBUG
	{
		DEBUGCONDUIT << fname << ": Reading signature" << endl;
	}
#endif

      QFile f(fConfig->readEntry("Signature"));
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
	{
		DEBUGCONDUIT << fname << ": Done" << endl;
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
		{
			DEBUGCONDUIT << fname
				<< ": Message " << i
				<< " is " << len << " bytes long"
				<< endl;
		}
#endif

		if (len > 16000) 
		{
			kdWarning() << k_funcinfo
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
		if (fDatabase->writeRecord(pilotRec) > 0) 
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
	char buffer[0xffff];
	int offset;
	int flags=0;
	int msgcount;


	// Setup the flags to reflect the settings in
	// the config file.
	//
	//
	if (fConfig->readNumEntry("LeaveMail") == 0)
	{
		flags |= POP_DELE ;
	}

	popSocket = new KSocket(fConfig->readEntry("PopServer").latin1(),
		fConfig->readNumEntry("PopPort"));
	CHECK_PTR(popSocket);

#ifdef DEBUG
	{
		DEBUGCONDUIT << fname
			<< ": Attempted to connect to POP3 server "
			<< fConfig->readEntry("PopServer")
			<< endl;
	}
#endif

	if(popSocket->socket() < 0)
	{
		showResponseResult(PERROR,
			"Cannot connect to POP server -- no socket",
			0L,"doPopQuery");
		delete popSocket;
		return -1;
	}



	popSocket->enableRead(true);
	popSocket->enableWrite(true);

#ifdef DEBUG
	{
		DEBUGCONDUIT << fname
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


	sprintf(buffer, "USER %s\r\n", fConfig->readEntry("PopUser").latin1());
	write(popSocket->socket(), buffer, strlen(buffer));
	if (getPOPResponse(popSocket,"USER command to POP server failed",
		buffer,1024)<0)
	{
		delete popSocket;
		return -1;
	}

	if(fConfig->readNumEntry("StorePass", 0))
	{
#ifdef DEBUG
		{
			DEBUGCONDUIT << fname
				<< ": Reading password from config."
				<< endl;
		}
#endif

		sprintf(buffer, "PASS %s\r\n",
			fConfig->readEntry("PopPass").latin1());
	}
	else
	{
		// Create a modal password dialog.
		//
		//
		PasswordDialog* passDialog = new PasswordDialog(
			i18n("Please enter your POP password:"),
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
			DEBUGCONDUIT << fname
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
	// Surely POP3 speaks latin1?
	QString msg(QString::fromLatin1(buffer+offset));
	if (msg.find( fConfig->readEntry("PopUser")) != -1) // with username
	{
		sscanf(buffer+offset, "%*s %*s %*s %d %*s", &msgcount);
	}
	else // normal version
	{
		sscanf(buffer+offset, "%*s %d %*s", &msgcount);
	}

#ifdef DEBUG
	{
		DEBUGCONDUIT << fname
			<< ": POP STAT is "
			<< buffer+offset
			<< endl;
		DEBUGCONDUIT << fname
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



	retrievePOPMessages(popSocket,msgcount,flags,buffer,1024);

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
		{
			DEBUGCONDUIT <<  fname << ": Got line " << buffer ;
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
		{
			DEBUGCONDUIT << fname << ": Looking for From line." << endl;
		}
#endif

		skipBlanks(f,line,LINESIZE);
		if (strncmp(line,"From ",5))
		{
			kdWarning() << k_funcinfo
				<< ": No leading From line." << endl;
			return 0;
		}

#ifdef DEBUG
		{
			DEBUGCONDUIT << fname << ": Found it." << endl;
		}
#endif
	}

	while ((skipBlanks(f,line,LINESIZE)==0) && !feof(f))
	{
		if ((line[0]=='.') && (line[1]=='\n') && (line[2] == 0))
		{
#ifdef DEBUG
			{
				DEBUGCONDUIT << fname << ": Found end-of-headers " 
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
			{
				DEBUGCONDUIT << fname << ": Found end-of-headers"
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
	{
		DEBUGCONDUIT << fname << ": Read " << count << " lines." << endl;
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
	{
		DEBUGCONDUIT << fname << ": Buffer @" << (int) buf << endl;
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
		{
			DEBUGCONDUIT << fname << ": Got line ["
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
		kdWarning() << k_funcinfo 
			<< ": Bad headers in message." << endl;
		return 0;
	}


	if (messageLength>0)
	{
		messageLength=strlen(buffer);
#ifdef DEBUG
		{
			DEBUGCONDUIT << fname << ": Message so far:" << endl
				<< buffer << endl;
			DEBUGCONDUIT << fname << ": Length " 
				<< messageLength << endl;
			DEBUGCONDUIT << fname << ": Buffer @" << (int) buffer 
				<< endl;
		}
#endif

		if (readBody(mailbox,
			buffer+messageLength,
			bufferSize-messageLength) < 0)
		{
			kdWarning() << k_funcinfo
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
		filename=fConfig->readEntry("UNIX Mailbox");
		if (filename.isEmpty()) return 0;

#ifdef DEBUG
		{
			DEBUGCONDUIT << fname << ": Trying to read mailbox "
				<< filename << endl;
		}
#endif

		QFileInfo info(filename);
		if (!info.exists())
		{
			kdWarning() << k_funcinfo
				<< ": Mailbox doesn't exist."
				<< endl;
			return -1;
		}

#ifdef DEBUG
		{
			DEBUGCONDUIT << fname << ": Mailbox found." << endl;
		}
#endif

	}

	mailbox=fopen(filename.latin1(),"r");
	if (mailbox==0L)
	{
		kdWarning() << k_funcinfo << ": Can't open mailbox:" 
			<< perror
			<< endl;
		return -1;
	}

	while (!feof(mailbox))
	{
		pilotRec=readMessage(mailbox,buffer,BUFFERSIZE);
		if  (pilotRec && fDatabase->writeRecord(pilotRec)>0)
		{
			messageCount++;
#ifdef DEBUG
			{
				DEBUGCONDUIT << fname << ": Read message "
					<< messageCount << " from mailbox." 
					<< endl;
			}
#endif
		}
		else
		{
			kdWarning() << k_funcinfo << ": Message "
				<< messageCount << " couldn't be written."
				<< endl;
			showMessage(i18n("Error writing mail message to Pilot"));
		}
		delete pilotRec;
	}

#ifdef DEBUG
	{
		DEBUGCONDUIT << fname << ": Wrote "
			<< messageCount
			<< " messages to pilot."
			<< endl;
	}
#endif

	return messageCount;
}
#undef BUFFERSIZE

/* virtual */ void PopMailConduit::doTest()
{
	FUNCTIONSETUP;


	QString outbox = getKMailOutbox();

#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": KMail's outbox is "
		<< outbox
		<< endl;
#endif
}

/* virtual */ bool PopMailConduit::exec()
{
	FUNCTIONSETUP;
	DEBUGCONDUIT<<popmail_conduit_id<<endl;

	if (!fConfig) return false;

	KConfigGroupSaver cfgs(fConfig,PopmailConduitFactory::group);

	fDatabase=new PilotSerialDatabase(pilotSocket(),
		CSL1("MailDB"),this,"MailDB");

	if (!fDatabase || !fDatabase->isDBOpen())
	{
		emit logError(i18n("Unable to open mail database on handheld"));
		KPILOT_DELETE(fDatabase);
		return false;
	}

	if (isTest())
	{
		doTest();
	}
	else if (isBackup())
	{
		emit logError(TODO_I18N("Cannot perform backup on mail database"));
	}
	else
	{
		doSync();
		fDatabase->resetSyncFlags();
	}

	KPILOT_DELETE(fDatabase);
	emit syncDone(this);
	return true;
}
