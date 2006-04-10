/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** This defines the "ActionQueue", which is the pile of actions
** that will occur during a HotSync.
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
#include "options.h"

static const char *syncStack_id = "$Id$";

#include <unistd.h>

#include <qtimer.h>
#include <qfile.h>
#include <qdir.h>
#include <qtextcodec.h>
#include <QApplication>
#include <kservice.h>
#include <kservicetype.h>
#include <kuserprofile.h>
#include <klibloader.h>
#include <ksavefile.h>

#include "pilotUser.h"
#include "hotSync.h"
#include "interactiveSync.h"
#include "fileInstaller.h"
#include "kpilotSettings.h"
#include "pilotAppCategory.h"

#include "syncStack.moc"



WelcomeAction::WelcomeAction(KPilotDeviceLink *p) :
	SyncAction(p,"welcomeAction")
{
	FUNCTIONSETUP;

	(void) syncStack_id;
}

/* virtual */ bool WelcomeAction::exec()
{
	FUNCTIONSETUP;

	addSyncLogEntry(i18n("KPilot %1 HotSync starting...\n",
		 QString::fromLatin1(KPILOT_VERSION)));
	emit logMessage( i18n("Using encoding %1 on the handheld.", PilotAppCategory::codecName()) );
	emit syncDone(this);
	return true;
}

SorryAction::SorryAction(KPilotDeviceLink *p, const QString &s) :
	SyncAction(p,"sorryAction"),
	fMessage(s)
{
	if (fMessage.isEmpty())
	{
		fMessage = i18n("KPilot is busy and cannot process the "
			"HotSync right now.");
	}
}

bool SorryAction::exec()
{
	FUNCTIONSETUP;

	addSyncLogEntry(fMessage);
	return delayDone();
}

LocalBackupAction::LocalBackupAction(KPilotDeviceLink *p, const QString &d) :
	SyncAction(p,"LocalBackupAction"),
	fDir(d)
{
}

bool LocalBackupAction::exec()
{
	FUNCTIONSETUP;

	startTickle();

	QString dirname = fDir +
		PilotAppCategory::codec()->toUnicode(fHandle->getPilotUser()->getUserName()) +
		CSL1("/");
	QDir dir(dirname,QString::null,QDir::Unsorted,QDir::Files);

	if (!dir.exists())
	{
		emit logMessage( i18n("Cannot create local backup.") );
		return false;
	}

	logMessage( i18n("Creating local backup of databases in %1.", dirname) );
	addSyncLogEntry( i18n("Creating local backup ..") );
	qApp->processEvents();

	QStringList files = dir.entryList();

	for (QStringList::Iterator i = files.begin() ;
		i != files.end();
		++i)
	{
		KSaveFile::backupFile(dirname + (*i));
	}

	stopTickle();

	return delayDone();
}


ConduitProxy::ConduitProxy(KPilotDeviceLink *p,
	const QString &name,
	const SyncAction::SyncMode &m) :
	ConduitAction(p,name.toLatin1(),m.list()),
	fDesktopName(name)
{
	FUNCTIONSETUP;
}

/* virtual */ bool ConduitProxy::exec()
{
	FUNCTIONSETUP;

	// query that service
	KSharedPtr < KService > o = KService::serviceByDesktopName(fDesktopName);
	if (!o)
	{
		kWarning() << k_funcinfo
			<< ": Can't find desktop file for conduit "
			<< fDesktopName
			<< endl;
		addSyncLogEntry(i18n("Could not find conduit %1.", fDesktopName));
		emit syncDone(this);
		return true;
	}


	// load the lib
	fLibraryName = o->library();
#ifdef DEBUG
	DEBUGKPILOT << fname
		<< ": Loading desktop "
		<< fDesktopName
		<< " with lib "
		<< fLibraryName
		<< endl;
#endif

	KLibFactory *factory = KLibLoader::self()->factory(
		QFile::encodeName(fLibraryName));
	if (!factory)
	{
		kWarning() << k_funcinfo
			<< ": Can't load library "
			<< fLibraryName
			<< endl;
		addSyncLogEntry(i18n("Could not load conduit %1.", fDesktopName));
		emit syncDone(this);
		return true;
	}

	QStringList l = syncMode().list();

#ifdef DEBUG
	DEBUGDAEMON << fname << ": Flags: " << syncMode().name() << endl;
#endif

	QObject *object = factory->create(fHandle,name(),"SyncAction",l);

	if (!object)
	{
		kWarning() << k_funcinfo
			<< ": Can't create SyncAction."
			<< endl;
		addSyncLogEntry(i18n("Could not create conduit %1.", fDesktopName));
		emit syncDone(this);
		return true;
	}

	fConduit = dynamic_cast<ConduitAction *>(object);

	if (!fConduit)
	{
		kWarning() << k_funcinfo
			<< ": Can't cast to ConduitAction."
			<< endl;
		addSyncLogEntry(i18n("Could not create conduit %1.", fDesktopName));
		emit syncDone(this);
		return true;
	}

	addSyncLogEntry(i18n("[Conduit %1]", fDesktopName));

	// Handle the syncDone signal properly & unload the conduit.
	QObject::connect(fConduit,SIGNAL(syncDone(SyncAction *)),
		this,SLOT(execDone(SyncAction *)));
	// Proxy all the log and error messages.
	QObject::connect(fConduit,SIGNAL(logMessage(const QString &)),
		this,SIGNAL(logMessage(const QString &)));
	QObject::connect(fConduit,SIGNAL(logError(const QString &)),
		this,SIGNAL(logError(const QString &)));
	QObject::connect(fConduit,SIGNAL(logProgress(const QString &,int)),
		this,SIGNAL(logProgress(const QString &,int)));

	QTimer::singleShot(0,fConduit,SLOT(execConduit()));
	return true;
}

