/* vcal-setup.cc			VCal Conduit
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


#include "options.h"

#ifndef QDIR_H
#include <qdir.h>
#endif

#ifndef QLABEL_H
#include <qlabel.h>
#endif

#ifndef QPUSHBT_H
#include <qpushbt.h>
#endif

#ifndef QDIALOG_H
#include <qdialog.h>
#endif

#ifndef QLINED_H
#include <qlined.h>
#endif

#ifndef QCHKBOX_H
#include <qchkbox.h>
#endif

#ifndef QLAYOUT_H
#include <qlayout.h>
#endif

#ifndef _KAPP_H
#include <kapp.h>
#endif

#ifndef _KLOCALE_H
#include <klocale.h>
#endif

#ifndef _KDEBUG_H
#include <kdebug.h>
#endif

#ifndef _KFILEDIALOG_H
#include <kfiledialog.h>
#endif

#ifndef _KDEBUG_H
#include <kdebug.h>
#endif

#ifndef _KPILOT_KPILOTCONFIG_H
#include "kpilotConfig.h"
#endif

#ifndef _KPILOT_VCAL_CONDUIT_H
#include "vcal-conduit.h"
#endif


#include "vcal-setup.moc"

VCalSetupPage::VCalSetupPage(setupDialog *parent, KConfig& config) :
	setupDialogPage(i18n("Calendar"),parent)
{
  FUNCTIONSETUP;

  grid = new QGridLayout(this, 2, 4, SPACING);
  
  FileLabel =
    new QLabel(i18n("Calendar File:"), this);

  fCalendarFile = new QLineEdit(this);
  fCalendarFile->setText(config.readEntry("CalFile", ""));
  fCalendarFile->resize(200, fCalendarFile->height());
  
  fBrowseButton = new QPushButton(i18n("Browse"), this);
  fBrowseButton->adjustSize();
  connect(fBrowseButton, SIGNAL(clicked()), this, SLOT(slotBrowse()));
    
  grid->addWidget(FileLabel, 0, 0);
  grid->addWidget(fCalendarFile, 0, 1);
  grid->addWidget(fBrowseButton, 0, 2);
  
  fPromptFirstTime = new QCheckBox(i18n("&Prompt before changing data."), 
				   this);
  fPromptFirstTime->adjustSize();
  fPromptFirstTime->setChecked(config.readBoolEntry("FirstTime", TRUE));
  
  grid->addWidget(fPromptFirstTime, 1, 1);
  
  fDeleteOnPilot =
    new QCheckBox(i18n("Delete locally deleted records on pilot"),
		  this);
  fDeleteOnPilot->adjustSize();
  fDeleteOnPilot->setChecked(config.readBoolEntry("DeleteOnPilot", true));
  grid->addWidget(fDeleteOnPilot, 2, 1);
}


VCalSetupPage::~VCalSetupPage()
{
  delete fCalendarFile;
  delete fPromptFirstTime;
  delete fBrowseButton;
  delete fDeleteOnPilot;
  delete FileLabel;
  delete grid;
}


int VCalSetupPage::commitChanges(KConfig& config)
{
  FUNCTIONSETUP;

  config.writeEntry("CalFile", fCalendarFile->text());
  config.writeEntry("FirstTime", 
		    fPromptFirstTime->isChecked() ? "true" : "false");
  config.writeEntry("DeleteOnPilot",
		    fDeleteOnPilot->isChecked() ? "true" : "false");
  return 0;
}



void VCalSetupPage::slotBrowse()
{
	FUNCTIONSETUP;

	QString fileName = KFileDialog::getOpenFileName(0L, "*.vcs");
	if(fileName.isNull()) return;
	fCalendarFile->setText(fileName);
}



/* static */ const QString VCalSetup::VCalGroup("vcalOptions");

VCalSetup::VCalSetup(QWidget *parent) :
	setupDialog(parent, VCalGroup)
{
	FUNCTIONSETUP;
	KConfig& config = KPilotConfig::getConfig(VCalGroup);
	sPage = new VCalSetupPage(this, config);
	if (sPage) addPage(sPage);
	iPage = new setupInfoPage(this);
	if (iPage) addPage(iPage);

	setupDialog::setupWidget();
}

VCalSetup::~VCalSetup() {
  delete sPage;
  delete iPage;
}


// $Log$
// Revision 1.13  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.12  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
// Revision 1.11  2001/02/07 15:46:32  adridg
// Updated copyright headers for source release. Added CVS log. No code change.
//
