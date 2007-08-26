#ifndef _KPILOT_ACTIONQUEUE_H
#define _KPILOT_ACTIONQUEUE_H
/*
**
** Copyright (C) 1998-2001,2003 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
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

#include <qptrqueue.h>

#include "syncAction.h"

/** @file
* This file defines the ActionQueue.
*
* This used to be called SyncStack, and while a stack is cool
* for some things, it actually rather confuses the issue because
* you _use_ this class for specifying "do this, then that, then ..."
* and in program code you need to reverse that order when adding
* items to a stack. So now it's a Queue, FIFO, and program
* code looks more normal.
*/

/**
* The ActionQueue is a meta-action, which handles running a bunch of
* SyncActions in sequence. It is a SyncAction itself, so it can even
* be queued on another ActionQueue.
*
* An ActionQueue is constructed with a @p device. As usual, you should
* connect the device's deviceReady() signal with the exec() slot --
* or something to that effect. The ActionQueue will then run all the
* actions in the queue in sequence.
*
*/
KDE_EXPORT class ActionQueue : public SyncAction
{
Q_OBJECT
public:
	/**
	* Constructor. Pass in a KPilot device link for it to act on.
	* It is legal to pass in 0 (NULL) as a device. Ownership of
	* the device is unchanged.
	*/
	ActionQueue(KPilotLink *device);

	/** Destructor. */
	virtual ~ActionQueue();

	/** Is the queue empty? Returns @c true if it is. */
	bool isEmpty() const
	{
		return SyncActionQueue.isEmpty();
	};

	/**
	* You can push your own action @p a onto the queue. Ownership
	* of the action is given to the ActionQueue object.
	*/
	void addAction(SyncAction *a)
	{
		SyncActionQueue.enqueue(a);
	};

public:
	/*
	* Call these queue*() functions to append standard functional
	* blocks. You should at least call queueInit() and
	* queueCleanup() to add the welcome and cleanup actions to
	* the queue (unless you do that yourself.)
	*
	* For queueInit, a WelcomeAction is added.
	* For queueConduits, whatever is relevant for the conduits
	*   can be used, usually things in the FlagMask and ActionMask.
	*   The list of conduits in @p conduits is queued automatically.
	*/

	/**
	* Initialize the queue. This empties it out and adds a
	* welcome action (see WelcomeAction in actions.h) so that
	* the user knows what is happening when the ActionQueue
	* begins to execute. Equivalent to
	* @code
	* clear(); addAction(new WelcomeAction);
	* @endcode
	*/
	void queueInit();

	/**
	* Queue a (series) of conduits @p conduits with a given
	* sync mode @p mode. Each of the conduits named is called
	* through a ConduitProxy object which handles loading the
	* conduit's shared library and creating the actual SyncAction
	* for that conduit. Actions named "internal_*" are silently
	* ignored since those names are used by KPilot internally
	* for administrative purposes.
	*/
	void queueConduits(const QStringList &conduits,
		const SyncAction::SyncMode &mode);

	/**
	* Convenience function for adding a cleanup action (see
	* CleanupAction in actions.h) to the queue. Should be the
	* last action added to the queue because a HotSync can only
	* have @em one cleanup.
	*/
	void queueCleanup();

protected:
	/**
	* Remove all the actions from the queue and delete them
	* (the queue owns the actions, after all).
	*/
	void clear();

	/**
	* Dequeue the next action in the queue, ready for processing.
	* This takes the action off the queue, so remember to delete it
	* eventually.
	*/
	SyncAction *nextAction()
	{
		return SyncActionQueue.dequeue();
	};

	/** Reimplemented from SyncAction. */
	virtual bool exec();

protected slots:
	/**
	* When one action finishes, start the next one.
	*/
	void actionCompleted(SyncAction *);

private:
	/** A queue of actions to take. */
	QPtrQueue < SyncAction > SyncActionQueue;
};


#endif
