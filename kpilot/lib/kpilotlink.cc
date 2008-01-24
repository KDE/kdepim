/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
** Copyright (C) 2006-2007 Adriaan de Groot <groot@kde.org>
** Copyright (C) 2007 Jason 'vanRijn' Kasper <vR@movingparts.net>
**
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"



#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <iostream>

#include <pi-source.h>
#include <pi-socket.h>
#include <pi-dlp.h>
#include <pi-file.h>
#include <pi-buffer.h>

#include <qdir.h>
#include <qtimer.h>
#include <qdatetime.h>
#include <qthread.h>

#include <kconfig.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kio/netaccess.h>

#include "pilotUser.h"
#include "pilotSysInfo.h"
#include "pilotCard.h"
#include "pilotSerialDatabase.h"
#include "pilotLocalDatabase.h"

#include "kpilotlink.moc"

/** Class that handles periodically tickling the handheld through
*   the virtual tickle() method; deals with cancels through the
*   shared fDone variable.
*/
class TickleThread : public QThread
{
public:
	TickleThread(KPilotLink *d, bool *done, int timeout) :
		QThread(),
		fHandle(d),
		fDone(done),
		fTimeout(timeout)
	{ };
	virtual ~TickleThread();

	virtual void run();

	static const int ChecksPerSecond = 5;
	static const int SecondsPerTickle = 5;

private:
	KPilotLink *fHandle;
	bool *fDone;
	int fTimeout;
} ;

TickleThread::~TickleThread()
{
}

void TickleThread::run()
{
	FUNCTIONSETUP;
	int subseconds = ChecksPerSecond;
	int ticktock = SecondsPerTickle;
	int timeout = fTimeout;
	DEBUGKPILOT << fname << ": Running for "
		<< timeout << " seconds." << endl;
	DEBUGKPILOT << fname << ": Done @" << (void *) fDone << endl;

	while (!(*fDone))
	{
		QThread::msleep(1000/ChecksPerSecond);
		if (!(--subseconds))
		{
			if (timeout)
			{
				if (!(--timeout))
				{
					QApplication::postEvent(fHandle, new QEvent(static_cast<QEvent::Type>(KPilotLink::EventTickleTimeout)));
					break;
				}
			}
			subseconds=ChecksPerSecond;
			if (!(--ticktock))
			{
				ticktock=SecondsPerTickle;
				fHandle->tickle();
			}
		}
	}
}









KPilotLink::KPilotLink( QObject *parent, const char *name ) :
	QObject( parent, name ),
	fPilotPath(QString::null),
	fPilotUser(0L),
	fPilotSysInfo(0L),
	fTickleDone(true),
	fTickleThread(0L)

{
	FUNCTIONSETUP;

	fPilotUser = new KPilotUser();
	strncpy( fPilotUser->data()->username, "Henk Westbroek",
		sizeof(fPilotUser->data()->username)-1);
	fPilotUser->setLastSuccessfulSyncDate( 1139171019 );

	fPilotSysInfo = new KPilotSysInfo();
	memset(fPilotSysInfo->sysInfo()->prodID, 0,
		sizeof(fPilotSysInfo->sysInfo()->prodID));
	strncpy(fPilotSysInfo->sysInfo()->prodID, "LocalLink",
		sizeof(fPilotSysInfo->sysInfo()->prodID)-1);
	fPilotSysInfo->sysInfo()->prodIDLength =
		strlen(fPilotSysInfo->sysInfo()->prodID);
}

KPilotLink::~KPilotLink()
{
	FUNCTIONSETUP;
	KPILOT_DELETE(fPilotUser);
	KPILOT_DELETE(fPilotSysInfo);
}

/* virtual */ bool KPilotLink::event(QEvent *e)
{
	if ((int)e->type() == EventTickleTimeout)
	{
		stopTickle();
		emit timeout();
		return true;
	}
	else return QObject::event(e);
}

/*
Start a tickle thread with the indicated timeout.
*/
void KPilotLink::startTickle(unsigned int timeout)
{
	FUNCTIONSETUP;

	Q_ASSERT(fTickleDone);

	/*
	** We've told the thread to finish up, but it hasn't
	** done so yet - so wait for it to do so, should be
	** only 200ms at most.
	*/
	if (fTickleDone && fTickleThread)
	{
		fTickleThread->wait();
		KPILOT_DELETE(fTickleThread);
	}

	DEBUGKPILOT << fname << ": Done @" << (void *) (&fTickleDone) << endl;

	fTickleDone = false;
	fTickleThread = new TickleThread(this,&fTickleDone,timeout);
	fTickleThread->start();
}

void KPilotLink::stopTickle()
{
	FUNCTIONSETUP;
	fTickleDone = true;
	if (fTickleThread)
	{
		fTickleThread->wait();
		KPILOT_DELETE(fTickleThread);
	}
}

unsigned int KPilotLink::installFiles(const QStringList & l, const bool deleteFiles)
{
	FUNCTIONSETUP;

	QStringList::ConstIterator i,e;
	unsigned int k = 0;
	unsigned int n = 0;
	unsigned int total = l.count();

	for (i = l.begin(), e = l.end(); i != e; ++i)
	{
		emit logProgress(QString::null,
			(int) ((100.0 / total) * (float) n));

		if (installFile(*i, deleteFiles))
			k++;
		n++;
	}
	emit logProgress(QString::null, 100);

	return k;
}

void KPilotLink::addSyncLogEntry(const QString & entry, bool log)
{
	FUNCTIONSETUP;
	if (entry.isEmpty()) return;

	addSyncLogEntryImpl(entry);
	if (log)
	{
		emit logMessage(entry);
	}
}


/* virtual */ int KPilotLink::openConduit()
{
	return 0;
}

/* virtual */ int KPilotLink::pilotSocket() const
{
	return -1;
}

/* virtual */ PilotDatabase *KPilotLink::database( const DBInfo *info )
{
	FUNCTIONSETUP;
	return database( Pilot::fromPilot( info->name ) );
}

