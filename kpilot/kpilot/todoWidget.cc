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

#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QList>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QTextEdit>


#include <KMessageBox>

#include "pilot.h"
#include "pilotLocalDatabase.h"
#include "pilotTodoEntry.h"

#include "kpilotConfig.h"
#include "todoEditor.h"


#include "todoWidget.moc"


class TodoItem : public QListWidgetItem
{
public:
	TodoItem( QListWidget *parent, PilotTodoEntry *r ) :
		QListWidgetItem( parent ),
		fRecord(r)
	{
		setText( r->getDescription() );
		QString n = r->getNote();
		if (!n.isEmpty())
		{
			setToolTip(n);
		}
		setFlags( Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
		setCheckState( r->getComplete() ? Qt::Checked : Qt::Unchecked );
	}

	const PilotTodoEntry *record() const
	{
		return fRecord;
	}
protected:
	PilotTodoEntry *fRecord;
} ;

class TodoWidget::Private
{
public:
	Private() : fTodoDB(0L), fAppInfo(0L), fPendingTodos(0) {}
	~Private()
	{
		KPILOT_DELETE(fAppInfo);
		KPILOT_DELETE(fTodoDB);
	}
	PilotDatabase *fTodoDB;
	PilotToDoInfo *fAppInfo;
	QList<TodoItem *> fItems;

