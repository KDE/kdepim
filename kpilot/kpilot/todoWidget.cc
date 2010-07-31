/* KPilot
**
** Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** This file defines the todoWidget, that part of KPilot that
** displays todo records from the Pilot.
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
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <tqptrlist.h>
#include <klistview.h>
#include <tqpushbutton.h>
#include <tqlayout.h>
#include <tqlabel.h>
#include <tqtextview.h>
#include <tqcombobox.h>
#include <tqwhatsthis.h>
#include <tqtextcodec.h>

#include <kmessagebox.h>

#include "kpilotConfig.h"
#include "todoEditor.h"
#include "pilotLocalDatabase.h"
#include "todoWidget.moc"




TodoCheckListItem::TodoCheckListItem(TQListView*parent, const TQString&text,
	recordid_t pilotid, void*r):PilotCheckListItem(parent, text, pilotid, r)
{

}

void TodoCheckListItem::stateChange(bool state)
{
	TodoListView*par=dynamic_cast<TodoListView*>(listView());
	if (par) par->itemWasChecked(this, state);
}



TodoWidget::TodoWidget(TQWidget * parent,
	const TQString & path) :
	PilotComponent(parent, "component_todo", path),
	fTodoInfo(0L),
	fTodoAppInfo(0L),
	fTodoDB(0L),
	fPendingTodos(0)
{
	FUNCTIONSETUP;

	setupWidget();
	fTodoList.setAutoDelete(true);

}

TodoWidget::~TodoWidget()
{
	FUNCTIONSETUP;
	KPILOT_DELETE( fTodoDB );
}

int TodoWidget::getAllTodos(PilotDatabase * todoDB)
{
	FUNCTIONSETUP;

	int currentRecord = 0;
	PilotRecord *pilotRec;
	PilotTodoEntry *todo;

#ifdef DEBUG
	DEBUGKPILOT << fname << ": Reading ToDoDB..." << endl;
#endif

	while ((pilotRec = todoDB->readRecordByIndex(currentRecord)) != 0L)
	{
		if (!(pilotRec->isDeleted()) &&
			(!(pilotRec->isSecret()) || KPilotSettings::showSecrets()))
		{
			todo = new PilotTodoEntry(pilotRec);
			if (todo == 0L)
			{
				WARNINGKPILOT << "Couldn't allocate record "
					<< currentRecord++
					<< endl;
				break;
			}
			fTodoList.append(todo);
		}
		KPILOT_DELETE( pilotRec );

		currentRecord++;
	}

#ifdef DEBUG
	DEBUGKPILOT << fname
		<< ": Total " << currentRecord << " records" << endl;
#endif

	return currentRecord;
}

void TodoWidget::showComponent()
{
	FUNCTIONSETUP;
	if ( fPendingTodos>0 ) return;

#ifdef DEBUG
	DEBUGKPILOT << fname
		<< ": Reading from directory " << dbPath() << endl;
#endif

	fTodoDB = new PilotLocalDatabase(dbPath(), CSL1("ToDoDB"));

	fTodoList.clear();

	if (fTodoDB->isOpen())
	{
		KPILOT_DELETE(fTodoAppInfo);
		fTodoAppInfo = new PilotToDoInfo(fTodoDB);
		populateCategories(fCatList, fTodoAppInfo->categoryInfo());
		getAllTodos(fTodoDB);

	}
	else
	{
		populateCategories(fCatList, 0L);
		WARNINGKPILOT << "Could not open local TodoDB" << endl;
	}

	KPILOT_DELETE( fTodoDB );

	updateWidget();
}

/* virtual */ bool TodoWidget::preHotSync(TQString &s)
{
	FUNCTIONSETUP;

	if (fPendingTodos)
	{
#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": fPendingTodo="
			<< fPendingTodos
			<< endl;
#endif

#if KDE_VERSION<220
		s = i18n("There are still %1 to-do editing windows open.")
			.arg(TQString::number(fPendingTodos));
#else
		s = i18n("There is still a to-do editing window open.",
			"There are still %n to-do editing windows open.",
			fPendingTodos);
#endif
		return false;
	}

	return true;
}

void TodoWidget::postHotSync()
{
	FUNCTIONSETUP;

	fTodoList.clear();
	showComponent();
}

void TodoWidget::hideComponent()
{
	FUNCTIONSETUP;
	if ( fPendingTodos==0 )
	{
		fTodoList.clear();
		fListBox->clear();
		KPILOT_DELETE( fTodoDB );
	}
}

