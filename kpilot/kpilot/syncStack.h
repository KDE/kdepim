#ifndef _KPILOT_SYNCSTACK_H
#define _KPILOT_SYNCSTACK_H
/* syncStack.h                        KPilot
**
** Copyright (C) 1998-2001,2003 by Dan Pilone
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
* the device's deviceReady() signal with the exec() slot -- or something to that effect.
* The ActionQueue will then run all the actions in the queue in sequence.
*
*/

/*
* The constructor with lots of parameters is DEPRECATED.
*
* The @p config parameter is passed on to conduit actions that may be in the queue.
*
* @p conduits is a list of .desktop filenames (without the extension); for each conduit,
* a ConduitProxy action is created, which loads the conduit and runs the SyncAction
* from that conduit.
*
* For FileInstallers, pass in the list of filenames and the directory in @p installDir
* and @p installFiles.
*
* When you create an ActionQueue, pass in all the necessary conduit- and filenames.
* You cannot change them later. You must also call prepare() to add all the items
* to the queue; ie.
*
* ActionQueue s(p,c,QStringList(),"/tmp","foo.prc");
*
* Creates a queue with some information set, but without any actions in it.
* Next, call prepare() or somesuch:
*
* s.prepareSync();
*
* This will add a buch of "standard" actions to the queue for a HotSync, ie. a
* welcome message action, user check, the conduits (if any; none were specified in the
* constructor above), a file install action (which will try to install /tmp/foo.prc) and
* some cleanup actions.
*
*
* Alternatively, you can use addAction() to fill up the stack yourself.
*/

/**
* The constructor with one parameter is preferred. You can
* call the public member functions to enqueue actions in
* several standard ways.
*/

class ActionQueue : public SyncAction
{
Q_OBJECT
public:
	ActionQueue(KPilotDeviceLink *device);
	/** DEPRECATED **/
	ActionQueue(KPilotDeviceLink *device,
		KConfig *config,
		const QStringList &conduits = QStringList(),
		const QString &installDir = QString::null,
		const QStringList &installFiles = QStringList());
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
	KConfig *fConfig;

	QString fInstallerDir;
	QStringList fInstallerFiles;
	QStringList fConduits;

public:
	enum SyncModes {
		// Exactly one of these four modes must be set
		// (although Test can't be set explicitly).
		//
		Test=0,
		Backup=1,
		Restore=2,
		HotSync=4,         // Normal operation
		                   // 8 still available
		// These are optional (mixins)
		//
		//
		WithBackup=0x10,
		WithUserCheck=0x20,
		WithInstaller=0x40,
		WithConduits=0x80,

		// These are misc. flags you can set
		FlagPCToHH=0x100,
		FlagHHToPC=0x200,
		FlagLocal=0x1000,
		FlagFull=0x2000,
		                   // 8192 still available
		FlagTest=0x4000,

		                   // 32768 still available
		// These are masks you can use to select
		// the bits coding the action, mixins (With*)
		// and misc. flags.
		//
		//
		ActionMask=0xf,
		MixinMask=0xf0,
		FlagMask=0xff00,
		// These are derived values for convenience.
		// Note that a HotSync doesn't install files by default.
		//
		//
		TestMode = Test | WithUserCheck | WithConduits | FlagTest,
		BackupMode = Backup | WithUserCheck | WithConduits | WithBackup,
		RestoreMode = Restore | WithUserCheck,
		HotSyncMode = HotSync | WithUserCheck | WithConduits
		} ;

	/**
	* Call prepare() to push a "standard profile" of SyncActions onto the stack,
	* ready for execution. These are welcome, cleanup, and actions indicated
	* by @ref m. @p m is a bitwise or of items from SyncModes --- Exactly
	* one of Test, Backup, Restore or HotSync, for the main intent of the stack,
	* and possibly one or more of the With* elements, which insert extra
	* actions at the relevant moment in the execution of the stack.
	*/

	/* DEPRECATED */
	void prepare(int m);
	void prepareBackup() { prepare(BackupMode); } ;
	void prepareRestore() { prepare(RestoreMode); } ;
	void prepareSync() { prepare(HotSyncMode); } ;

	/**
	* Call these queue*() functions to append
	* standard functional blocks. They're pretty
	* much mutually exclusive with the prepare*()
	* functions above.
	*
	* You should at least call queueInit() and
	* queueCleanup() to add the welcome and cleanup
	* actions to the queue (unless you do that
	* yourself.)
	*
	* For queueInit, relevant modes are WithUserCheck.
	* For queueConduits, whatever is relevant for the conduits
	*   can be used, usually things in the FlagMask and ActionMask.
	*/

	void queueInit(int mode=WithUserCheck);
	void queueConduits(KConfig *,const QStringList &conduits,int mode=0);
	void queueInstaller(const QString &dir,const QStringList &files);
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
Q_OBJECT

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
Q_OBJECT

public:
	SorryAction(KPilotDeviceLink *);

protected:
	virtual bool exec();
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
		int m);

protected:
	virtual bool exec();
protected slots:
	void execDone(SyncAction *);

protected:
	QString fDesktopName;
	QString fLibraryName;
	ConduitAction *fConduit;
	int fMode;
} ;


#endif
