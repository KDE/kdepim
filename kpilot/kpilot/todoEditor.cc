// -*- C++ -*-
/* todoEditor.cc		KPilot
**
** Copyright (C) 2000 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** This is a dialog window that edits one single todo record.
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

#include <qcombobox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qtextedit.h>
#include <qcheckbox.h>

#include <kdatewidget.h>

#include "pilotTodoEntry.h"
#include "todoEditor_base.h"
#include "todoEditor.moc"

static const char *todoEditor_id =
	"$Id$";

TodoEditor::TodoEditor(PilotTodoEntry * p, struct ToDoAppInfo *appInfo,
	QWidget * parent, const char *name) :
	KDialogBase(parent, name, false, i18n("To-do Editor"), Ok|Cancel),
	fDeleteOnCancel(p == 0L),
	fTodo(p),
	fAppInfo(appInfo)
{
	FUNCTIONSETUP;

	fWidget=new TodoEditorBase(this);
	setMainWidget(fWidget);
	fillFields();

	connect(parent, SIGNAL(recordChanged(PilotTodoEntry *)),
		this, SLOT(updateRecord(PilotTodoEntry *)));

	(void) todoEditor_id;
}

TodoEditor::~TodoEditor()
{
	FUNCTIONSETUP;

	if (fDeleteOnCancel && fTodo)
	{
#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": Deleting private todo record." << endl;
#endif
		delete fTodo;
		fTodo = 0L;
	}

#ifdef DEBUG
	DEBUGKPILOT << fname << ": Help! I'm deleting!" << endl;
#endif
}



void TodoEditor::fillFields()
{
	FUNCTIONSETUP;

	if (fTodo == 0L)
	{
		fTodo = new PilotTodoEntry(*fAppInfo);
		fDeleteOnCancel = true;
	}

	fWidget->fDescription->setText(fTodo->getDescription());
	fWidget->fCompleted->setChecked(fTodo->getComplete());
	if (fTodo->getIndefinite())
	{
		fWidget->fHasEndDate->setChecked(false);
	}
	else
	{
		fWidget->fHasEndDate->setChecked(true);
		fWidget->fEndDate->setDate(readTm(fTodo->getDueDate()).date());
	}
	fWidget->fPriority->setCurrentItem(fTodo->getPriority());
//	fCategory->setCurrentItem(fTodo->getCategory()));
	fWidget->fNote->setText(fTodo->getNote());
}



/* slot */ void TodoEditor::slotCancel()
{
	FUNCTIONSETUP;

	if (fDeleteOnCancel && fTodo)
	{
		delete fTodo;

		fTodo = 0L;
	}
	KDialogBase::slotCancel();
}

/* slot */ void TodoEditor::slotOk()
{
	FUNCTIONSETUP;

	// Commit changes here
	fTodo->setDescription(fWidget->fDescription->text());
	fTodo->setComplete(fWidget->fCompleted->isChecked());
	if (fWidget->fHasEndDate->isChecked())
	{
		fTodo->setIndefinite(false);
		struct tm duedate=writeTm(fWidget->fEndDate->date());
		fTodo->setDueDate(duedate);
	}
	else
	{
		fTodo->setIndefinite(true);
	}
	fTodo->setPriority(fWidget->fPriority->currentItem());
//	fTodo->setCategory(fWidget->fCategory->currentItem());
	fTodo->setNote(fWidget->fNote->text());

	emit(recordChangeComplete(fTodo));
	KDialogBase::slotOk();
}

/* slot */ void TodoEditor::updateRecord(PilotTodoEntry * p)
{
	FUNCTIONSETUP;
	if (p != fTodo)
	{
		// Not meant for me
		//
		//
		return;
	}

	if (p->isDeleted())
	{
		delayedDestruct();
		return;
	}
	else
	{
		fillFields();
	}
}

