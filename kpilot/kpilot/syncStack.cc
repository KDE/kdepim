/* syncStack.cc                       KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/
#include "options.h"

static const char *syncStack_id = "$Id$";

#include <unistd.h>
#include <assert.h>

#include <qtimer.h>
#include <qfile.h>

#include <kservice.h>
#include <kservicetype.h>
#include <kuserprofile.h>
#include <klibloader.h>

#include "pilotUser.h"
#include "hotSync.h"
#include "interactiveSync.h"
#include "fileInstaller.h"
#include "kpilotSettings.h"

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

	addSyncLogEntry(i18n("KPilot %1 HotSync starting...\n")
		.arg(QString::fromLatin1(KPILOT_VERSION)));
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

ConduitProxy::ConduitProxy(KPilotDeviceLink *p,
	const QString &name,
	SyncAction::SyncMode m,
	bool local) :
	ConduitAction(p,name.latin1()),
	fDesktopName(name),
	fMode(m),
	fLocal(local)
{
	FUNCTIONSETUP;
}

/* static */ QStringList ConduitProxy::flagsForMode(SyncAction::SyncMode m)
{
	FUNCTIONSETUP;
	QStringList l;
	switch(m)
	{
	case eBackup :
		l.append(CSL1("--backup"));
		break;
	case eTest:
		l.append(CSL1("--test"));
		break;
	case eFastSync: /* FALLTHRU */
	case eHotSync:
		/* Nothing to do for fast or hotsync */
		break;
	case eFullSync:
		l.append(CSL1("--full"));
		break;
	case eCopyHHToPC:
		l.append(CSL1("--copyHHToPC"));
		break;
	case eCopyPCToHH:
		l.append(CSL1("--copyPCToHH"));
		break;
	case eRestore:
		kdWarning() << k_funcinfo << ": Running conduits during restore." << endl;
		l.append(CSL1("--test"));
		break;
	case eDefaultSync:
		assert(m!=eDefaultSync);
		break;
	}
	return l;
}

/* virtual */ bool ConduitProxy::exec()
{
	FUNCTIONSETUP;

	// query that service
	KSharedPtr < KService > o = KService::serviceByDesktopName(fDesktopName);
	if (!o)
	{
		kdWarning() << k_funcinfo
			<< ": Can't find desktop file for conduit "
			<< fDesktopName
			<< endl;
		addSyncLogEntry(i18n("Could not find conduit %1.").arg(fDesktopName));
		emit syncDone(this);
		return true;
	}


	// load the lib
#ifdef DEBUG
	DEBUGKPILOT << fname
		<< ": Loading desktop "
		<< fDesktopName
		<< " with lib "
		<< o->library()
		<< endl;
#endif

	fLibraryName = o->library();
	KLibFactory *factory = KLibLoader::self()->factory(
		QFile::encodeName(o->library()));
	if (!factory)
	{
		kdWarning() << k_funcinfo
			<< ": Can't load library "
			<< o->library()
			<< endl;
		addSyncLogEntry(i18n("Could not load conduit %1.").arg(fDesktopName));
		emit syncDone(this);
		return true;
	}

	QStringList l = flagsForMode(fMode);

	if (fLocal)
	{
		l.append(CSL1("--local"));
	}


#ifdef DEBUG
	DEBUGDAEMON << fname << ": Flags: " << fMode << endl;
#endif

	QObject *object = factory->create(fHandle,name(),"SyncAction",l);

	if (!object)
	{
		kdWarning() << k_funcinfo
			<< ": Can't create SyncAction."
			<< endl;
		addSyncLogEntry(i18n("Could not create conduit %1.").arg(fDesktopName));
		emit syncDone(this);
		return true;
	}

	fConduit = dynamic_cast<ConduitAction *>(object);

	if (!fConduit)
	{
		kdWarning() << k_funcinfo
			<< ": Can't cast to ConduitAction."
			<< endl;
		addSyncLogEntry(i18n("Could not create conduit %1.").arg(fDesktopName));
		emit syncDone(this);
		return true;
	}

	addSyncLogEntry(i18n("[Conduit %1]").arg(fDesktopName));

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
		kdError() << k_funcinfo
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

void ActionQueue::queueConduits(const QStringList &l,SyncAction::SyncMode m, bool /*local*/)
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
			<< ": Creating proxy with mode=" << m << endl;
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
		emit syncDone(this);
		return;
	}

	SyncAction *a = nextAction();

	if (!a)
	{
		kdWarning() << k_funcinfo
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

