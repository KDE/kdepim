/* todoWidget.cc			KPilot
**
** Copyright (C) 2003 by Dan Pilone
** Written 2003 by Reinhold Kainhofer
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/
static const char *todowidget_id =
	"$Id$";


#include "options.h"

#include <qptrlist.h>
#include <klistview.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qtextview.h>
#include <qcombobox.h>
#include <qwhatsthis.h>
#include <qtextcodec.h>

#include <kmessagebox.h>

#include "kpilotConfig.h"
#include "todoEditor.h"
#include "pilotLocalDatabase.h"
#include "todoWidget.moc"




TodoCheckListItem::TodoCheckListItem(QListView*parent, const QString&text,
	recordid_t pilotid, void*r):PilotCheckListItem(parent, text, pilotid, r)
{

}

void TodoCheckListItem::stateChange(bool state)
{
	TodoListView*par=dynamic_cast<TodoListView*>(listView());
	if (par) par->itemWasChecked(this, state);
}



TodoWidget::TodoWidget(QWidget * parent,
	const QString & path) :
	PilotComponent(parent, "component_todo", path),
	fTodoInfo(0),
	fTodoDB(0),
	fPendingTodos(0)
{
	FUNCTIONSETUP;

	setupWidget();
	fTodoList.setAutoDelete(true);

	/* NOTREACHED */
	(void) todowidget_id;
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
			todo = new PilotTodoEntry(fTodoAppInfo, pilotRec);
			if (todo == 0L)
			{
				kdWarning() << k_funcinfo
					<< ": Couldn't allocate record "
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
	unsigned char buffer[PilotRecord::APP_BUFFER_SIZE];
	int appLen;

	fTodoList.clear();

	if (fTodoDB->isDBOpen())
	{
		appLen = fTodoDB->readAppBlock(buffer, PilotRecord::APP_BUFFER_SIZE);
		unpack_ToDoAppInfo(&fTodoAppInfo, buffer, appLen);

		populateCategories(fCatList, &fTodoAppInfo.category);
		getAllTodos(fTodoDB);

	}
	else
	{
		populateCategories(fCatList, 0L);
		kdWarning() << k_funcinfo
			<< ": Could not open local TodoDB" << endl;
	}

	KPILOT_DELETE( fTodoDB );

	updateWidget();
}

/* virtual */ bool TodoWidget::preHotSync(QString &s)
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
			.arg(QString::number(fPendingTodos));
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

	QLabel *label;
	QGridLayout *grid = new QGridLayout(this, 6, 4, SPACING);

	fCatList = new QComboBox(this);
	grid->addWidget(fCatList, 0, 1);
	connect(fCatList, SIGNAL(activated(int)),
		this, SLOT(slotSetCategory(int)));
	QWhatsThis::add(fCatList,
		i18n("<qt>Select the category of to-dos to display here.</qt>"));

	label = new QLabel(i18n("Category:"), this);
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
	connect(fListBox, SIGNAL(selectionChanged(QListViewItem*)),
		this, SLOT(slotShowTodo(QListViewItem*)));
	connect(fListBox, SIGNAL(doubleClicked(QListViewItem*)),
		this, SLOT(slotEditRecord(QListViewItem*)));
	connect(fListBox, SIGNAL(returnPressed(QListViewItem*)),
		this, SLOT(slotEditRecord(QListViewItem*)));
	connect(fListBox, SIGNAL(itemChecked(QCheckListItem*, bool)),
		this, SLOT(slotItemChecked(QCheckListItem*, bool)));
	connect(fListBox, SIGNAL(itemRenamed(QListViewItem*, const QString &, int)),
		this, SLOT(slotItemRenamed(QListViewItem*, const QString &, int)));
	QWhatsThis::add(fListBox,
		i18n("<qt>This list displays all the to-dos "
			"in the selected category. Click on "
			"one to display it to the right.</qt>"));

	label = new QLabel(i18n("To-do info:"), this);
	grid->addWidget(label, 0, 2);

	// todo info text view
	fTodoInfo = new QTextView(this);
	grid->addMultiCellWidget(fTodoInfo, 1, 4, 2, 2);

	QPushButton *button;
	QString wt;

	fEditButton = new QPushButton(i18n("Edit Record..."), this);
	grid->addWidget(fEditButton, 2, 0);
	connect(fEditButton, SIGNAL(clicked()), this, SLOT(slotEditRecord()));

	wt = KPilotSettings::internalEditors() ?
		i18n("<qt>You can edit a to-do when it is selected.</qt>") :
		i18n("<qt><i>Editing is disabled by the 'internal editors' setting.</i></qt>");
	QWhatsThis::add(fEditButton,wt);

	button = new QPushButton(i18n("New Record..."), this);
	grid->addWidget(button, 2, 1);
	connect(button, SIGNAL(clicked()), this, SLOT(slotCreateNewRecord()));
	wt = KPilotSettings::internalEditors() ?
		i18n("<qt>Add a new to-do to the to-do list.</qt>") :
		i18n("<qt><i>Adding new to-dos is disabled by the 'internal editors' setting.</i></qt>");
	QWhatsThis::add(button, wt);
	button->setEnabled(KPilotSettings::internalEditors());

	fDeleteButton = new QPushButton(i18n("Delete Record"), this);
	grid->addWidget(fDeleteButton, 3, 0);
	connect(fDeleteButton, SIGNAL(clicked()),
		this, SLOT(slotDeleteRecord()));
	wt = KPilotSettings::internalEditors() ?
		i18n("<qt>Delete the selected to-do from the to-do list.</qt>") :
		i18n("<qt><i>Deleting is disabled by the 'internal editors' setting.</i></qt>") ;
	QWhatsThis::add(fDeleteButton,wt);
}

