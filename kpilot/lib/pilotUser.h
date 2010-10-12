#ifndef _KPILOT_PILOTUSER_H
#define _KPILOT_PILOTUSER_H
/* pilotUser.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** Wrapper for the PilotUser struct from pilot-link, which describes
** the user-data set in the Pilot.
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

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <pi-dlp.h>

#include "pilot.h"

class KPilotUser
{
public:
	/** Constructor. Create an empty PilotUser structure. */
	KPilotUser()
	{
		::memset(&fUser,0,sizeof(struct PilotUser));
	}
	/** Constructor. Use the given PilotUser structure.
	*  This creates a copy; no ownership is transferred.
	*/
	KPilotUser(const PilotUser *user)
	{
		fUser = *user;
	}

	/** Accessor for the whole PilotUser structure. */
	PilotUser *data()
	{
		return &fUser;
	}

	/** @return The username set on the handheld. */
	QString name() const
	{
		return Pilot::fromPilot( fUser.username );
	}
	/** Set the user name to the given @p name , truncated
	*  if necessary to the size of the field on the handheld.
	*/
	void setName( const QString &name )
	{
		Pilot::toPilot( name, fUser.username, sizeof(fUser.username) );
	}

	/** @return The length of the password on the handheld,
	*           in bytes.
	*/
	const int passwordLength() const
	{
		return fUser.passwordLength;
	}

	/** @return the ID (4 bytes) of the last PC to sync this handheld.
	 *          This is intended to help identify when the use has
	 *          changed PCs and needs a new full sync.
	 */
	unsigned long getLastSyncPC() const
	{
		return fUser.lastSyncPC;
	}
	/** Set the ID of the PC syncing the handheld to @p pc . This
	 *  should be unique in some way (perhaps IP addresses can be
	 *  used this way, or hostnames).
	 */
	void setLastSyncPC(unsigned long pc)
	{
		fUser.lastSyncPC = pc;
	}

	/** @return the timestamp that the handheld was last synced
	 *  successfully.
	 */
	time_t getLastSuccessfulSyncDate()
	{
		return fUser.successfulSyncDate;
	}
	/** Set the timestamp for a successful sync. */
	void setLastSuccessfulSyncDate(time_t when)
	{
		fUser.successfulSyncDate = when;
	}

	/** @return the timestamp of the last sync attempt. */
	time_t getLastSyncDate()
	{
		return fUser.lastSyncDate;
	}
	/** Set the timestamp of the sync attempt. */
	void setLastSyncDate(time_t when)
	{
		fUser.lastSyncDate = when;
	}

private:
	struct PilotUser fUser;
};

#endif
