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
#include <qlayout.h>
#include <kapp.h>
#include <klocale.h>
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
	setupDialog(parent,VCalGroup,VCalConduit::version())
{
	FUNCTIONSETUP;
	KConfig& config=KPilotLink::getConfig(VCalGroup);
	addPage(new VCalSetupPage(this,config));
	addPage(new setupInfoPage(this));

	setupDialog::setupWidget();
}


