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

#include <QGridLayout>
#include <QLabel>
#include <QList>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>

#include <KComboBox>
#include <KMessageBox>
#include <KTextEdit>

#include "pilot.h"
#include "pilotLocalDatabase.h"
#include "pilotTodoEntry.h"

#include "kpilotConfig.h"

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
	Private() : fTodoDB(0L), fAppInfo(0L) {}
	~Private()
	{
		KPILOT_DELETE(fAppInfo);
		KPILOT_DELETE(fTodoDB);
	}
	PilotDatabase *fTodoDB;
	PilotToDoInfo *fAppInfo;
	QList<TodoItem *> fItems;

} ;

TodoWidget::TodoWidget(QWidget *parent, const QString & path) :
	PilotComponent(parent, "component_todo", path),
	fCategoryList(0L),
	fTodoList(0L),
	fTodoViewer(0L),
	fP( new Private )
{
	FUNCTIONSETUP;

	QLabel *label;
	QGridLayout *grid = new QGridLayout(this);
	grid->setSpacing( SPACING );

	fCategoryList = new KComboBox(this);
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
	fTodoViewer = new KTextEdit(this);
	fTodoViewer->setReadOnly( true );
	grid->addWidget(fTodoViewer, 1, 2, 4, 1);

	QString wt;

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
	Q_UNUSED(s);
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
	fP->fItems.clear();
	fTodoList->clear();
	KPILOT_DELETE( fP->fAppInfo );
	KPILOT_DELETE( fP->fTodoDB );
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


