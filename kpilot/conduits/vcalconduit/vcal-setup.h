/* vcal-setup.h		-*- C++ -*-	VCal Conduit
**
** Copyright (C) 1998-2001 Dan Pilone
**
** This file is part of the vcal conduit, a conduit for KPilot that
** synchronises the Pilot's datebook application with the outside world,
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

#ifndef __VCAL_SETUP_H
#define __VCAL_SETUP_H

class QLineEdit;
class QCheckBox;
class QPushButton;
class QGridLayout;
class QLabel;

#include "gsetupDialog.h"

class VCalSetupPage : public setupDialogPage
{
	Q_OBJECT

public:
	VCalSetupPage(setupDialog *, KConfig&);
	virtual ~VCalSetupPage();
	virtual int commitChanges(KConfig&);

public slots:
	void slotBrowse();

private:
  QLineEdit *fCalendarFile;
  QCheckBox *fPromptFirstTime;
  QPushButton *fBrowseButton;
  QCheckBox *fDeleteOnPilot;
  QGridLayout *grid;
  QLabel* FileLabel;
};


class VCalSetup : public setupDialog
{
  Q_OBJECT

  //friend class VCalConduit;
public:
  VCalSetup(QWidget *parent=0L);
  virtual ~VCalSetup();
  static const QString VCalGroup;

private:
  VCalSetupPage *sPage;
  setupInfoPage *iPage;
};

#endif


// $Log$
// Revision 1.6  2001/02/07 15:46:32  adridg
// Updated copyright headers for source release. Added CVS log. No code change.
//
