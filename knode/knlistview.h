/***************************************************************************
                     knlistview.h - description
 copyright            : (C) 1999 by Christian Thurner
 email                : cthurner@freepage.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KNLISTVIEW_H
#define KNLISTVIEW_H

#include <qlistview.h>


class KNListView : public QListView  {

	Q_OBJECT

	public:
		KNListView(QWidget *parent, const char *name=0);
		~KNListView();
		
		int sortColumn()								{ return sCol; }
		bool ascending() 								{ return sAsc; }
		void setColAsc(int c, bool a)	  { sCol=c; sAsc=a; }
		
		virtual void setSelected(QListViewItem *item, bool select);
		void selectedRemoved()          { exclusiveSelectedItem = 0; }
		void clear();
		
	public slots:
		void slotSortList(int col);			
	
		
	protected:
		void keyPressEvent(QKeyEvent *e);
		void mouseDoubleClickEvent(QMouseEvent *e);
		void focusInEvent(QFocusEvent *e);
		void focusOutEvent(QFocusEvent *e);
		bool sAsc;
		int sCol;	
	
		
	signals:
		void sortingChanged(int);
		void focusChanged(QFocusEvent*);	
		
	private:
	  QListViewItem* exclusiveSelectedItem;     // single selection in multi mode hack...	
};

#endif

