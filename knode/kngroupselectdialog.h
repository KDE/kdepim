/***************************************************************************
                     kngroupselectdialog.h - description
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

#ifndef KNGROUPSELECTDIALOG_H
#define KNGROUPSELECTDIALOG_H


#include "kngroupbrowser.h"


class KNGroupSelectDialog : public KNGroupBrowser {

  Q_OBJECT

  public:
    KNGroupSelectDialog(QWidget *parent, KNNntpAccount *a, QCString &act);
		~KNGroupSelectDialog();
		
		QCString selectedGroups();
		void itemChangedState(CheckItem *it, bool s);
		
	protected:
	  void updateItemState(CheckItem *it, bool isSub);
	  QListView *selView;
	
	protected slots:
	  void slotItemSelected(QListViewItem *it);
	  void slotArrowBtn1();
	  void slotArrowBtn2();	
	  	  		
};

/*#include <qdialog.h>

class QListBox;
class QPushButton;
class QGroupBox;

class KNGroupListWidget;

class KNGroupSelectDialog : public QDialog  {
	
	Q_OBJECT

	public:
		KNGroupSelectDialog(KNNntpAccount *a, QCString &groups, QWidget *parent=0);
		~KNGroupSelectDialog();
		
		QCString& selectedGroups();
		
		
	protected:
		void resizeEvent(QResizeEvent *e);
 	  void addToSelected(const QString& text);
	
	  QGroupBox *gb1, *gb2;	  	
		QPushButton *ok, *cancel, *help, *add, *del;
		QListBox *lb;
		KNGroupListWidget *glw;
		QCString selGroups;
	
	protected slots:
		
		void slotAddBtn();				
		void slotRemoveBtn();			
		void slotAdd(const QString &text);
		void slotRemove(int i);
};*/

#endif















