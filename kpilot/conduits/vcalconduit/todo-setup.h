/* todo-setup.h			KPilot
**
** Copyright (C) 1998-2001 Dan Pilone
** Copyright (C) 1998-2000 Preston Brown
**
** This file is part of the todo conduit, a conduit for KPilot that
** synchronises the Pilot's todo application with the outside world,
** which currently means KOrganizer.
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
#ifndef __TODO_SETUP_H
#define __TODO_SETUP_H

#include "gsetupDialog.h"

class QLineEdit;
class QCheckBox;
class QPushButton;
class QGridLayout;
class QLabel;

class TodoSetupPage : public setupDialogPage
{
	Q_OBJECT

public:
	TodoSetupPage(setupDialog *,KConfig&);
	virtual ~TodoSetupPage();

	virtual int commitChanges(KConfig&);

public slots:
	void slotBrowse();

private:
	QLineEdit* fCalendarFile;
	QCheckBox* fPromptFirstTime;
	QCheckBox* fDeleteOnPilot;
	QPushButton *fBrowseButton;
	QLabel* fCalFileLabel;
	QGridLayout* grid;
} ;


class TodoSetup : public setupDialog
{
	Q_OBJECT

friend class TodoConduit;
public:
	TodoSetup(QWidget *parent=0L);

protected:
	static const QString TodoGroup;
};

#endif


// $Log$
// Revision 1.1  2001/04/16 13:36:20  adridg
// Moved todoconduit
//
// Revision 1.6  2001/02/07 15:46:32  adridg
// Updated copyright headers for source release. Added CVS log. No code change.
//
