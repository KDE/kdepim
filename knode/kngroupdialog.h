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

#include <qsemimodal.h>

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
};

#endif







