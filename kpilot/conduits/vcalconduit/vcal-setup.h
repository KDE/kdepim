// vcal-setup.h
//
// Copyright (C) 1998,1999 Dan Pilone
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
// $Revision$





#ifndef __VCAL_SETUP_H
#define __VCAL_SETUP_H

class QLineEdit;
class QCheckBox;
class QPushButton;

#include "gsetupDialog.h"

class VCalSetupPage : public setupDialogPage
{
	Q_OBJECT

public:
	VCalSetupPage(setupDialog *,KConfig&);

	virtual int commitChanges(KConfig&);

public slots:
	void slotBrowse();

private:
	QLineEdit* fCalendarFile;
	QCheckBox* fPromptYesNo;
	QPushButton *fBrowseButton;
} ;




class VCalSetup : public setupDialog
{
  Q_OBJECT

friend class VCalConduit;
public:
	VCalSetup(QWidget *parent=0L);

protected:
	static const QString VCalGroup;
};

#endif
