/* KPilot
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"
#include "popmail-conduit.h"

extern "C"
{

long version_conduit_popmail = KPILOT_PLUGIN_API;
const char *id_conduit_popmail =
	"$Id$";

}

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

#include "popmailSettings.h"
#include "setupDialog.h"

#if 0
static void reset_Mail(struct Mail *t)
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
#endif

PopMailConduit::PopMailConduit(KPilotDeviceLink *d,
	const char *n,
	const QStringList &l) :
	ConduitAction(d,n,l)
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGCONDUIT << id_conduit_popmail << endl;
#endif
	fConduitName=i18n("KMail");
}

PopMailConduit::~PopMailConduit()
{
	FUNCTIONSETUP;
}

void PopMailConduit::doSync()
{
	FUNCTIONSETUP;

	int sent_count=0;
	int mode=MailConduitSettings::syncOutgoing();

#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": Outgoing mail disposition "
		<< mode << endl;
#endif

	if(mode)
	{
		sent_count=sendPendingMail(mode);
	}

	if (sent_count>0)
	{
		if (sent_count>0)
		{
			addSyncLogEntry(i18n("Sent one message",
				"Sent %n messages",sent_count));
		}
	}
}


// additional changes by Michael Kropfberger
int PopMailConduit::sendPendingMail(int mode)
{
	FUNCTIONSETUP;
	int count=0;

	if (mode==PopMailWidgetConfig::SendKMail)
	{
		count=sendViaKMail();
	}

	if (count == 0)
	{
		kdWarning() << k_funcinfo
			<< ": Mail was not sent at all!"
			<< endl;
		emit logError(i18n("No mail was sent."));
	}
	else if (count < 0)
	{
		kdWarning() << k_funcinfo
			<< ": Mail sending returned error " << count
			<< endl;
		emit logError(i18n("No mail could be sent."));
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


QString PopMailConduit::getKMailOutbox() const
{
	FUNCTIONSETUP;

	// Default to "outbox" with newer KMails.
	KSimpleConfig c(CSL1("kmailrc"),true);
	c.setGroup("General");

	QString outbox = c.readEntry("outboxFolder");
	if (outbox.isEmpty())
	{
		outbox = MailConduitSettings::outboxFolder();
	}

	if (outbox.isEmpty()) outbox=CSL1("outbox");

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
	QString kmailOutboxName = getKMailOutbox();

	DCOPClient *dcopptr = KApplication::kApplication()->dcopClient();
	if (!dcopptr)
	{
		kdWarning() << k_funcinfo
			<< ": Cannot get DCOP client."
			<< endl;
		KMessageBox::error(0L,
			i18n("Could not connect to DCOP server for "
				"the KMail connection."),
			i18n("Error Sending Mail"));
		return -1;
	}

	if (!dcopptr->isAttached())
	{
		dcopptr->attach();
	}

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
				<< ": Cannot open temp file."
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
				<< ": Cannot open temporary file for writing!"
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

		arg << kmailOutboxName << t.name();

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
		pilotRec->setCategory(3);
		pilotRec->setAttrib(pilotRec->getAttrib() & ~dlpRecAttrDirty);
		fDatabase->writeRecord(pilotRec);
		delete pilotRec;
		// This is ok since we got the mail with unpack mail..
		free_Mail(&theMail);

		count++;
	}

#if 0
	if ((count > 0)  && sendImmediate)
	{
		QByteArray data;
		if (dcopptr->send("kmail","KMailIface","sendQueued",data))
		{
			kdWarning() << k_funcinfo
				<< ": Could not flush queue."
				<< endl;
		}
	}
#endif

	return count;
}

// From pilot-link-0.8.7 by Kenneth Albanowski
// additional changes by Michael Kropfberger

void PopMailConduit::writeMessageToFile(FILE* sendf, struct Mail& theMail)
{
	FUNCTIONSETUP;

	QTextStream mailPipe(sendf, IO_WriteOnly);

	QString fromAddress = MailConduitSettings::emailAddress();
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
	DEBUGCONDUIT << fname << ": To: " << theMail.to << endl;
#endif


	if(theMail.body)
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": Sent body." << endl;
#endif
		mailPipe << theMail.body << "\r\n";
	}

	//insert the real signature file from disk
	QString signature = MailConduitSettings::signature();
	if(!signature.isEmpty())
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": Reading signature" << endl;
#endif

		QFile f(signature);
		if ( f.open(IO_ReadOnly) )
		{    // file opened successfully
			mailPipe << "-- \r\n";
			QTextStream t( &f );        // use a text stream
			while ( !t.eof() )
			{        // until end of file...
				mailPipe << t.readLine() << "\r\n";
			}
			f.close();
		}
	}
	mailPipe << "\r\n";

#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Done" << endl;
#endif
}


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
	DEBUGCONDUIT << id_conduit_popmail << endl;


	if (isTest())
	{
		doTest();
	}
	else if (isBackup())
	{
		emit logError(i18n("Cannot perform backup of mail database"));
	}
	else
	{
		fDatabase=new PilotSerialDatabase(pilotSocket(),
			CSL1("MailDB"));

		if (!fDatabase || !fDatabase->isDBOpen())
		{
			emit logError(i18n("Unable to open mail database on handheld"));
			KPILOT_DELETE(fDatabase);
			return false;
		}

		doSync();
		fDatabase->resetSyncFlags();
		KPILOT_DELETE(fDatabase);
	}
	delayDone();
	return true;
}
