#ifndef _KPILOT_DatabaseAction_SETUP_H
#define _KPILOT_DatabaseAction_SETUP_H
/* MultiDB-setup.h                         KPilot
**
** Copyright (C) 2002 by Reinhold Kainhofer
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the class for the behavior of the setup dialog.
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
#include "plugin.h"
#include "MultiDB-conduit.h"
#include "MultiDB-setup.h"
#include "MultiDB-factory.h"
#include "DatabaseActiondlgPrivate.h"

class DBSettings : public KDialogBase {
Q_OBJECT
public:
	DBSettings(QWidget *, const char *, DBSyncInfo*itm, SyncTypeList_t *tps, bool changeDBName=true, bool allowask=true);
	virtual ~DBSettings();

protected:
	virtual void commitChanges();
	virtual int IdToSyncType(int tp);
	virtual int SyncTypeToId(int tt);

protected slots:
	void slotOk();
	void slotApply();
	void slotBrowseFile();
private:
	DatabaseActionDlgPrivate *fConfigWidget;
	DBSyncInfo*item;
	QWidget *fMainWidget;
	SyncTypeList_t *synctypes;
protected:
	QWidget* widget() {return fMainWidget;}
} ;


#endif
