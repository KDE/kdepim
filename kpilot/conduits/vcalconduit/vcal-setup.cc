// vcal-setup.cc
//
// Copyright (C) 1998,1999 Dan Pilone
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
// $Revision$




#include <qdir.h>
#include <qlabel.h>
#include <qpushbt.h>
#include <qdialog.h>
#include <qlined.h>
#include <qchkbox.h>
#include <kapp.h>
#include <klocale.h>
#include <kfiledialog.h>
#include "vcal-conduit.h"
#include "vcal-setup.moc"
#include "kpilot.h"

VCalSetupPage::VCalSetupPage(setupDialog *parent,KConfig *config) :
	setupDialogPage(parent,config)
{
	FUNCTIONSETUP;


	QLabel* currentLabel;


	currentLabel = new QLabel(i18n("Calendar File:"),
			    this);
	currentLabel->adjustSize();
	currentLabel->move(10, 10);

	fCalendarFile = new QLineEdit(this);
	fCalendarFile->setText(config->readEntry("CalFile", ""));
	fCalendarFile->resize(200, fCalendarFile->height());
	fCalendarFile->move(RIGHT(currentLabel), currentLabel->y()-4);

	fBrowseButton = new QPushButton(i18n("Browse"), this);
	fBrowseButton->adjustSize();
	fBrowseButton->move(RIGHT(fCalendarFile), fCalendarFile->y());
	connect(fBrowseButton, SIGNAL(clicked()), this, SLOT(slotBrowse()));

	fPromptYesNo = new QCheckBox(i18n("&Prompt before changing data."), this);
	fPromptYesNo->adjustSize();
	fPromptYesNo->setChecked(config->readBoolEntry("FirstTime", TRUE));
	fPromptYesNo->move(fCalendarFile->x(), BELOW(fCalendarFile));
}

int VCalSetupPage::commitChanges(KConfig *config)
{
	FUNCTIONSETUP;

	config->writeEntry("CalFile", fCalendarFile->text());
	if (fPromptYesNo->isChecked())
	{
		config->writeEntry("FirstTime", "true");
	}
	else
	{
		config->writeEntry("FirstTime", "false");
	}

	return 0;
}

/* virtual */ const char *VCalSetupPage::tabName()
{
	return i18n("Calendar");
}


void VCalSetupPage::slotBrowse()
{
	FUNCTIONSETUP;

	QString fileName = KFileDialog::getOpenFileName(0L, "*.vcs");
	if(fileName.isNull()) return;
	fCalendarFile->setText(fileName);
}



/* virtual */ const char *VCalSetup::groupName()
{
	return configGroup();
}

/* static */ const char *VCalSetup::configGroup()
{
	return "VCal Conduit";
}

VCalSetup::VCalSetup(QWidget *parent) :
	setupDialog(parent,"vcalOptions",VCalConduit::version())
{
	FUNCTIONSETUP;
	KConfig *config=kapp->getConfig();
	config->setGroup(configGroup());
	addPage(new VCalSetupPage(this,config));
	addPage(new setupInfoPage(this,
		VCalConduit::version(),
		i18n("By D. Pilone, P. Brown & H.J. Steehouwer")
		));

	setupDialog::setupWidget();
}


