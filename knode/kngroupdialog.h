/***************************************************************************
                     kngroupdialog.h - description
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


#ifndef KNGROUPDIALOG_H
#define KNGROUPDIALOG_H

#include "kngroupbrowser.h"


class KNGroupDialog : public KNGroupBrowser {

  Q_OBJECT

  public:
    KNGroupDialog(QWidget *parent, KNNntpAccount *a);
    ~KNGroupDialog();

    void itemChangedState(CheckItem *it, bool s);
    void toSubscribe(QStrList *l);
    void toUnsubscribe(QStrList *l);
    void newList()  { slotLoadList(); }

  protected:
    enum arrowDirection { right, left };
    enum arrowButton { btn1, btn2 };
    void updateItemState(CheckItem *it, bool isSub);
    void setButtonDirection(arrowButton b, arrowDirection d);
    QPushButton *newListBtn;
    QListView *subView, *unsubView;
    arrowDirection dir1, dir2;

  protected slots:
    void slotItemSelected(QListViewItem *it);
    void slotArrowBtn1();
    void slotArrowBtn2();
    void slotNewListBtn();

  signals:
    void newList(KNNntpAccount *a);
};




/*#include <qsemimodal.h>

class QStrList;
class QPushButton;

class KNGroupListWidget;
class KNNntpAccount;

class KNGroupDialog : public QSemiModal  {
	
	friend class KNGroupManager;

	Q_OBJECT

	public:
		KNGroupDialog(KNNntpAccount *a, QWidget *parent=0);
		~KNGroupDialog();
		
		QStrList* sub()						{ return mSub; }
		QStrList* unsub()					{ return mUnsub; }		
			
	protected:
		QPushButton *ok, *cancel, *newList, *help;
		KNGroupListWidget *glw;
		QStrList *mActive, *mSub, *mUnsub;
			
	protected slots:

		void slotItemSelected(const QString &text);
		void slotHelp();
		void slotOk();
		void slotCancel();
		void slotNewList();
			
	signals:
		void dialogDone(bool);
		void getNewList(KNNntpAccount*);
};*/

#endif