void ConduitProxy::execDone(SyncAction *p)
{
	FUNCTIONSETUP;

	if (p!=fConduit)
	{
		kError() << k_funcinfo
			<< ": Unknown conduit @"
			<< (long) p
			<< " finished."
			<< endl;
		emit syncDone(this);
		return;
	}

	delete p;
	addSyncLogEntry(CSL1("\n"),false); // Put bits of the conduit logs on separate lines
	emit syncDone(this);
}


ActionQueue::ActionQueue(KPilotDeviceLink *d) :
	SyncAction(d,"ActionQueue"),
	fReady(false)
	// The string lists have default constructors
{
	FUNCTIONSETUP;
}

ActionQueue::~ActionQueue()
{
	FUNCTIONSETUP;
}


void ActionQueue::queueInit(bool checkUser)
{
	FUNCTIONSETUP;

	addAction(new WelcomeAction(fHandle));

	if (checkUser)
	{
		addAction(new CheckUser(fHandle));
	}
}

void ActionQueue::queueConduits(const QStringList &l,const SyncAction::SyncMode &m, bool /*local*/)
{
	FUNCTIONSETUP;

	// Add conduits here ...
	//
	//
	for (QStringList::ConstIterator it = l.begin();
		it != l.end();
		++it)
	{
		if ((*it).startsWith(CSL1("internal_")))
		{
#ifdef DEBUG
			DEBUGDAEMON << fname <<
				": Ignoring conduit " << *it << endl;
#endif
			continue;
		}

#ifdef DEBUG
		DEBUGDAEMON << fname
			<< ": Creating proxy with mode=" << m.name() << endl;
#endif
		ConduitProxy *cp = new ConduitProxy(fHandle,*it,m);
		addAction(cp);
	}
}

void ActionQueue::queueInstaller(const QString &dir)
{
	addAction(new FileInstallAction(fHandle,dir));
}

void ActionQueue::queueCleanup()
{
	addAction(new CleanupAction(fHandle));
}

bool ActionQueue::exec()
{
	actionCompleted(0L);
	return true;
}

void ActionQueue::actionCompleted(SyncAction *b)
{
	FUNCTIONSETUP;

	if (b)
	{
#ifdef DEBUG
		DEBUGDAEMON << fname
			<< ": Completed action "
			<< b->name()
			<< endl;
#endif
		delete b;
	}

	if (isEmpty())
	{
		delayDone();
		return;
	}
	if ( deviceLink() && (!deviceLink()->tickle()) )
	{
		emit logError(i18n("The connection to the handheld "
			"was lost. Synchronization cannot continue."));
		SyncAction *del = 0L;
		while ( (del = nextAction()) )
		{
			delete del;
		}
		delayDone();
		return;
	}

	SyncAction *a = nextAction();

	if (!a)
	{
		kWarning() << k_funcinfo
			<< ": NULL action on stack."
			<< endl;
		return;
	}

#ifdef DEBUG
	DEBUGDAEMON << fname
		<< ": Will run action "
		<< a->name()
		<< endl;
#endif

	QObject::connect(a, SIGNAL(logMessage(const QString &)),
		this, SIGNAL(logMessage(const QString &)));
	QObject::connect(a, SIGNAL(logError(const QString &)),
		this, SIGNAL(logMessage(const QString &)));
	QObject::connect(a, SIGNAL(logProgress(const QString &, int)),
		this, SIGNAL(logProgress(const QString &, int)));
	QObject::connect(a, SIGNAL(syncDone(SyncAction *)),
		this, SLOT(actionCompleted(SyncAction *)));

	QTimer::singleShot(0,a,SLOT(execConduit()));
}

