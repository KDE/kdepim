/* *******************************************************
   KPilot - Hot-Sync Software for Unix.
   Copyright 1998 by Dan Pilone
   This code is released under the GNU PUBLIC LICENSE.
   Please see the file 'COPYING' included in the KPilot
   distribution.
   *******************************************************
 */

#ifndef __TODO_SETUP_H
#define __TODO_SETUP_H

#include "gsetupDialog.h"

class QLineEdit;
class QCheckBox;
class QPushButton;


class TodoSetupPage : public setupDialogPage
{
	Q_OBJECT

public:
	TodoSetupPage(setupDialog *,KConfig *);

	virtual int commitChanges(KConfig *);
	virtual const char *tabName();

public slots:
	void slotBrowse();

private:
	QLineEdit* fCalendarFile;
	QCheckBox* fPromptYesNo;
	QPushButton *fBrowseButton;
} ;


class TodoSetup : public setupDialog
{
	Q_OBJECT

public:
	TodoSetup(QWidget *parent=0L);
  
	static const char *configGroup();
	virtual const char *groupName();
};

#endif