	/**
	* Keep track of how many open todo editing windows there
	* are. You can't sync when there are open windows.
	*/
	int fPendingTodos;

} ;

TodoWidget::TodoWidget(QWidget *parent, const QString & path) :
	PilotComponent(parent, "component_todo", path),
	fCategoryList(0L),
	fTodoList(0L),
	fTodoViewer(0L),
	fEditButton(0L),
	fDeleteButton(0L),
	fP( new Private )
{
	FUNCTIONSETUP;

	QLabel *label;
	QGridLayout *grid = new QGridLayout(this);
	grid->setSpacing( SPACING );

	fCategoryList = new QComboBox(this);
	grid->addWidget(fCategoryList, 0, 1);
	connect(fCategoryList, SIGNAL(activated(int)),
		this, SLOT(slotSetCategory(int)));
	fCategoryList->setWhatsThis(
		i18n("<qt>Select the category of to-dos to display here.</qt>"));

	label = new QLabel(i18n("Category:"), this);
	label->setBuddy(fCategoryList);
	grid->addWidget(label, 0, 0);

	fTodoList = new QListWidget(this);
	grid->addWidget(fTodoList, 1, 0, 1, 2);
	connect(fTodoList, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
		this, SLOT(slotShowTodo(QListWidgetItem *)));
	fTodoList->setWhatsThis(
		i18n("<qt>This list displays all the to-dos "
			"in the selected category. Click on "
			"one to display it to the right.</qt>"));

	label = new QLabel(i18n("To-do info:"), this);
	grid->addWidget(label, 0, 2);

	// todo info text view
	fTodoViewer = new QTextEdit(this);
	fTodoViewer->setReadOnly( true );
	grid->addWidget(fTodoViewer, 1, 2, 4, 1);

	QPushButton *button;
	QString wt;

	fEditButton = new QPushButton(i18n("Edit Record..."), this);
	grid->addWidget(fEditButton, 2, 0);
	connect(fEditButton, SIGNAL(clicked()), this, SLOT(slotEditRecord()));

	wt = KPilotSettings::internalEditors() ?
		i18n("<qt>You can edit a to-do when it is selected.</qt>") :
		i18n("<qt><i>Editing is disabled by the 'internal editors' setting.</i></qt>");
	fEditButton->setWhatsThis(wt);

	button = new QPushButton(i18n("New Record..."), this);
	grid->addWidget(button, 2, 1);
	connect(button, SIGNAL(clicked()), this, SLOT(slotCreateNewRecord()));
	wt = KPilotSettings::internalEditors() ?
		i18n("<qt>Add a new to-do to the to-do list.</qt>") :
		i18n("<qt><i>Adding new to-dos is disabled by the 'internal editors' setting.</i></qt>");
	button->setWhatsThis( wt);
	button->setEnabled(KPilotSettings::internalEditors());

	fDeleteButton = new QPushButton(i18n("Delete Record"), this);
	grid->addWidget(fDeleteButton, 3, 0);
	connect(fDeleteButton, SIGNAL(clicked()),
		this, SLOT(slotDeleteRecord()));
	wt = KPilotSettings::internalEditors() ?
		i18n("<qt>Delete the selected to-do from the to-do list.</qt>") :
		i18n("<qt><i>Deleting is disabled by the 'internal editors' setting.</i></qt>") ;
	fDeleteButton->setWhatsThis(wt);

	slotUpdateButtons();
}


TodoWidget::~TodoWidget()
{
	FUNCTIONSETUP;
	KPILOT_DELETE( fP );
}

int TodoWidget::getAllTodos(PilotDatabase *todoDB)
{
	FUNCTIONSETUP;

	int currentRecord = 0;
	PilotRecord *pilotRec;
	PilotTodoEntry *todo;

	DEBUGKPILOT << "Reading ToDoDB...";

	while ((pilotRec = todoDB->readRecordByIndex(currentRecord)) != 0L)
	{
		if (!(pilotRec->isDeleted()) &&
			(!(pilotRec->isSecret()) || KPilotSettings::showSecrets()))
		{
			todo = new PilotTodoEntry(pilotRec);
			if (todo == 0L)
			{
				WARNINGKPILOT << "Couldn't allocate record"
					<< currentRecord++;
				break;
			}
			fP->fItems.append( new TodoItem( fTodoList, todo ) );
		}
		KPILOT_DELETE( pilotRec );

		currentRecord++;
	}

	DEBUGKPILOT << "Total " << currentRecord << " records.";

	return currentRecord;
}

void TodoWidget::showComponent()
{
	FUNCTIONSETUP;
	if ( fP->fPendingTodos>0 )
	{
		WARNINGKPILOT << "Open todo editors prevent re-reading data.";
		return;
	}

	DEBUGKPILOT << "Reading from directory [" << dbPath() << ']';

	fP->fItems.clear();
	fTodoList->clear();

	fP->fTodoDB = new PilotLocalDatabase(dbPath(), CSL1("ToDoDB"));
	if (fP->fTodoDB->isOpen())
	{
		KPILOT_DELETE(fP->fAppInfo);
		fP->fAppInfo = new PilotToDoInfo(fP->fTodoDB);
		populateCategories(fCategoryList, fP->fAppInfo->categoryInfo());
		getAllTodos(fP->fTodoDB);
	}
	else
	{
		populateCategories(fCategoryList, 0L);
		WARNINGKPILOT << "Could not open local TodoDB in [" << dbPath() << ']';
	}
	KPILOT_DELETE( fP->fTodoDB );

	updateWidget();
	slotUpdateButtons();
}

/* virtual */ bool TodoWidget::preHotSync(QString &s)
{
	FUNCTIONSETUP;

	if (fP->fPendingTodos)
	{
		DEBUGKPILOT << "fPendingTodo=" << fP->fPendingTodos;

		s = i18np("There is still a to-do editing window open.",
			"There are still %1 to-do editing windows open.",
			fP->fPendingTodos);
		return false;
	}

	return true;
}

void TodoWidget::postHotSync()
{
	FUNCTIONSETUP;

	fP->fItems.clear();
	fTodoList->clear();
	showComponent();
}

void TodoWidget::hideComponent()
{
	FUNCTIONSETUP;
	if ( fP->fPendingTodos==0 )
	{
		fP->fItems.clear();
		fTodoList->clear();
		KPILOT_DELETE( fP->fAppInfo );
		KPILOT_DELETE( fP->fTodoDB );
	}
	fTodoViewer->clear();
}

void TodoWidget::updateWidget()
{
	FUNCTIONSETUP;
#if BADLY_PORTED
	if (!shown || !fP->fAppInfo ) return;

	int listIndex = 0;

	int currentCatID = findSelectedCategory(fCategoryList,
		fAppInfo->categoryInfo());

	DEBUGKPILOT << "Adding records...";

	PilotTodoEntry*todo;
	while (fTodoList.current())
	{
		todo=fTodoList.current();
		if ((currentCatID == -1) ||
			(todo->category() == currentCatID))
		{
			QString title = todo->getDescription();

			TodoCheckListItem*item=new TodoCheckListItem(fListBox, title,
				listIndex, todo);
			item->setOn(todo->getComplete());
		}
		listIndex++;
		fTodoList.next();
	}

	DEBUGKPILOT  << listIndex << " records";

	slotUpdateButtons();
#endif
}



/* slot */ void TodoWidget::slotUpdateButtons()
{
	FUNCTIONSETUP;

	bool enabled = fTodoList->currentItem();

	enabled &= KPilotSettings::internalEditors() ;

	fEditButton->setEnabled(enabled);
	fDeleteButton->setEnabled(enabled);
}

void TodoWidget::slotSetCategory(int category)
{
	FUNCTIONSETUP;
	DEBUGKPILOT << "Selected category index" << category;
	int category_value = fCategoryList->itemData(category).toInt();
	DEBUGKPILOT << "Selected category value" << category_value;

	int e = fP->fItems.size();
	DEBUGKPILOT << "Scanning" << e << " items.";
	for (int i = 0; i<e; ++i)
	{
		TodoItem *item = fP->fItems[i];
		int item_category = item->record()->category();
		DEBUGKPILOT << "Item [" << item->text()
			<< "] category " << item_category;
		item->setHidden(category && (item_category != category_value));
	}
}

void TodoWidget::slotEditRecord()
{
	FUNCTIONSETUP;
}
void TodoWidget::slotEditRecord(QListWidgetItem *item)
{
	FUNCTIONSETUP;
	if (!isVisible())
	{
		return;
	}

	TodoItem *p = static_cast<TodoItem*>(item);
	if (!p)
	{
		return;
	}

#if 0
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

	connect(editor, SIGNAL(recordChangeComplete(PilotTodoEntry *)),
		this, SLOT(slotUpdateRecord(PilotTodoEntry *)));
	connect(editor, SIGNAL(cancelClicked()),
		this, SLOT(slotEditCancelled()));
	editor->show();

	fPendingTodos++;
#endif
}

void TodoWidget::slotCreateNewRecord()
{
	FUNCTIONSETUP;
	if (!isVisible())
	{
		return;
	}

#if 0
	// Response to bug 18072: Don't even try to
	// add records to an empty or unopened database,
	// since we don't have the DBInfo stuff to deal with it.
	//
	//
	PilotDatabase *myDB = new PilotLocalDatabase(dbPath(), CSL1("ToDoDB"));

	if (!myDB || !myDB->isOpen())
	{
		DEBUGKPILOT << "Tried to open ToDoDB in ["
			<< dbPath()
			<< "] and got pointer @"
			<< (void *) myDB
			<< " with status "
			<< ( myDB ? myDB->isOpen() : false );

		KMessageBox::sorry(this,
			i18n("You cannot add to-dos to the to-do list "
				"until you have done a HotSync at least once "
				"to retrieve the database layout from your Pilot."),
			i18n("Cannot Add New To-do"));

		KPILOT_DELETE( myDB );

		return;
	}

	TodoEditor *editor = new TodoEditor(0L,
		fTodoAppInfo->info(), this);

	connect(editor, SIGNAL(recordChangeComplete(PilotTodoEntry *)),
		this, SLOT(slotAddRecord(PilotTodoEntry *)));
	connect(editor, SIGNAL(cancelClicked()),
		this, SLOT(slotEditCancelled()));
	editor->show();

	fPendingTodos++;
#endif
}

void TodoWidget::slotAddRecord(PilotTodoEntry * todo)
{
#if 0
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
#endif
}

void TodoWidget::slotUpdateRecord(PilotTodoEntry * todo)
{
	FUNCTIONSETUP;
#if 0
	if ( !shown && fPendingTodos==0 ) return;

	writeTodo(todo);
	TodoCheckListItem* currentRecord = static_cast<TodoCheckListItem*>(fListBox->currentItem());

	// TODO: Just change the record
	updateWidget();
	fListBox->setCurrentItem(currentRecord);

	emit(recordChanged(todo));

	fPendingTodos--;
	if ( !shown && fPendingTodos==0 ) hideComponent();
#endif
}

void TodoWidget::slotEditCancelled()
{
	FUNCTIONSETUP;

	fP->fPendingTodos--;
	if ( !isVisible() && fP->fPendingTodos==0 )
	{
		hideComponent();
	}
}

void TodoWidget::slotDeleteRecord()
{
	FUNCTIONSETUP;
#if 0
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
			i18n("Delete Record?"), KStandardGuiItem::del(), KStandardGuiItem::cancel()) == KMessageBox::No)
		return;

	selectedRecord->setDeleted(true);
	writeTodo(selectedRecord);
	emit(recordChanged(selectedRecord));
	showComponent();
#endif
}



void TodoWidget::slotShowTodo(QListWidgetItem *item)
{
	FUNCTIONSETUP;
	if (!isVisible())
	{
		DEBUGKPILOT << "Widget is not shown. Ignoring.";
		return;
	}

	TodoItem *p = dynamic_cast<TodoItem*>(item);
	if (!p)
	{
		DEBUGKPILOT << "Item is not a TodoItem.";
		return;
	}

	QString text(CSL1("<qt>"));
	text += p->record()->getTextRepresentation(Qt::RichText);
	text += CSL1("</qt>\n");
	fTodoViewer->setText(text);

	slotUpdateButtons();
}



void TodoWidget::writeTodo(PilotTodoEntry * which,
	PilotDatabase * todoDB)
{
	FUNCTIONSETUP;
#if 0
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
		DEBUGKPILOT << "Todo database is not open.";
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
#endif
}

void TodoWidget::slotItemChecked(QListWidgetItem *item, bool on)
{
	FUNCTIONSETUP;
}

void TodoWidget::slotItemRenamed(QListWidgetItem *item, const QString &txt, int nr)
{
	FUNCTIONSETUP;
}