void TodoWidget::setupWidget()
{
	FUNCTIONSETUP;

	TQLabel *label;
	TQGridLayout *grid = new TQGridLayout(this, 6, 4, SPACING);

	fCatList = new TQComboBox(this);
	grid->addWidget(fCatList, 0, 1);
	connect(fCatList, TQT_SIGNAL(activated(int)),
		this, TQT_SLOT(slotSetCategory(int)));
	TQWhatsThis::add(fCatList,
		i18n("<qt>Select the category of to-dos to display here.</qt>"));

	label = new TQLabel(i18n("Category:"), this);
	label->setBuddy(fCatList);
	grid->addWidget(label, 0, 0);

	fListBox = new TodoListView(this);
	fListBox->addColumn( i18n( "To-do Item" ) );
	fListBox->setAllColumnsShowFocus( TRUE );
	fListBox->setResizeMode( KListView::LastColumn );
	fListBox->setFullWidth( TRUE );
	fListBox->setItemsMovable( FALSE );
	fListBox->setItemsRenameable (TRUE);
	grid->addMultiCellWidget(fListBox, 1, 1, 0, 1);
	connect(fListBox, TQT_SIGNAL(selectionChanged(TQListViewItem*)),
		this, TQT_SLOT(slotShowTodo(TQListViewItem*)));
	connect(fListBox, TQT_SIGNAL(doubleClicked(TQListViewItem*)),
		this, TQT_SLOT(slotEditRecord(TQListViewItem*)));
	connect(fListBox, TQT_SIGNAL(returnPressed(TQListViewItem*)),
		this, TQT_SLOT(slotEditRecord(TQListViewItem*)));
	connect(fListBox, TQT_SIGNAL(itemChecked(TQCheckListItem*, bool)),
		this, TQT_SLOT(slotItemChecked(TQCheckListItem*, bool)));
	connect(fListBox, TQT_SIGNAL(itemRenamed(TQListViewItem*, const TQString &, int)),
		this, TQT_SLOT(slotItemRenamed(TQListViewItem*, const TQString &, int)));
	TQWhatsThis::add(fListBox,
		i18n("<qt>This list displays all the to-dos "
			"in the selected category. Click on "
			"one to display it to the right.</qt>"));

	label = new TQLabel(i18n("To-do info:"), this);
	grid->addWidget(label, 0, 2);

	// todo info text view
	fTodoInfo = new TQTextView(this);
	grid->addMultiCellWidget(fTodoInfo, 1, 4, 2, 2);

	TQPushButton *button;
	TQString wt;

	fEditButton = new TQPushButton(i18n("Edit Record..."), this);
	grid->addWidget(fEditButton, 2, 0);
	connect(fEditButton, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotEditRecord()));

	wt = KPilotSettings::internalEditors() ?
		i18n("<qt>You can edit a to-do when it is selected.</qt>") :
		i18n("<qt><i>Editing is disabled by the 'internal editors' setting.</i></qt>");
	TQWhatsThis::add(fEditButton,wt);

	button = new TQPushButton(i18n("New Record..."), this);
	grid->addWidget(button, 2, 1);
	connect(button, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotCreateNewRecord()));
	wt = KPilotSettings::internalEditors() ?
		i18n("<qt>Add a new to-do to the to-do list.</qt>") :
		i18n("<qt><i>Adding new to-dos is disabled by the 'internal editors' setting.</i></qt>");
	TQWhatsThis::add(button, wt);
	button->setEnabled(KPilotSettings::internalEditors());

	fDeleteButton = new TQPushButton(i18n("Delete Record"), this);
	grid->addWidget(fDeleteButton, 3, 0);
	connect(fDeleteButton, TQT_SIGNAL(clicked()),
		this, TQT_SLOT(slotDeleteRecord()));
	wt = KPilotSettings::internalEditors() ?
		i18n("<qt>Delete the selected to-do from the to-do list.</qt>") :
		i18n("<qt><i>Deleting is disabled by the 'internal editors' setting.</i></qt>") ;
	TQWhatsThis::add(fDeleteButton,wt);
}

