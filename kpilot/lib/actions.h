#ifndef _KPILOT_ACTIONS_H
#define _KPILOT_ACTIONS_H
/*
**
** Copyright (C) 1998-2001,2003 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
** Copyright (C) 2006 Adriaan de Groot <groot@kde.org>
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

#include "syncAction.h"

/** @file
* This file defines some simple standard actions. None of these
* actions require much in the way of configuration; none provide
* particularly complicated functionality.
*/

/**
* This action puts "Welcome to KPilot" in the sync log of the handheld.
* It is added automatically to a ActionQueue by queueInit() in order
* to inform the user of the sync.
*/
KDE_EXPORT class WelcomeAction : public SyncAction
{
public:
	/** Constructor. */
	WelcomeAction(KPilotLink *);

protected:
	/** Reimplemented from SyncAction. */
	virtual bool exec();
} ;

/**
* This one just says "sorry, can't sync now". This is used
* in cases when the hotsync starts while KPilot is busy configuring
* something and can't be interrupted.
*/
KDE_EXPORT class SorryAction : public SyncAction
{
public:
	/**
	* Constructor. The action will be executed on the given
	* link @p device . If the given string @p s is non-empty,
	* print that message (it must be i18n()ed already) instead of
	* the standard message.
	*/
	SorryAction(KPilotLink *device, const QString &s=QString::null);

protected:
	/** Reimplemented from SyncAction. */
	virtual bool exec();

	/** Message to print to the sync log. */
	QString fMessage;
} ;

/**
* End the HotSync. This action cleans up the handheld and
* removes cruft. There should be exactly @em one CleanupAction
* executed during a HotSync. Since this action informs the
* device that the HotSync is over, it should be the last
* action executed.
*/
KDE_EXPORT class CleanupAction : public SyncAction
{
public:
	/** Constructor. */
	CleanupAction(KPilotLink *device);

protected:
	/** Reimplemented from SyncAction. */
	virtual bool exec();
} ;

/**
* This action is intended to test the link with the handheld
* and not do anything spectacular. It lists all the databases
* on the handheld in the sync log.
*/
KDE_EXPORT class TestLink : public SyncAction
{
public:
	/** Constructor. */
	TestLink(KPilotLink *device);

protected:
	/** Reimplemented from SyncAction. */
	virtual bool exec();
} ;


#endif
