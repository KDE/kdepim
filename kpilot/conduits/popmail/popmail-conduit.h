#ifndef _KPILOT_POPMAIL_CONDUIT_H
#define _KPILOT_POPMAIL_CONDUIT_H
/* popmail-conduit.h			KPilot
**
** Copyright (C) 1998,1999,2000 Dan Pilone
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


#include <stdio.h>

#include "plugin.h"

class KSocket;

class PilotRecord;
class PilotDatabase;

class PopMailConduit : public ConduitAction
{
public:
	PopMailConduit(KPilotDeviceLink *d,
		const char *n=0L,
		const QStringList &l=QStringList());
	virtual ~PopMailConduit();

	enum RetrievalMode {
		RECV_NONE=0,
		RECV_POP=1,
		RECV_UNIX=2 } ;
	enum SendMode {
		SEND_NONE=0,
		SEND_SENDMAIL=7,
		SEND_KMAIL=8,
		SEND_SMTP=12
		} ;

protected:
	virtual bool exec();

	static PilotRecord *readMessage(FILE *mailbox,
	        char *buffer,int bufferSize);

protected:
	void doSync();
	void doTest();

	// Pilot -> Sendmail
	//
	//
	int sendPendingMail(int mode /* unused */);
	int sendViaSendmail();
	int sendViaKMail();
	int sendViaSMTP();
	void writeMessageToFile(FILE* sendf, struct Mail& theMail);
	QString getKMailOutbox() const;


	// Local mail -> Pilot
	//
	//
	int retrieveIncoming(int mode);
	int doPopQuery();
	int doUnixStyle();


  // Taken from pilot-mail.c in pilot-link.0.8.7 by Kenneth Albanowski
  int getpopchar(int socket);
  int getpopstring(int socket, char * buf);
  int getpopresult(int socket, char * buf);

	// Helper functions for the POP mail handler
	//
	//
	typedef enum { POP_DELE=16 } retrieveFlags ;
	void retrievePOPMessages(KSocket *,
		int const msgcount,
		int const flags,
		char *buffer, int const buffer_size);

	static char* skipspace(char * c);
	static void header(struct Mail * m, char * t);

	/**
	* This function skips blank lines in the
	* input file; the first non-blank line
	* is returned in buf. size is the size of
	* buf in bytes; lines longer than size will
	* be truncated (this is like fgets()).
	* @return Number of blank lines skipped
	*/
	static int skipBlanks(FILE *,char *buf,int size);

	/**
	* Reads a file, using a buffer buf of size
	* size, and interprets this as mail headers.
	* The headers are placed in the struct Mail *.
	* If expectFrom is non-zero, requires a leading
	* "From address" line, as per standard UNIX mailbox
	* style.
	* @return 0 if the headers are bad (specifically,
	* if a required From line is missing)
	* @return <0 the number of header lines read,
	* if the message ends during the headers
	* return >0 the number of header lines read if
	* the message continues normally.
	*/
	static int readHeaders(FILE *,char *buf,int size,
		struct Mail *,int expectFrom);
	static int readBody(FILE *,char *buf,int size);

private:
	PilotDatabase *fDatabase;
};

#endif
