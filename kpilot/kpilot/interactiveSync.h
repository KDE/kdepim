#ifndef _KPILOT_INTERACTIVESYNC_H
#define _KPILOT_INTERACTIVESYNC_H
/* interactiveSync.h                    KPilot
**
** Copyright (C) 2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** This file specializes SyncAction to a kind that can have interaction
** with the user without the Sync timing out.
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
** Bug reports and questions can be sent to kde-pim@kde.org.
*/


class QTimer;

#include "syncAction.h"

class CheckUser : public SyncAction
{
public:
	CheckUser(KPilotDeviceLink *p,QWidget *w=0L);
	virtual ~CheckUser();

protected:
	virtual bool exec();
} ;

class RestoreAction : public SyncAction
{
Q_OBJECT
public:
	RestoreAction(KPilotDeviceLink *,QWidget *w=0L);

	typedef enum { InstallingFiles, GettingFileInfo,Done } Status;
	virtual QString statusString() const;

protected:
	virtual bool exec();

protected slots:
	void getNextFileInfo();
	void installNextFile();

private:
	// Use a private-d pointer for once (well, in KPilot
	// parlance it'd be fd, which is confusing, so it's
	// become a private fP) since we need QList or QPtrList.
	//
	//
	class RestoreActionPrivate;
	RestoreActionPrivate *fP;
} ;

#endif
