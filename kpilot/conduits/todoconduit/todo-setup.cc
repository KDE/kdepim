// toto-setup.cc
//
// Copyright (C) 1998,1999 Preston Brown
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
// $Revision$

static const char *id="$Id$";



#include <qdir.h>
#include <qlabel.h>
#include <qpushbt.h>
#include <qchkbox.h>
#include <qlined.h>
#include <kapp.h>
#include <klocale.h>
#include <kfiledialog.h>
#include "todo-conduit.h"
#include "todo-setup.moc"
#include "kpilot.h"

/* virtual */ const char *TodoSetup::groupName()
{
	return configGroup();
}

/* static */ const char *TodoSetup::configGroup()
{
	return "Todo Conduit";
}

TodoSetup::TodoSetup(QWidget *parent)
  : setupDialog(parent,"totoOptions",TodoConduit::version())
{
	FUNCTIONSETUP;
	KConfig *config=kapp->getConfig();
	config->setGroup(configGroup());
	addPage(new TodoSetupPage(this,config));
	addPage(new setupInfoPage(this,
		TodoConduit::version(),
		i18n("By Preston Brown")
		));
	setupDialog::setupWidget();
}


/* virtual */ const char *TodoSetupPage::tabName()
{
	return "Todo File";
}

int TodoSetupPage::commitChanges(KConfig *config)
{
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


void TodoSetupPage::slotBrowse()
{
	FUNCTIONSETUP;

  QString fileName = KFileDialog::getOpenFileName(0L, "*.vcs");
  if(fileName.isNull()) return;
  fCalendarFile->setText(fileName);
}

TodoSetupPage::TodoSetupPage(setupDialog *parent,KConfig *config) :
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
