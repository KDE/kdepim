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



#include <qdir.h>
#include <qlabel.h>
#include <qpushbt.h>
#include <qdialog.h>
#include <qlined.h>
#include <qchkbox.h>
#include <qlayout.h>
#include <kapp.h>
#include <klocale.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include "vcal-conduit.h"
#include "vcal-setup.moc"
#include "kpilot.h"

VCalSetupPage::VCalSetupPage(setupDialog *parent,KConfig& config) :
	setupDialogPage(i18n("Calendar"),parent)
{
	FUNCTIONSETUP;

	QGridLayout *grid=new QGridLayout(this,2,3,SPACING);


	QLabel* currentLabel;


	currentLabel = new QLabel(i18n("Calendar File:"),
			    this);

	fCalendarFile = new QLineEdit(this);
	fCalendarFile->setText(config.readEntry("CalFile", ""));
	fCalendarFile->resize(200, fCalendarFile->height());

	fBrowseButton = new QPushButton(i18n("Browse"), this);
	fBrowseButton->adjustSize();
	connect(fBrowseButton, SIGNAL(clicked()), this, SLOT(slotBrowse()));


	grid->addWidget(currentLabel,0,0);
	grid->addWidget(fCalendarFile,0,1);
	grid->addWidget(fBrowseButton,0,2);

	fPromptYesNo = new QCheckBox(i18n("&Prompt before changing data."), 
		this);
	fPromptYesNo->adjustSize();
	fPromptYesNo->setChecked(config.readBoolEntry("FirstTime", TRUE));

	grid->addWidget(fPromptYesNo,1,1);
}

int VCalSetupPage::commitChanges(KConfig& config)
{
	FUNCTIONSETUP;

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



void VCalSetupPage::slotBrowse()
{
	FUNCTIONSETUP;

	QString fileName = KFileDialog::getOpenFileName(0L, "*.vcs");
	if(fileName.isNull()) return;
	fCalendarFile->setText(fileName);
}



/* static */ const QString VCalSetup::VCalGroup("vcalOptions");

VCalSetup::VCalSetup(QWidget *parent) :
	setupDialog(parent,VCalGroup)
{
	FUNCTIONSETUP;
	KConfig& config=KPilotLink::getConfig(VCalGroup);
	addPage(new VCalSetupPage(this,config));
	addPage(new setupInfoPage(this));

	setupDialog::setupWidget();
}


// $Log:$
