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
#include <qlayout.h>
#include <kapp.h>
#include <klocale.h>
#include <kfiledialog.h>
#include "todo-conduit.h"
#include "todo-setup.moc"
#include "kpilot.h"


/* static */ const QString TodoSetup::TodoGroup("todoOptions");

TodoSetup::TodoSetup(QWidget *parent)
  : setupDialog(parent,TodoGroup,TodoConduit::version())
{
	FUNCTIONSETUP;
	KConfig& config=KPilotLink::getConfig(TodoGroup);
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