void TodoWidget::updateWidget()
{
	FUNCTIONSETUP;
	if (!shown || !fTodoAppInfo ) return;

	int listIndex = 0;

	int currentCatID = findSelectedCategory(fCatList,
		fTodoAppInfo->categoryInfo());

	fListBox->clear();
	fTodoList.first();

#ifdef DEBUG
	DEBUGKPILOT << fname << ": Adding records..." << endl;
#endif

	PilotTodoEntry*todo;
	while (fTodoList.current())
	{
		todo=fTodoList.current();
		if ((currentCatID == -1) ||
			(todo->category() == currentCatID))
		{
			TQString title = todo->getDescription();

			TodoCheckListItem*item=new TodoCheckListItem(fListBox, title,
				listIndex, todo);
			item->setOn(todo->getComplete());
		}
		listIndex++;
		fTodoList.next();
	}

#ifdef DEBUG
	DEBUGKPILOT << fname << ": " << listIndex << " records" << endl;
#endif

	slotUpdateButtons();
}



/* slot */ void TodoWidget::slotUpdateButtons()
{
	FUNCTIONSETUP;

	bool enabled = (fListBox->currentItem() != 0L);

	enabled &= KPilotSettings::internalEditors() ;

	fEditButton->setEnabled(enabled);
	fDeleteButton->setEnabled(enabled);
}

void TodoWidget::slotSetCategory(int)
{
	FUNCTIONSETUP;

	updateWidget();
}

void TodoWidget::slotEditRecord()
{
	slotEditRecord(fListBox->currentItem());
}
void TodoWidget::slotEditRecord(TQListViewItem*item)
{
	FUNCTIONSETUP;
	if (!shown) return;

	TodoCheckListItem*p = static_cast<TodoCheckListItem*>(item);
	if (!p) return;
	PilotTodoEntry *selectedRecord = (PilotTodoEntry *) p->rec();

	if (selectedRecord->id() == 0)
	{
		KMessageBox::error(0L,
			i18n("Cannot edit new records until "
				"HotSynced with Pilot."),
			i18n("HotSync Required"));
		return;
	}

	TodoEditor *editor = new TodoEditor(selectedRecord,
		fTodoAppInfo->info(), this);

	connect(editor, TQT_SIGNAL(recordChangeComplete(PilotTodoEntry *)),
		this, TQT_SLOT(slotUpdateRecord(PilotTodoEntry *)));
	connect(editor, TQT_SIGNAL(cancelClicked()),
		this, TQT_SLOT(slotEditCancelled()));
	editor->show();

	fPendingTodos++;
}

void TodoWidget::slotCreateNewRecord()
{
	FUNCTIONSETUP;
	if (!shown) return;

	// Response to bug 18072: Don't even try to
	// add records to an empty or unopened database,
	// since we don't have the DBInfo stuff to deal with it.
	//
	//
	PilotDatabase *myDB = new PilotLocalDatabase(dbPath(), CSL1("ToDoDB"));

	if (!myDB || !myDB->isOpen())
	{
#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": Tried to open "
			<< dbPath()
			<< "/ToDoDB"
			<< " and got pointer @"
			<< (void *) myDB
			<< " with status "
			<< ( myDB ? myDB->isOpen() : false )
			<< endl;
#endif

		KMessageBox::sorry(this,
			i18n("You cannot add to-dos to the to-do list "
				"until you have done a HotSync at least once "
				"to retrieve the database layout from your Pilot."),
			i18n("Cannot Add New To-do"));

		if (myDB)
			KPILOT_DELETE( myDB );

		return;
	}

	TodoEditor *editor = new TodoEditor(0L,
		fTodoAppInfo->info(), this);

	connect(editor, TQT_SIGNAL(recordChangeComplete(PilotTodoEntry *)),
		this, TQT_SLOT(slotAddRecord(PilotTodoEntry *)));
	connect(editor, TQT_SIGNAL(cancelClicked()),
		this, TQT_SLOT(slotEditCancelled()));
	editor->show();

	fPendingTodos++;
}

void TodoWidget::slotAddRecord(PilotTodoEntry * todo)
{
	FUNCTIONSETUP;
	if ( !shown && fPendingTodos==0 ) return;

	int currentCatID = findSelectedCategory(fCatList,
		fTodoAppInfo->categoryInfo(), true);


	todo->PilotRecordBase::setCategory(currentCatID);
	fTodoList.append(todo);
	writeTodo(todo);
	// TODO: Just add the new record to the lists
	updateWidget();

	// k holds the item number of the todo just added.
	//
//	int k = fListBox->count() - 1;
//
//	fListBox->setCurrentItem(k);	// Show the newest one
//	fListBox->setBottomItem(k);

	fPendingTodos--;
	if ( !shown && fPendingTodos==0 ) hideComponent();
}

