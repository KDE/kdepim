// -*- C++ -*-
/* todoEditor.cc		KPilot
**
** Copyright (C) 2000 by Dan Pilone
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
#include "todoEditor.moc"

static const char *todoEditor_id =
	"$Id$";

TodoEditor::TodoEditor(PilotTodoEntry * p, struct ToDoAppInfo *appInfo,
	QWidget * parent, const char *name) :
	KDialogBase(KDialogBase::Plain, i18n("Todo Editor"),
		Ok | Cancel, Cancel, parent, name, false /* non-modal */ ),
	fDeleteOnCancel(p == 0L),
	fTodo(p),
	fAppInfo(appInfo)
{
	FUNCTIONSETUP;

	initLayout();
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

	fDescription->setText(fTodo->getDescription());
	fCompleted->setChecked(fTodo->getComplete());
	fHasEndDate->setChecked(!fTodo->getIndefinite());
	fEndDate->setDate(readTm(fTodo->getDueDate()).date());
	fPriority->setCurrentItem(fTodo->getPriority());
//	fCategory->setCurrentItem(fTodo->getCategory()));
	fNote->setText(fTodo->getNote());
}



void TodoEditor::initLayout()
{
	FUNCTIONSETUP;

	QFrame *p = plainPage();
	QGridLayout *grid = new QGridLayout(p, 1, 1, 0, SPACING);

	QLabel*fDescriptionLabel = new QLabel(i18n("&Description:"), p, "fDescriptionLabel" );
	fDescriptionLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)4, (QSizePolicy::SizeType)5, 0, 0, fDescriptionLabel->sizePolicy().hasHeightForWidth() ) );
	fDescriptionLabel->setAlignment( int( QLabel::AlignTop ) );
	grid->addWidget( fDescriptionLabel, 0, 0 );

	fDescription = new QTextEdit( p, "fDescription" );
	fDescription->setMaximumSize( QSize( 32767, 80 ) );
	grid->addMultiCellWidget( fDescription, 0, 0, 1, 3 );
	fDescriptionLabel->setBuddy( fDescription );


	fCompleted = new QCheckBox(i18n("&Completed"), p, "fCompleted" );
	grid->addMultiCellWidget(fCompleted, 1, 1, 0, 1 );

	fHasEndDate = new QCheckBox(i18n("Has &end date:"), p, "fHasEndDate" );
	grid->addMultiCellWidget( fHasEndDate, 2, 2, 0, 1 );

	fEndDate = new KDateWidget(QDate::currentDate(), "fEndDate" );
	fEndDate->setEnabled( FALSE );
	grid->addMultiCellWidget( fEndDate, 3, 3, 0, 1 );


	QLabel*fPriorityLabel = new QLabel( i18n("&Priority:"), p, "fPriorityLabel" );
	grid->addWidget( fPriorityLabel, 1, 2 );
	fPriority = new QComboBox( FALSE, p, "fPriority" );
	grid->addWidget( fPriority, 1, 3 );
	fPriorityLabel->setBuddy( fPriority );
	fPriority->insertItem( tr("1") );
	fPriority->insertItem( tr("2") );
	fPriority->insertItem( tr("3") );
	fPriority->insertItem( tr("4") );
	fPriority->insertItem( tr("5") );


/*	fCategoryLabel = new QLabel( i18n("Ca&tegory:"), p, "fCategoryLabel" );
	grid->addWidget( fCategoryLabel, 2, 2 );
	fCategory = new QComboBox( FALSE, p, "fCategory" );
	grid->addWidget( fCategory, 2, 3 );
	fCategoryLabel->setBuddy( fCategory );
	// TODO: Fill the list of categories
	// TODO: Add possibility to edit categories
*/


	QLabel*fNoteLabel = new QLabel(i18n( "&Note:" ), p, "fNoteLabel" );
	fNoteLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)4, (QSizePolicy::SizeType)5, 0, 0, fNoteLabel->sizePolicy().hasHeightForWidth() ) );
	fNoteLabel->setAlignment( int( QLabel::AlignTop ) );
	grid->addWidget( fNoteLabel, 4, 0 );

	fNote = new QTextEdit(p, "fNote" );
	grid->addMultiCellWidget( fNote, 4, 4, 1, 3 );
	fNoteLabel->setBuddy( fNote );

	connect( fHasEndDate, SIGNAL( toggled(bool) ), fEndDate, SLOT( setEnabled(bool) ) );
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
	fTodo->setDescription(fDescription->text());
	fTodo->setComplete(fCompleted->isChecked());
	if (fHasEndDate->isChecked())
	{
		fTodo->setIndefinite(false);
		struct tm duedate=writeTm(fEndDate->date());
		fTodo->setDueDate(duedate);
	}
	else
	{
		fTodo->setIndefinite(true);
	}
	fTodo->setPriority(fPriority->currentItem());
//	fTodo->setCategory(fCategory->currentItem());
	fTodo->setNote(fNote->text());

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

