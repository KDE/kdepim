#ifndef _NULL_NULL_CONDUIT_H
#define _NULL_NULL_CONDUIT_H
/* null-conduit.h			KPilot
**
** Copyright (C) 2000-2001 by Adriaan de Groot
**
** This file is part of the NULL conduit, a conduit for KPilot that
** does nothing except add a log message to the Pilot's HotSync log.
** It is also intended as a programming example.
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

#include "plugin.h"

class PilotRecord;
class PilotDatabase;

/**
 * The conduit Null does nothing. Almost nothing, anyway.
 * It writes a single log message to the sync log and then
 * completes successfully. For debugging purposes it can
 * also simulate failure, but that is a very specialized
 * case available only programmatically.
 */
class NullConduit : public ConduitAction
{
public:
	/** Constructor. Special case is if @p contains
	 *  @c --fail as an argument to the conduit, then
	 *  the conduit will fail instead of trivially succeeding.
	 */
	NullConduit(KPilotLink *,
		const char *name=0L,
		const QStringList &args = QStringList());
	virtual ~NullConduit();

protected:
	virtual bool exec();

protected:
	PilotDatabase *fDatabase;
	bool fFailImmediately;
};

#endif
