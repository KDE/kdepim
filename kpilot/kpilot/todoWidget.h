/* todoWidget.h			KPilot
**
** Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**

** This file defines the todo-viewing widget used in KPilot
** to display the Pilot's todo records.
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
#ifndef _KPILOT_TODOWIDGET_H
#define _KPILOT_TODOWIDGET_H

class TodoListView;
class QComboBox;
class QPushButton;
class QTextView;
class PilotTodoEntry;

class PilotDatabase;

#include "pilotComponent.h"
#include "pilotTodoEntry.h"
#include "listItems.h"

class TodoListView : public KListView
{
Q_OBJECT
public:
	TodoListView(QWidget * parent = 0, const char * name = 0 ):KListView(parent, name){};
	~TodoListView() {};
signals:
	void itemChecked(QCheckListItem*item);
	void itemChecked(QCheckListItem*item, bool on);
//protected:
public:
	void itemWasChecked(QCheckListItem*item, bool on) {
		emit itemChecked(item);
		emit itemChecked(item, on);
	}
};

class TodoCheckListItem : public PilotCheckListItem
{
public:
	TodoCheckListItem(QListView*parent, const QString&text, recordid_t pilotid, void*r);
	~TodoCheckListItem()  {};
	virtual void  stateChange(bool state);
};

class TodoWidget : public PilotComponent
{
Q_OBJECT

public:
	TodoWidget(QWidget* parent,const QString& dbpath);
	~TodoWidget();

	// Pilot Component Methods:
	virtual bool preHotSync(QString &);
	virtual void postHotSync();
	virtual void showComponent();
	virtual void hideComponent();

public slots:
	/**
	* Called when a particular todo is selected. This slot displays
	* it in the viewer widget.
	*/
	void slotShowTodo(QListViewItem*);
	void slotEditRecord(QListViewItem*item);
	void slotEditRecord();
	void slotCreateNewRecord();
	void slotDeleteRecord();
	void slotEditCancelled();

	void slotUpdateButtons();	// Enable/disable buttons

signals:
	void recordChanged(PilotTodoEntry *);

protected slots:
	/**
	* When an edit window is closed, the corresponding record
	* is updated and possibly re-displayed.
	*/
	void slotUpdateRecord(PilotTodoEntry*);

	/**
	* Pop up an edit window for a new record.
	*/
	void slotAddRecord(PilotTodoEntry*);

	/**
	* Change category. This means that the display should be
	* cleared and that the list should be repopulated.
	*/
	void slotSetCategory(int);


	void slotItemChecked(QCheckListItem*item, bool on);
	void slotItemRenamed(QListViewItem*item, const QString &txt, int nr);
private:
	void setupWidget();
	void updateWidget(); // Called with the lists have changed..
	void writeTodo(PilotTodoEntry* which,PilotDatabase *db=0L);

	/**
	* getAllTodos reads the database and places all
	* the todos from the database in the list
	* in memory --- not the list on the screen.
	* @see fTodoList
	*/
	int getAllTodos(PilotDatabase *todoDB);

	/**
	* Create a sensible "title" for an todo, composed
	* of first + last name if possible.
	*/
	QString createTitle(PilotTodoEntry *,int displayMode);

	/**
	* We use a QComboBox fCatList to hold the user-visible names
	* of all the categories. The QTextView fTodoInfo is for
	* displaying the currently selected todo, if any.
	* The QListView fListBox lists all the todoes in the
	* currently selected category.
	*
	* The entire todo database is read into memory in the
	* QList fTodoList. We need the appinfo block from the
	* database to determine which categories there are; this
	* is held in fTodoAppInfo.
	*
	* The two buttons should speak for themselves.
	*/
	QComboBox		*fCatList;
	QTextView		*fTodoInfo;
	struct ToDoAppInfo 	fTodoAppInfo;
	QPtrList<PilotTodoEntry>	fTodoList;
	TodoListView		*fListBox;
	QPushButton		*fEditButton,*fDeleteButton;
	PilotDatabase		*fTodoDB;
protected:
	/**
	* Keep track of how many open todo editing windows there
	* are. You can't sync when there are open windows.
	*/
	int fPendingTodos;

};

#endif
