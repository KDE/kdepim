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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/


#include "plugin.h"

class KSocket;

class PilotRecord;
class PilotDatabase;

class PopMailConduit : public ConduitAction
{
public:
	PopMailConduit(KPilotLink *d,
		const char *n=0L,
		const QStringList &l=QStringList());
	virtual ~PopMailConduit();

protected:
	virtual bool exec();

	// static PilotRecord *readMessage(FILE *mailbox,
	//	char *buffer,int bufferSize);

protected:
	void doSync();
	void doTest();

	// Pilot -> Sendmail
	//
	//
	int sendPendingMail(int mode /* unused */);
	// int sendViaSendmail();
	int sendViaKMail();
	// int sendViaSMTP();
	void writeMessageToFile(FILE* sendf, struct Mail& theMail);
	QString getKMailOutbox() const;

};

#endif
