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

#include <qtimer.h>
#include <qfile.h>

#include <kservice.h>
#include <kservicetype.h>
#include <kuserprofile.h>
#include <klibloader.h>

#include "hotSync.h"
#include "interactiveSync.h"
#include "fileInstaller.h"

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

SorryAction::SorryAction(KPilotDeviceLink *p) :
	SyncAction(p,"sorryAction")
{
}

bool SorryAction::exec()
{
	FUNCTIONSETUP;

	addSyncLogEntry(i18n("KPilot is busy and cannot process the "
		"HotSync right now."));
	return delayDone();
}

ConduitProxy::ConduitProxy(KPilotDeviceLink *p,
	const QString &name,
	int m) :
	ConduitAction(p,name.latin1()),
	fDesktopName(name),
	fMode(m)
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
		kdWarning() << k_funcinfo
			<< ": Can't find desktop file for conduit "
			<< fDesktopName
			<< endl;
		addSyncLogEntry(i18n("Couldn't find conduit %1.").arg(fDesktopName));
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
		addSyncLogEntry(i18n("Couldn't load conduit %1.").arg(fDesktopName));
		emit syncDone(this);
		return true;
	}

	QStringList l;
	switch(fMode & ActionQueue::ActionMask)
	{
	case ActionQueue::Backup :
		l.append(CSL1("--backup"));
		break;
	default:
		;
	}
	if (fMode & ActionQueue::FlagTest)
	{
		l.append(CSL1("--test"));
	}
	if (fMode & ActionQueue::FlagLocal)
	{
		l.append(CSL1("--local"));
	}


	QObject *object = factory->create(fHandle,name(),"SyncAction",l);

	if (!object)
	{
		kdWarning() << k_funcinfo
			<< ": Can't create SyncAction."
			<< endl;
		addSyncLogEntry(i18n("Couldn't create conduit %1.").arg(fDesktopName));
		emit syncDone(this);
		return true;
	}

	fConduit = dynamic_cast<ConduitAction *>(object);

	if (!fConduit)
	{
		kdWarning() << k_funcinfo
			<< ": Can't cast to ConduitAction."
			<< endl;
		addSyncLogEntry(i18n("Couldn't create conduit %1.").arg(fDesktopName));
		emit syncDone(this);
		return true;
	}
	fConduit->setConfig(fConfig);

	logMessage(i18n("[Conduit %1]").arg(fDesktopName));

	QString conduitFlags = TODO_I18N("Running with flags: ");
	for (QStringList::ConstIterator i = l.begin() ; i!=l.end(); ++i)
	{
		conduitFlags.append(*i);
		conduitFlags.append(CSL1("  "));
	}
	
	logMessage(conduitFlags);

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
			<< (int) p
			<< " finished."
			<< endl;
		emit syncDone(this);
		return;
	}

	delete p;
	emit syncDone(this);
}

ActionQueue::ActionQueue(KPilotDeviceLink *d,
	KConfig *config,
	const QStringList &conduits,
	const QString &dir,
	const QStringList &files) :
	SyncAction(d,"ActionQueue"),
	fReady(false),
	fConfig(config),
	fInstallerDir(dir),
	fInstallerFiles(files),
	fConduits(conduits)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	if (!conduits.count())
	{
		DEBUGCONDUIT << fname << ": No conduits." << endl;
	}
	else
	{
		DEBUGCONDUIT << fname << ": Conduits : " << conduits.join(CSL1(" + ")) << endl;
	}
#endif

	kdWarning() << "SyncStack usage is deprecated." << endl;
}

ActionQueue::ActionQueue(KPilotDeviceLink *d) :
	SyncAction(d,"ActionQueue"),
	fReady(false),
	fConfig(0L)
	// The string lists have default constructors
{
	FUNCTIONSETUP;
}

ActionQueue::~ActionQueue()
{
	FUNCTIONSETUP;
}

void ActionQueue::prepare(int m)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGDAEMON << fname
		<< ": Using sync mode " << m
		<< endl;
#endif
	
	switch ( m & (Test | Backup | Restore | HotSync))
	{
	case Test:
	case Backup:
	case Restore:
	case HotSync:
		fReady=true;
		break;
	default:
		kdWarning() << k_funcinfo
			<< ": Strange sync mode " << m << " set. Aborting."
			<< endl;
		return;
	}

	queueInit(m);
	if (m & WithConduits)
		queueConduits(fConfig,fConduits,m);
	
	switch ( m & (Test | Backup | Restore | HotSync))
	{
	case Test:
		addAction(new TestLink(fHandle));
		break;
	case Backup:
		addAction(new BackupAction(fHandle));
		break;
	case Restore:
		addAction(new RestoreAction(fHandle));
		break;
	case HotSync:
		break;
	default:
		// We already checked for this case!
		fReady=false;
		return;
	}

	if (m & WithInstaller)
		queueInstaller(fInstallerDir,fInstallerFiles);

	queueCleanup();
}

void ActionQueue::queueInit(int m)
{
	FUNCTIONSETUP;

	addAction(new WelcomeAction(fHandle));

	if (m & WithUserCheck)
	{
		addAction(new CheckUser(fHandle));
	}
}

void ActionQueue::queueConduits(KConfig *config,const QStringList &l,int m)
{
	FUNCTIONSETUP;

	// Add conduits here ...
	//
	//
	for (QStringList::ConstIterator it = l.begin();
		it != l.end();
		++it)
	{
		ConduitProxy *cp = new ConduitProxy(fHandle,*it,m);
		cp->setConfig(config);
		addAction(cp);
	}
}

void ActionQueue::queueInstaller(const QString &dir, const QStringList &files)
{
	addAction(new FileInstallAction(fHandle,dir,files));
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

