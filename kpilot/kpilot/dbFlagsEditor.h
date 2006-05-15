#ifndef _KPILOT_DBFLAGSEDITOR_H
#define _KPILOT_DBFLAGSEDITOR_H
/* dbFlagsEditor.h                 KPilot
**
** Copyright (C) 2003 Reinhold Kainhofer <reinhold@kainhofer.com>
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"
#include <kdialogbase.h>

class DBFlagsEditorWidget;
struct DBInfo;
/**
@author Reinhold Kainhofer
*/
class DBFlagsEditor : public KDialogBase
{
Q_OBJECT
public:
	DBFlagsEditor(DBInfo*dbinfo=0L, QWidget *parent = 0);
	~DBFlagsEditor();

protected:
	void fillWidgets();
	DBInfo*dbi;
	DBFlagsEditorWidget*widget;
protected slots:
	virtual void slotOk();
	virtual void slotCancel();


};

#endif
