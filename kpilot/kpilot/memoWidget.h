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
#ifndef __MEMO_WIDGET_H
#define __MEMO_WIDGET_H

#include "pilotComponent.h"
#include <qmlined.h>
#include <qcombo.h>
#include <time.h>
#include "pi-memo.h"
#include "pilotMemo.h"
#include "kpilotlink.h"

class KPilotInstaller;
class QListBox;

class MemoWidget : public PilotComponent
{
  Q_OBJECT
  
public:
  MemoWidget(KPilotInstaller* installer, QWidget* parent);
  ~MemoWidget();
  
    // Pilot Component Methods:
  void initialize();
  void preHotSync(char*);
  void postHotSync();
  bool saveData();
  
  static int MAX_MEMO_LEN;

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
  unsigned int       fLookupTable[1000]; // Used to index the listbox into the memolist
  QListBox *          fListBox;

	QPushButton *fExportButton,*fDeleteButton;
};

#endif


// $Log:$
