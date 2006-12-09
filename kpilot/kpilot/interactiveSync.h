#ifndef _KPILOT_INTERACTIVESYNC_H
#define _KPILOT_INTERACTIVESYNC_H
/* interactiveSync.h                    KPilot
**
** Copyright (C) 2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
** Copyright (C) 2006 Adriaan de Groot <groot@kde.org>
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org.
*/


class QTimer;

#include "syncAction.h"

class CheckUser : public SyncAction
{
public:
	CheckUser(KPilotLink *p,QWidget *w=0L);
	virtual ~CheckUser();

protected:
	virtual bool exec();
} ;

class RestoreAction : public SyncAction
{
Q_OBJECT
public:
	RestoreAction(KPilotLink *,QWidget *w=0L);

	typedef enum { InstallingFiles, GettingFileInfo,Done } Status;
	virtual QString statusString() const;

	/** By default, a path based on the user name (either
	*   on the handheld or set in KPilot) is used to
	*   determine the restory directory name ( generally
	*   $KDEHOME/share/apps/kpilot/DBBackup/_user_name_ ).
	*   Use setDirectory() to change that and use a given
	*   @p path as target for the source. Use an empty
	*   @p path to restore the default behavior of using
	*   the username.
	*/
	void setDirectory( const QString &path );

protected:
	virtual bool exec();

protected slots:
	void installNextFile();

private:
	class Private;
	Private *fP;
} ;

#endif
