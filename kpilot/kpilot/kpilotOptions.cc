// kpilotOptions.cc
//
// Copyright (C) 1998,1999 Dan Pilone
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 

// $Revision$


// REVISION HISTORY 
//
// 3.1b9	By Dan Pilone
// 3.1b10	By Adriaan de Groot: comments added all over the place.
//		Modified KFM connection code, hopefully preventing
//		a crash.
//
//		Remaining questions are marked with QADE.

#include <stream.h>
#include <stdlib.h>
#include <qlabel.h>
#include <qchkbox.h>
#include <qgrpbox.h>
#include <qbttngrp.h>
#include <qradiobt.h>

#include <kapp.h>
#include <kconfig.h>
#include <kwm.h>
#include <kfm.h>
#include <kmsgbox.h>


#include "kpilotOptions.moc"
#include "kpilot.h"


static char *id="$Id$";

// --------------------------------------------------
//
// KPilotOptionsPrivacy
//
// Privacy and Secrecy settings
//
//
KPilotOptionsPrivacy::KPilotOptionsPrivacy(setupDialog *p,KConfig *c) :
	setupDialogPage(p,c)
{
	FUNCTIONSETUP;

	fuseSecret=new QCheckBox(i18n("Show Secrets"),this);

	if (fuseSecret)
	{
		fuseSecret->adjustSize();
		fuseSecret->move(10,12);
		c->setGroup(0L);
		fuseSecret->setChecked(c->readNumEntry("ShowSecrets"));
	}

	if (debug_level>TEDIOUS)
	{
		cerr << fname << ": Read value "
			<< c->readNumEntry("ShowSecrets")
			<< " from file." << endl;
		cerr << fname << ": Checkbox value "
			<< fuseSecret->isChecked() << endl;
	}
}


int KPilotOptionsPrivacy::commitChanges(KConfig *config)
{
	FUNCTIONSETUP;

	if (fuseSecret)
	{
		config->setGroup(0L);
		config->writeEntry("ShowSecrets",fuseSecret->isChecked());

		if (debug_level>TEDIOUS)
		{
			cerr << fname << ": Wrote value " 
				<< fuseSecret->isChecked()
				<< " to file." << endl;
			cerr << fname << ": Got value "
				<< config->readNumEntry("ShowSecrets")
				<< " from file." << endl;
		}
	}

	return 0;
}

/* virtual */ const char *KPilotOptionsPrivacy::tabName()
{
	return i18n("Privacy");
}


KPilotOptionsAddress::KPilotOptionsAddress(setupDialog *w,KConfig *c) :
	setupDialogPage(w,c)
{
	FUNCTIONSETUP;

	c->setGroup(groupName());

	QLabel *currentLabel;

	formatGroup=new QGroupBox(klocale->translate("Address Formats"),this);
	currentLabel = new QLabel(klocale->translate("Import Format:"),
		formatGroup);
	currentLabel->adjustSize();
	currentLabel->move(10, 20);

	fIncomingFormat = new QLineEdit(formatGroup);
	fIncomingFormat->setText(c->readEntry("IncomingFormat", 
		"%LN,%FN,%CO,%P1,%P2,%P3,%P4,%P5,"
		"%AD,%CI,%ST,%ZI,%CT,%TI,"
		"%C1,%C2,%C3,%C4"));
	fIncomingFormat->resize(250, fIncomingFormat->height());
	fIncomingFormat->move(110, 10);

	currentLabel = new QLabel(klocale->translate("Export Format:"),
		formatGroup);
	currentLabel->adjustSize();
	currentLabel->move(10, 55);

	fOutgoingFormat = new QLineEdit(formatGroup);
	fOutgoingFormat->setText(c->readEntry("OutgoingFormat", 
		"%LN,%FN,%CO,%P1,%P2,%P3,%P4,%P5,"
		"%AD,%CI,%ST,%ZI,%CT,%TI,"
		"%C1,%C2,%C3,%C4"));
	fOutgoingFormat->resize(250, fIncomingFormat->height());
	fOutgoingFormat->move(110, 50);

	fUseKeyField = new QCheckBox(klocale->translate("Use &Key Field"), 
		formatGroup);
	fUseKeyField->adjustSize();
	fUseKeyField->setChecked(c->readNumEntry("UseKeyField", 0));
	fUseKeyField->move(150,BELOW(fOutgoingFormat));


	formatGroup->adjustSize();
	formatGroup->move(10,10);



	displayGroup=new QButtonGroup(klocale->translate("Address Display"),
		this,"bg");
	displayGroup->move(10,BELOW(formatGroup));

	fNormalDisplay=new QRadioButton(klocale->translate("Last,First"),
		displayGroup);
	fCompanyDisplay=new QRadioButton(klocale->translate("Company,Last"),
		displayGroup);

	fNormalDisplay->move(10,20);
	fNormalDisplay->adjustSize();

	fCompanyDisplay->adjustSize();
	fCompanyDisplay->move(10,BELOW(fNormalDisplay));

	displayGroup->adjustSize();






	setRadio(c->readNumEntry("AddressDisplay",0));
}



	
/* static */ const char * KPilotOptionsAddress::groupName()
{
	return "Address Widget";
}

