// -*- C++ -*-
/* todoEditor.h		KPilot
**
** Copyright (C) 1998-2000 by Dan Pilone
**
** This is a dialog window that is used to edit a single todo record.
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

#ifndef _KPILOT_TODOEDITOR_H
#define _KPILOT_TODOEDITOR_H

#include <kdialogbase.h>

class PilotTodoEntry;
struct ToDoAppInfo;

class QComboBox;
class QTextEdit;
class QCheckBox;
class KDateWidget;
class TodoEditorBase;

class TodoEditor : public KDialogBase
{
	Q_OBJECT


public:
	TodoEditor(PilotTodoEntry *todo,
		struct ToDoAppInfo *appInfo,
		QWidget *parent, const char *name=0L);
	~TodoEditor();


signals:
	void recordChangeComplete ( PilotTodoEntry* );

public slots:
	void slotOk();
	void slotCancel();
	void updateRecord(PilotTodoEntry *);

private:
	TodoEditorBase*fWidget;
	bool fDeleteOnCancel;

	PilotTodoEntry* fTodo;
	struct ToDoAppInfo *fAppInfo;

	void fillFields();
};
#endif