void TodoWidget::slotUpdateRecord(PilotTodoEntry * todo)
{
	FUNCTIONSETUP;
	if ( !shown && fPendingTodos==0 ) return;

	writeTodo(todo);
	TodoCheckListItem* currentRecord = static_cast<TodoCheckListItem*>(fListBox->currentItem());

	// TODO: Just change the record
	updateWidget();
	fListBox->setCurrentItem(currentRecord);

	emit(recordChanged(todo));

	fPendingTodos--;
	if ( !shown && fPendingTodos==0 ) hideComponent();
}

void TodoWidget::slotEditCancelled()
{
	FUNCTIONSETUP;

	fPendingTodos--;
	if ( !shown && fPendingTodos==0 ) hideComponent();
}

void TodoWidget::slotDeleteRecord()
{
	FUNCTIONSETUP;
	if (!shown) return;

	TodoCheckListItem* p = static_cast<TodoCheckListItem*>(fListBox->currentItem());
	if (p == 0L) return;

	PilotTodoEntry *selectedRecord = (PilotTodoEntry *) p->rec();

	if (selectedRecord->id() == 0)
	{
		KMessageBox::error(this,
			i18n("New records cannot be deleted until "
				"HotSynced with pilot."),
			i18n("HotSync Required"));
		return;
	}

	if (KMessageBox::questionYesNo(this,
			i18n("Delete currently selected record?"),
			i18n("Delete Record?"), KStdGuiItem::del(), KStdGuiItem::cancel()) == KMessageBox::No)
		return;

	selectedRecord->setDeleted(true);
	writeTodo(selectedRecord);
	emit(recordChanged(selectedRecord));
	showComponent();
}



void TodoWidget::slotShowTodo(TQListViewItem*item)
{
	FUNCTIONSETUP;
	if (!shown) return;

	TodoCheckListItem *p = dynamic_cast<TodoCheckListItem*>(item);
	if (!p) return;
	PilotTodoEntry *todo = (PilotTodoEntry *) p->rec();

#ifdef DEBUG
	DEBUGKPILOT << fname << ": Showing "<< todo->getDescription()<<endl;
#endif

	TQString text(CSL1("<qt>"));
	text += todo->getTextRepresentation(Qt::RichText);
	text += CSL1("</qt>\n");
	fTodoInfo->setText(text);

	slotUpdateButtons();
}



void TodoWidget::writeTodo(PilotTodoEntry * which,
	PilotDatabase * todoDB)
{
	FUNCTIONSETUP;

	// Open a database (myDB) only if needed,
	// i.e. only if the passed-in todoDB
	// isn't valid.
	//
	//
	PilotDatabase *myDB = todoDB;
	bool usemyDB = false;

	if (myDB == 0L || !myDB->isOpen())
	{
		myDB = new PilotLocalDatabase(dbPath(), CSL1("ToDoDB"));
		usemyDB = true;
	}

	// Still no valid todo database...
	//
	//
	if (!myDB->isOpen())
	{
#ifdef DEBUG
		DEBUGKPILOT << fname << ": Todo database is not open" <<
			endl;
#endif
		return;
	}


	// Do the actual work.
	PilotRecord *pilotRec = which->pack();

	myDB->writeRecord(pilotRec);
	markDBDirty(CSL1("ToDoDB"));
	KPILOT_DELETE(pilotRec);


	// Clean up in the case that we allocated our own DB.
	//
	//
	if (usemyDB)
	{
		KPILOT_DELETE(myDB);
	}
}

void TodoWidget::slotItemChecked(TQCheckListItem*item, bool on)
{
	TodoCheckListItem*p = static_cast<TodoCheckListItem*>(item);
	if (!p) return;
	PilotTodoEntry *selectedRecord = (PilotTodoEntry *) p->rec();
	if (!selectedRecord) return;
	selectedRecord->setComplete(on);
	slotShowTodo(item);
}

void TodoWidget::slotItemRenamed(TQListViewItem*item, const TQString &txt, int nr)
{
	TodoCheckListItem*p = static_cast<TodoCheckListItem*>(item);
	if (!p) return;
	PilotTodoEntry *selectedRecord = (PilotTodoEntry *) p->rec();
	if (!selectedRecord) return;
	if (nr==0)
	{
		selectedRecord->setDescription(txt);
		slotShowTodo(item);
	}
}