int KPilotOptionsAddress::commitChanges(KConfig *c)
{
	FUNCTIONSETUP;

	c->setGroup(groupName());

	c->writeEntry("AddressDisplay",getRadio());
	if (debug_level>TEDIOUS) { cerr << fname << 
		": Selected display mode " << getRadio() << '\n' ; }

	c->writeEntry("IncomingFormat", fIncomingFormat->text());
	c->writeEntry("OutgoingFormat", fOutgoingFormat->text());
	c->writeEntry("UseKeyField", (int)fUseKeyField->isChecked());

	return 0;
}

/* static */ int KPilotOptionsAddress::getDisplayMode(KConfig *c)
{
	FUNCTIONSETUP;

	if (c==NULL) c=kapp->getConfig();
	if (c==NULL) return 0;

	c->setGroup(groupName());

	return c->readNumEntry("AddressDisplay",0);
}

void KPilotOptionsAddress::setRadio(int s)
{
	FUNCTIONSETUP;

	switch(s)
	{
		case 0 : fNormalDisplay->setChecked(1); break;
		case 1 : fCompanyDisplay->setChecked(1); break;
		default : fNormalDisplay->setChecked(1); break;
	}
}

int KPilotOptionsAddress::getRadio() const
{
	FUNCTIONSETUP;

	if (fNormalDisplay->isChecked()) return 0;
	if (fCompanyDisplay->isChecked()) return 1;

	return 0;
}

/* virtual */ const char *KPilotOptionsAddress::tabName()
{
	return i18n("Address");
}

// --------------------------------------------------
//
// KPilotOptionsGeneral
//
// General (hardware) settings. (UNFINISHED)
//
//

KPilotOptionsGeneral::KPilotOptionsGeneral(setupDialog *w,KConfig *config) :
	setupDialogPage(w,config),
	fKFM(0L)
{
	FUNCTIONSETUP;

	QLabel *currentLabel;
	int value;

	config->setGroup(0L);
	currentLabel = new QLabel(i18n("Pilot Device: "), this);
	currentLabel->adjustSize();
	currentLabel->move(10, 12);
	fPilotDevice = new QLineEdit(this);
	fPilotDevice->setText(config->readEntry("PilotDevice", "/dev/pilot"));
	fPilotDevice->resize(100, fPilotDevice->height());
	fPilotDevice->move(110,10);

	currentLabel = new QLabel(i18n("Speed: "), this);
	currentLabel->adjustSize();
	currentLabel->move(220, 12);
	fPilotSpeed = new QComboBox(this);
	fPilotSpeed->insertItem("9600");
	fPilotSpeed->insertItem("19200");
	fPilotSpeed->insertItem("38400");
	fPilotSpeed->insertItem("57600");
	fPilotSpeed->insertItem("115200");
	value=config->readNumEntry("PilotSpeed", 0);
	if (debug_level>TEDIOUS)
	{
		cerr << fname << ": Read pilot speed "
			<< value << " from config." << endl;
	}
	fPilotSpeed->setCurrentItem(value);
	fPilotSpeed->move(280, 10);

	currentLabel = new QLabel(i18n("Pilot User: "), this);
	currentLabel->adjustSize();
	currentLabel->move(10, 52);
	fUserName = new QLineEdit(this);
	fUserName->setText(config->readEntry("UserName", "$USER"));
	fUserName->resize(200, fUserName->height());
	fUserName->move(110, 50);

	currentLabel = new QLabel(i18n("Sync Options:"), this);
	currentLabel->adjustSize();
	currentLabel->move(10, 120);
	fSyncFiles = new QCheckBox(i18n("Sync &Files"), this);
	fSyncFiles->adjustSize();
	fSyncFiles->setChecked(config->readNumEntry("SyncFiles", 1));
	fSyncFiles->move(110, 120);

	fOverwriteRemote = new QCheckBox(
		i18n("Local &overrides Pilot."), this);
	fOverwriteRemote->adjustSize();
	fOverwriteRemote->setChecked(
		config->readNumEntry("OverwriteRemote", 0));
	fOverwriteRemote->move(110, 140);

	fStartDaemonAtLogin = new QCheckBox(
		i18n("Start Hot-Sync Daemon at login. "
			"(Only available with KFM)"), this);
	fStartDaemonAtLogin->adjustSize();
	fStartDaemonAtLogin->setChecked(
		config->readNumEntry("StartDaemonAtLogin", 0));
	fStartDaemonAtLogin->move(10, 180);

	{
		KFM *fKFM=new KFM();
		if (fKFM==NULL)
		{
			cerr << fname << ": Can't allocate KFM object.\n";
		}

		if (fKFM==NULL || fKFM->isKFMRunning()!=true)
		{
			fStartDaemonAtLogin->setEnabled(false);
		}
		if (fKFM!=NULL)
		{
			delete fKFM;
			fKFM=NULL;
		}
	}
	connect(fStartDaemonAtLogin, SIGNAL(clicked()), 
		this, SLOT(slotSetupDaemon()));

	fStartKPilotAtHotSync = new QCheckBox(
		i18n("Start KPilot at Hot-Sync."), this);
	fStartKPilotAtHotSync->adjustSize();
	fStartKPilotAtHotSync->setChecked(
		config->readNumEntry("StartKPilotAtHotSync", 0));
	fStartKPilotAtHotSync->move(10, 200);

	fDockDaemon = new QCheckBox(
		i18n("Show Daemon in KPanel. "
			"(Only available with KWM.)"), 
				this);
	fDockDaemon->adjustSize();
	fDockDaemon->setChecked(config->readNumEntry("DockDaemon", 0));
	fDockDaemon->move(10, 220);

	{
		KWM* kwm = new KWM();
		if (kwm!=NULL)
		{
			if(kwm->isKWMInitialized() == false)
			fDockDaemon->setEnabled(false);
			delete kwm;
		}
	}


}

