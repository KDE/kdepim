#ifndef _KPILOT_MultiDB_SETUP_H
#define _KPILOT_MultiDB_SETUP_H
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
#include "MultiDB-conduitDialog.h"
#include "MultiDB-factory.h"
#include "MultiDBWidgetPrivate.h"

class MultiDBWidgetSetup : public ConduitConfig {
Q_OBJECT
public:
	MultiDBWidgetSetup(QWidget *,const char *,const QStringList &, SyncTypeList_t *lst=0L, KAboutData *abt=NULL);
	virtual ~MultiDBWidgetSetup();

protected:
	virtual void readSettings();
	virtual void commitChanges();
	virtual const QString getSyncTypeEntry() { return "SyncAction"; };
	virtual const QString getSyncFileEntry() { return "SyncFile"; };
	virtual const QString settingsFileList() { return "Databases"; };
	virtual const QString getSettingsDefaultAct() { return "DefaultAction";};
	virtual const QString getSettingsGroup()=0;
	virtual QString ActIdToName(int act);
	virtual int ActNameToId(QString act);

protected slots:
	virtual void insert_db();
	virtual void edit_db();
	virtual void delete_db();
	virtual void slotOk();
	virtual void slotApply();
	virtual void edit(QListViewItem*listitem);

protected:
	SyncTypeList_t *synctypes;
private:
	MultiDBWidgetPrivate*fConfigWidget;
} ;


#endif
