/* syncStack.cc                       KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This defines the "SyncStack", which is the pile of actions
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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
** MA 02139, USA.
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
	switch(fMode && SyncStack::ActionMask)
	{
	case SyncStack::Backup :
		l.append(CSL1("--backup"));
		break;
	default:
		;
	}
	if (fMode & SyncStack::FlagTest)
	{
		l.append(CSL1("--test"));
	}
	if (fMode & SyncStack::FlagLocal)
	{
		l.append(CSL1("--local"));
	}


	QObject *object = factory->create(fHandle,0L,"SyncAction",l);

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

SyncStack::SyncStack(KPilotDeviceLink *d,
	KConfig *config,
	const QStringList &conduits,
	const QString &dir,
	const QStringList &files) :
	SyncAction(d,"SyncStack"),
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
}

SyncStack::~SyncStack()
{
	FUNCTIONSETUP;
}

void SyncStack::prepare(int m)
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

	addAction(new CleanupAction(fHandle));

	if (m & WithInstaller)
	{
		addAction(new FileInstallAction(fHandle,fInstallerDir,fInstallerFiles));
	}

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

	// Add conduits here ...
	//
	//
	for (QStringList::ConstIterator it = fConduits.begin();
		it != fConduits.end();
		++it)
	{
		ConduitProxy *cp =new ConduitProxy(fHandle,*it,m);
		cp->setConfig(fConfig);
		addAction(cp);
	}

	if (m & WithUserCheck)
	{
		addAction(new CheckUser(fHandle));
	}

	addAction(new WelcomeAction(fHandle));
}

bool SyncStack::exec()
{
	actionCompleted(0L);
	return true;
}

void SyncStack::actionCompleted(SyncAction *b)
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

