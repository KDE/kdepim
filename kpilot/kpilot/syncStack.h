#ifndef _KPILOT_SYNCSTACK_H
#define _KPILOT_SYNCSTACK_H
/* syncStack.h                        KPilot
**
** Copyright (C) 1998-2001,2003 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** This defines the "ActionQueue", which is the sequence of actions
** that will occur during a HotSync. There's also two fairly
** unimportant SyncActions defined.
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

#include <qptrqueue.h>

#include "plugin.h"

/**
* This used to be called SyncStack, and while a stack is cool
* for some things, it actually rather confuses the issue because
* you _use_ this class for specifying "do this, then that, then ..."
* and in program code you need to reverse that order when adding
* items to a stack. So now it's a Queue, FIFO, and program
* code looks more normal.
*
* Also updated to Qt3 in all aspects.
*/

/**
* The ActionQueue is a meta-action, which handles running a bunch of SyncActions
* in sequence. It is a SyncAction itself, so it can even be queued on another
* ActionQueue.
*
* An ActionQueue is constructed with a @p device. As usual, you should connect
* the device's deviceReady(KPilotDeviceLink*) signal with the exec() slot -- or something to that effect.
* The ActionQueue will then run all the actions in the queue in sequence.
*
*/

class ActionQueue : public SyncAction
{
Q_OBJECT
public:
	ActionQueue(KPilotDeviceLink *device);

	virtual ~ActionQueue();

private:
	QPtrQueue < SyncAction > SyncActionQueue;

public:
	bool isEmpty() const { return SyncActionQueue.isEmpty(); };
	/**
	* You can push your own actions onto the stack, but you should
	* only do so if you don't call prepare().
	*/
	void addAction(SyncAction * a) { SyncActionQueue.enqueue(a); };

protected:
	void clear() { SyncActionQueue.clear(); };
	SyncAction *nextAction() { return SyncActionQueue.dequeue(); };

	bool fReady;

	QString fInstallerDir;
	QStringList fInstallerFiles;
	QStringList fConduits;

public:
	/**
	* Call these queue*() functions to append standard functional
	* blocks. You should at least call queueInit() and
	* queueCleanup() to add the welcome and cleanup actions to
	* the queue (unless you do that yourself.)
	*
	* For queueInit, @p checkUser causes a CheckUser action to
	*    be queued automatically.
	* For queueConduits, whatever is relevant for the conduits
	*   can be used, usually things in the FlagMask and ActionMask.
	*   The list of conduits in @p conduits is queued automatically.
	* For queueInstaller, the directory @p dir is used as a source
	*   of files to install (checked at exec() time).
	*/

	void queueInit(bool checkUser = false);
	void queueConduits(const QStringList &conduits,SyncAction::SyncMode e, bool local=false);
	void queueInstaller(const QString &dir);
	void queueCleanup();


protected:
	virtual bool exec();

protected slots:
	/**
	* When one action finishes, start the next one.
	*/
	void actionCompleted(SyncAction *);
};

/**
* This very special SyncAction puts "Welcome to KPilot"
* in the sync log of the Pilot.
*/
class WelcomeAction : public SyncAction
{
public:
	WelcomeAction(KPilotDeviceLink *);

protected:
	virtual bool exec();
} ;

/**
* This one just says "sorry, can't sync now". This is used
* in cases when the hotsync starts while KPilot is busy configuring
* something and can't be interrupted.
*/
class SorryAction : public SyncAction
{
public:
	SorryAction(KPilotDeviceLink *, const QString &s=QString::null);

protected:
	virtual bool exec();
	QString fMessage;
} ;

/**
* This conduit isn't really a conduit -- it just makes a quick
* backup copy of all the pdbs in the standard DBBackup/ directory,
* so that in case something goes horribly wrong we still have the
* state from before this sync started, cq. the state after the
* last sync.
*/
class LocalBackupAction : public SyncAction
{
public:
	LocalBackupAction(KPilotDeviceLink *, const QString &);
protected:
	virtual bool exec();
	QString fDir;
} ;

/**
* The ConduitProxy action delays loading the plugin for a conduit until the conduit
* actually executes; the proxy then loads the file, creates a SyncAction for the conduit
* and runs that. Once the conduit has finished, the proxy unloads everything
* and emits syncDone().
*/
class ConduitProxy : public ConduitAction
{
Q_OBJECT

public:
	ConduitProxy(KPilotDeviceLink *,
		const QString &desktopName,
		SyncAction::SyncMode m,
		bool local = false);

	static QStringList flagsForMode(SyncAction::SyncMode m);

protected:
	virtual bool exec();
protected slots:
	void execDone(SyncAction *);

protected:
	QString fDesktopName;
	QString fLibraryName;
	ConduitAction *fConduit;
	SyncAction::SyncMode fMode;
	bool fLocal;
} ;


#endif
