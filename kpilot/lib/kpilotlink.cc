/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone <dan@kpilot.org>
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

// KPilot headers
#include "kpilotlink.moc"
#include "options.h"
#include "pilotCard.h"
#include "pilotUser.h"
#include "pilotSerialDatabase.h"
#include "pilotLocalDatabase.h"
#include "pilotSysInfo.h"

// pilot-link headers
#include <pi-buffer.h>
#include <pi-dlp.h>
#include <pi-file.h>
#include <pi-source.h>
#include <pi-socket.h>

// Qt headers
#include <QtCore/QDir>
#include <QtCore/QDateTime>
#include <QtCore/QEvent>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtGui/QApplication>

// KDE headers
#include <kconfig.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kio/netaccess.h>

// other headers
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

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
	FUNCTIONSETUPL(5);
	int subseconds = ChecksPerSecond;
	int ticktock = SecondsPerTickle;
	int timeout = fTimeout;
	DEBUGKPILOT << "Running for" << timeout << " seconds.";
	DEBUGKPILOT << "Done @" << (void *) fDone;

	while (!(*fDone))
	{
		QThread::msleep(1000/ChecksPerSecond);
		if (!(--subseconds))
		{
			if (timeout)
			{
				if (!(--timeout))
				{
					DEBUGKPILOT << "posting timeout event.";
					QApplication::postEvent(fHandle, new QEvent(static_cast<QEvent::Type>(KPilotLink::EventTickleTimeout)));
					break;
				}
			}
			subseconds=ChecksPerSecond;
			if (!(--ticktock))
			{
				ticktock=SecondsPerTickle;
				DEBUGKPILOT << "sending tickle from tickle thread.";
				fHandle->tickle();
			}
		}
	}
}









KPilotLink::KPilotLink( QObject *parent, const char *name ) :
	QObject( parent ),
	fPilotPath(QString()),
	fPilotUser(0L),
	fPilotSysInfo(0L),
	fTickleDone(true),
	fTickleThread(0L)

{
	FUNCTIONSETUP;

	if (name)
	{
		setObjectName(name);
	}

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

	if (fTickleThread)
	{
		stopTickle();
	}

	DEBUGKPILOT << "Done @" << (void *) (&fTickleDone);

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
		emit logProgress(QString(),
			(int) ((100.0 / total) * (float) n));

		if (installFile(*i, deleteFiles))
			k++;
		n++;
	}
	emit logProgress(QString(), 100);

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

