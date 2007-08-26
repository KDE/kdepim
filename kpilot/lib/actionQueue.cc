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

#include <qtimer.h>

#include "actions.h"
#include "plugin.h"

#include "actionQueue.moc"




ActionQueue::ActionQueue(KPilotLink *d) :
	SyncAction(d,"ActionQueue")
	// The string lists have default constructors
{
	FUNCTIONSETUP;
}

ActionQueue::~ActionQueue()
{
	FUNCTIONSETUP;
	clear();
}

void ActionQueue::clear()
{
	SyncAction *del = 0L;
	while ( (del = nextAction()) )
	{
		delete del;
	}

	Q_ASSERT(isEmpty());
}

void ActionQueue::queueInit()
{
	FUNCTIONSETUP;

	addAction(new WelcomeAction(fHandle));
}

void ActionQueue::queueConduits(const QStringList &l,
	const SyncAction::SyncMode &m)
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
			DEBUGKPILOT << fname <<
				": Ignoring conduit " << *it << endl;
#endif
			continue;
		}

#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": Creating proxy with mode=" << m.name() << endl;
#endif
		ConduitProxy *cp = new ConduitProxy(fHandle,*it,m);
		addAction(cp);
	}
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
		DEBUGKPILOT << fname
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
		clear();
		delayDone();
		return;
	}

	SyncAction *a = nextAction();

	if (!a)
	{
		WARNINGKPILOT << "NULL action on stack."
			<< endl;
		return;
	}

#ifdef DEBUG
	DEBUGKPILOT << fname
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

	// Run the action picked from the queue when we get back
	// to the event loop.
	QTimer::singleShot(0,a,SLOT(execConduit()));
}