KPilotOptionsGeneral::~KPilotOptionsGeneral()
{
	FUNCTIONSETUP;

	if (fKFM)
	{
		delete fKFM;
		fKFM=0L;
	}
}




int KPilotOptionsGeneral::commitChanges(KConfig *config)
{
	FUNCTIONSETUP;

	config->setGroup(0L);
	config->writeEntry("PilotDevice", fPilotDevice->text());
	config->writeEntry("PilotSpeed", fPilotSpeed->currentItem());
	config->writeEntry("UserName", fUserName->text());
	config->writeEntry("SyncFiles", (int)fSyncFiles->isChecked());
	config->writeEntry("OverwriteRemote", 
		(int)fOverwriteRemote->isChecked());
	config->writeEntry("StartDaemonAtLogin", 
		(int)fStartDaemonAtLogin->isChecked());
	config->writeEntry("StartKPilotAtHotSync", 
		(int)fStartKPilotAtHotSync->isChecked());
	config->writeEntry("DockDaemon", (int)fDockDaemon->isChecked());

	return 0;
}

void
KPilotOptionsGeneral::slotSetupDaemon()
{
	FUNCTIONSETUP;

  QString destDir = getenv("HOME");
  

  if(fStartDaemonAtLogin->isChecked())
    {
	if (fKFM==NULL) fKFM = new KFM;
	if (fKFM==NULL)
	{
		KMsgBox::message(this,klocale->translate("Setup Daemon"),
			klocale->translate("Cannot connect to KFM."),
			KMsgBox::STOP);
		return ;
	}

      if(fKFM!=NULL && fKFM->isOK() == false)
	{
	  KMsgBox::message(0L, klocale->translate("Setup Daemon"), 
			   klocale->translate("Cannot connect to KFM."), 
			   KMsgBox::STOP);
	  delete fKFM;
	  fKFM = NULL;
	  return;
	}
      connect(fKFM, SIGNAL(finished()), this, SLOT(slotKFMFinished()));
      QString destDir = getenv("HOME");
      fKFM->copy(kapp->kde_appsdir() + "/Utilities/KPilotDaemon.kdelnk", destDir + "/Desktop/Autostart/KPilotDaemon.kdelnk");
      // memory leak..
    }
  else
      {
	unlink(destDir + "/Desktop/Autostart/KPilotDaemon.kdelnk");
      }
}


void KPilotOptionsGeneral::slotKFMFinished()
{
	FUNCTIONSETUP;

	if (fKFM!=NULL)
	{
		delete fKFM;
		fKFM=NULL;
	}
}



/* virtual */ const char *KPilotOptionsGeneral::tabName()
{
	return "General";
}

// ----------------------------------------------------
//
// KPilotOptions
//
// The actual dialog that contains all the pages.

KPilotOptions::KPilotOptions(QWidget* parent) :
	setupDialog(parent,
		"optionsDialog",
		i18n("KPilot Options"),
		TRUE) 				// modal dialog
{
	FUNCTIONSETUP;
	KConfig *config=kapp->getConfig();

	addPage(new KPilotOptionsGeneral(this,config));
	addPage(new  KPilotOptionsAddress(this,config));
	addPage(new KPilotOptionsPrivacy(this,config));
	addPage(new setupInfoPage(this,
		KPilotInstaller::version(0),
		KPilotInstaller::authors()));
	
	setupDialog::setupWidget();
}


