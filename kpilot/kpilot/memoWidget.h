#ifndef _KPILOT_MEMOWIDGET_H
#define _KPILOT_MEMOWIDGET_H
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "pilotComponent.h"

class KPilotInstaller;
class QListBox;
class QComboBox;
class QPushButton;
class KTextEdit;

class PilotMemo;
class PilotListItem;

class MemoWidget : public PilotComponent
{
Q_OBJECT

public:
	MemoWidget(TQWidget* parent, const TQString& dbpath);
	virtual ~MemoWidget();

	// Pilot Component Methods:
	/* virtual */ void showComponent();
	/* virtual */ void hideComponent();
	/* virtual */ bool preHotSync(TQString &);
	/* virtual */ void postHotSync();

	// Added by David Bishop, please move to correct location!
	bool saveAsXML(const TQString &fileName,const TQPtrList<PilotListItem> &menu_item );
	bool saveAsText(const TQString &fileName,const TQPtrList<PilotListItem> &menu_item );

	typedef enum {
		MAX_MEMO_LEN = 8192
		} Constants ;

protected:
	void initializeCategories(PilotDatabase *);
	void initializeMemos(PilotDatabase *);

	void saveChangedMemo();

	bool addMemo(const TQString &text, int category);

public slots:
	/**
	* Called whenever the selected memo changes in order to:
	*   - display it if necessary
	*   - update which buttons are active, to prevent the delete
	*     button from being active when it can't do anything.
	*
	*/
	void slotShowMemo(int);
	void slotUpdateButtons();

	void slotImportMemo();
	void slotExportMemo();
	void slotDeleteMemo();
	void slotAddMemo();
	void slotSetCategory(int);

protected:
	void showMemo(const PilotMemo *);


private:
	void setupWidget();
	void updateWidget(); // Called with the lists have changed..
	void writeMemo(PilotMemo* which);

	class Private;

	TQComboBox* fCatList;

	KTextEdit*		fTextWidget;
	Private *d;
	TQListBox*		fListBox;

	TQPushButton *fExportButton,*fDeleteButton;

	int lastSelectedMemo;
};


#endif