void TodoWidget::updateWidget()
{
	FUNCTIONSETUP;
	if (!shown) return;

	int listIndex = 0;

	int currentCatID = findSelectedCategory(fCatList,
		&(fTodoAppInfo.category));

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
			(todo->getCat() == currentCatID))
		{
			QString title = todo->getDescription();

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
void TodoWidget::slotEditRecord(QListViewItem*item)
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
		&fTodoAppInfo, this);

	connect(editor, SIGNAL(recordChangeComplete(PilotTodoEntry *)),
		this, SLOT(slotUpdateRecord(PilotTodoEntry *)));
	connect(editor, SIGNAL(cancelClicked()),
		this, SLOT(slotEditCancelled()));
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

	if (!myDB || !myDB->isDBOpen())
	{
#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": Tried to open "
			<< dbPath()
			<< "/ToDoDB"
			<< " and got pointer @"
			<< (void *) myDB
			<< " with status "
			<< ( myDB ? myDB->isDBOpen() : false )
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
		&fTodoAppInfo, this);

	connect(editor, SIGNAL(recordChangeComplete(PilotTodoEntry *)),
		this, SLOT(slotAddRecord(PilotTodoEntry *)));
	connect(editor, SIGNAL(cancelClicked()),
		this, SLOT(slotEditCancelled()));
	editor->show();

	fPendingTodos++;
}

void TodoWidget::slotAddRecord(PilotTodoEntry * todo)
{
	FUNCTIONSETUP;
	if ( !shown && fPendingTodos==0 ) return;

	int currentCatID = findSelectedCategory(fCatList,
		&(fTodoAppInfo.category), true);


	todo->setCat(currentCatID);
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
			i18n("Delete Record?")) == KMessageBox::No)
		return;

	selectedRecord->makeDeleted();
	writeTodo(selectedRecord);
	emit(recordChanged(selectedRecord));
	showComponent();
}



void TodoWidget::slotShowTodo(QListViewItem*item)
{
	FUNCTIONSETUP;
	if (!shown) return;

	TodoCheckListItem *p = dynamic_cast<TodoCheckListItem*>(item);
	if (!p) return;
	PilotTodoEntry *todo = (PilotTodoEntry *) p->rec();

#ifdef DEBUG
	DEBUGKPILOT << fname << ": Showing "<< todo->getDescription()<<endl;
#endif

	QString text(CSL1("<qt>"));
	text += todo->getTextRepresentation(true);
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

	if (myDB == 0L || !myDB->isDBOpen())
	{
		myDB = new PilotLocalDatabase(dbPath(), CSL1("ToDoDB"));
		usemyDB = true;
	}

	// Still no valid todo database...
	//
	//
	if (!myDB->isDBOpen())
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
	markDBDirty("ToDoDB");
	KPILOT_DELETE(pilotRec);


	// Clean up in the case that we allocated our own DB.
	//
	//
	if (usemyDB)
	{
		KPILOT_DELETE(myDB);
	}
}

void TodoWidget::slotItemChecked(QCheckListItem*item, bool on)
{
	TodoCheckListItem*p = static_cast<TodoCheckListItem*>(item);
	if (!p) return;
	PilotTodoEntry *selectedRecord = (PilotTodoEntry *) p->rec();
	if (!selectedRecord) return;
	selectedRecord->setComplete(on);
	slotShowTodo(item);
}

void TodoWidget::slotItemRenamed(QListViewItem*item, const QString &txt, int nr)
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
