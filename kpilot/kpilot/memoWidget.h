/* memoWidget.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This is the memo viewer widget.
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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/
#ifndef _KPILOT_MEMOWIDGET_H
#define _KPILOT_MEMOWIDGET_H

#ifndef QMLINED_H
#include <qmlined.h>
#endif

#ifndef QCOMBOBOX_H
#include <qcombobox.h>
#endif

#include <time.h>

#ifndef _PILOT_MEMO_H_
#include <pi-memo.h>
#endif

class KPilotInstaller;
class QListBox;


#ifndef _KPILOT_PILOTMEMO_H
#include "pilotMemo.h"
#endif

#ifndef _KPILOT_PILOTCOMPONENT_H
#include "pilotComponent.h"
#endif

class MemoWidget : public PilotComponent
{
  Q_OBJECT
  
public:
	MemoWidget(QWidget* parent, const QString& dbpath);
  ~MemoWidget();
  
    // Pilot Component Methods:
  void initialize();
  void preHotSync(char*);
  void postHotSync();
  bool saveData();
  
	typedef enum { 
		MAX_MEMO_LEN = 8192 
		} Constants ;

	// int findSelectedCategory(bool AllIsUnfiled=false);

protected:
	void initializeCategories(PilotDatabase *);
	void initializeMemos(PilotDatabase *);
  
 public slots:
	/**
	* Called whenever the selected memo changes to indicate
	* which buttons are active, mostly to prevent the delete
	* button from being active when it can't do anything.
	*/
	void slotUpdateButtons();
 void slotShowMemo(int);
  void slotTextChanged();
  void slotImportMemo();
  void slotExportMemo();
  void slotDeleteMemo(); // Delets the currently selected memo
  void slotSetCategory(int);

private:
  void setupWidget();
  void updateWidget(); // Called with the lists have changed..
  void writeMemo(PilotMemo* which);
  QComboBox* fCatList;
  
  QMultiLineEdit*    fTextWidget;
  struct MemoAppInfo fMemoAppInfo;
  QList<PilotMemo>   fMemoList;
  QListBox *          fListBox;

	QPushButton *fExportButton,*fDeleteButton;
};

#else
#warning "File doubly included"
#endif


// $Log$
// Revision 1.11  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.10  2001/03/04 13:11:49  adridg
// More response to bug 21392
//
// Revision 1.9  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
// Revision 1.8  2001/02/07 14:21:45  brianj
// Changed all include definitions for libpisock headers
// to use include path, which is defined in Makefile.
//
// Revision 1.7  2001/02/06 08:05:19  adridg
// Fixed copyright notices, added CVS log, added surrounding #ifdefs. No code changes.
//
