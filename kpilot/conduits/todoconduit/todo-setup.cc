/* todo-setup.cc			KPilot
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
static const char *id="$Id$";



#include <qdir.h>
#include <qlabel.h>
#include <qpushbt.h>
#include <qchkbox.h>
#include <qlined.h>
#include <qlayout.h>
#include <kapp.h>
#include <klocale.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include "todo-conduit.h"
#include "todo-setup.moc"
#include "kpilotConfig.h"


/* static */ const QString TodoSetup::TodoGroup("todoOptions");

TodoSetup::TodoSetup(QWidget *parent)
  : setupDialog(parent,TodoGroup,TodoConduit::version())
{
	FUNCTIONSETUP;
	KConfig& config=KPilotConfig::getConfig(TodoGroup);
	addPage(new TodoSetupPage(this,config));
	addPage(new setupInfoPage(this));
	setupDialog::setupWidget();
}


int TodoSetupPage::commitChanges(KConfig& config)
{
	config.writeEntry("CalFile", fCalendarFile->text());
	if (fPromptYesNo->isChecked())
	{
		config.writeEntry("FirstTime", "true");
	}
	else
	{
		config.writeEntry("FirstTime", "false");
	}

	return 0;
}


void TodoSetupPage::slotBrowse()
{
	FUNCTIONSETUP;

  QString fileName = KFileDialog::getOpenFileName(0L, "*.vcs");
  if(fileName.isNull()) return;
  fCalendarFile->setText(fileName);
}

TodoSetupPage::TodoSetupPage(setupDialog *parent,KConfig& config) :
	setupDialogPage(i18n("ToDo File"),parent)
{
	FUNCTIONSETUP;

	QGridLayout *grid=new QGridLayout(this,2,3,SPACING);

  QLabel* currentLabel;


  currentLabel = new QLabel(i18n("Calendar File:"),
			    this);
  currentLabel->adjustSize();
  
  fCalendarFile = new QLineEdit(this);
  fCalendarFile->setText(config.readEntry("CalFile", ""));
  fCalendarFile->resize(200, fCalendarFile->height());

  fBrowseButton = new QPushButton(i18n("Browse"), this);
  fBrowseButton->adjustSize();
  connect(fBrowseButton, SIGNAL(clicked()), this, SLOT(slotBrowse()));
  
	grid->addWidget(currentLabel,0,0);
	grid->addWidget(fCalendarFile,0,1);
	grid->addWidget(fBrowseButton,0,2);

  fPromptYesNo = new QCheckBox(i18n("&Prompt before changing data."), this);
  fPromptYesNo->adjustSize();
  fPromptYesNo->setChecked(config.readBoolEntry("FirstTime", TRUE));

	grid->addWidget(fPromptYesNo,1,1);
}


// $Log$
// Revision 1.10  2001/02/07 15:46:32  adridg
// Updated copyright headers for source release. Added CVS log. No code change.
//
