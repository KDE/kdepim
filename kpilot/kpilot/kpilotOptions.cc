// kpilotOptions.cc
//
// Copyright (C) 1998,1999 Dan Pilone
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 



// REVISION HISTORY 
//
// 3.1b9	By Dan Pilone
// 3.1b10	By Adriaan de Groot: comments added all over the place.
//		Modified KFM connection code, hopefully preventing
//		a crash.
//
//		Remaining questions are marked with QADE.

#include <stream.h>
#include <qlabel.h>
#include <kapp.h>
#include <kconfig.h>
#include <stdlib.h>
#include <kwm.h>
#include <kfm.h>
#include <kmsgbox.h>
#include "kpilotOptions.moc"
#include "kpilot.h"

// --------------------------------------------------
//
// KPilotOptionsPage
//
// General (abstract) behavior for pages in the options dialog.
//
//
KPilotOptionsPage::KPilotOptionsPage(QWidget *parent, KConfig *config) :
	QWidget(parent)
{
	FUNCTIONSETUP;

	config->setGroup(groupName());
}

KPilotOptionsPage::~KPilotOptionsPage()
{
	FUNCTIONSETUP;
}



const char *KPilotOptionsPage::groupName() const
{
	FUNCTIONSETUP;

	return NULL;
}


void KPilotOptionsPage::cancelChanges()
{
	FUNCTIONSETUP;
}

// --------------------------------------------------
//
// KPilotOptionsPrivacy
//
// Privacy and Secrecy settings
//
//
KPilotOptionsPrivacy::KPilotOptionsPrivacy(QWidget *p,KConfig *c) :
	KPilotOptionsPage(p,c)
{
	FUNCTIONSETUP;

	fuseSecret=new QCheckBox(klocale->translate("Show Secrets: "),this);

	if (fuseSecret!=NULL)
	{
		fuseSecret->adjustSize();
		fuseSecret->move(10,12);
		fuseSecret->setChecked(c->readNumEntry("ShowSecrets"));
	}
}

KPilotOptionsPrivacy::~KPilotOptionsPrivacy()
{
	FUNCTIONSETUP;

	if (fuseSecret!=NULL)
	{
		delete fuseSecret;
		fuseSecret=NULL;
	}
}


void KPilotOptionsPrivacy::commitChanges(KConfig *config)
{
	FUNCTIONSETUP;

	config->setGroup(NULL);
	if (fuseSecret!=NULL)
	{
		config->writeEntry("ShowSecrets",fuseSecret->isChecked());
	}
}

#define BELOW(a)	(a->y()+a->height()+SPACING)

KPilotOptionsAddress::KPilotOptionsAddress(QWidget *w,KConfig *c) :
	KPilotOptionsPage(w,c)
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


KPilotOptionsAddress::~KPilotOptionsAddress()
{
	FUNCTIONSETUP;

}

const char *KPilotOptionsAddress::theGroupName()
{
	return "Address Widget";
}
	
const char * KPilotOptionsAddress::groupName() const
{
	return theGroupName();
}

void KPilotOptionsAddress::commitChanges(KConfig *c)
{
	FUNCTIONSETUP;

	c->setGroup(groupName());

	c->writeEntry("AddressDisplay",getRadio());
	if (debug_level>TEDIOUS) { cerr << fname << 
		": Selected display mode " << getRadio() << '\n' ; }

	c->writeEntry("IncomingFormat", fIncomingFormat->text());
	c->writeEntry("OutgoingFormat", fOutgoingFormat->text());
	c->writeEntry("UseKeyField", (int)fUseKeyField->isChecked());
}

/* static */ int KPilotOptionsAddress::getDisplayMode(KConfig *c)
{
	FUNCTIONSETUP;

	if (c==NULL) c=kapp->getConfig();
	if (c==NULL) return 0;

	c->setGroup(theGroupName());

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


// --------------------------------------------------
//
// KPilotOptionsGeneral
//
// General (hardware) settings. (UNFINISHED)
//
//

KPilotOptionsGeneral::KPilotOptionsGeneral(QTabDialog *w,KConfig *config,
	char *name) :
	KPilotOptionsPage(w,config)
{
	FUNCTIONSETUP;

	QLabel *currentLabel;

	currentLabel = new QLabel(klocale->translate("Pilot Device: "), this);
	currentLabel->adjustSize();
	currentLabel->move(10, 12);
	fPilotDevice = new QLineEdit(this);
	fPilotDevice->setText(config->readEntry("PilotDevice", "/dev/pilot"));
	fPilotDevice->resize(100, fPilotDevice->height());
	fPilotDevice->move(110,10);

	currentLabel = new QLabel(klocale->translate("Speed: "), this);
	currentLabel->adjustSize();
	currentLabel->move(220, 12);
	fPilotSpeed = new QComboBox(this);
	fPilotSpeed->insertItem("9600");
	fPilotSpeed->insertItem("19200");
	fPilotSpeed->insertItem("38400");
	fPilotSpeed->insertItem("57600");
	fPilotSpeed->insertItem("115200");
	fPilotSpeed->setCurrentItem(config->readNumEntry("PilotSpeed", 0));
	fPilotSpeed->move(280, 10);

	currentLabel = new QLabel(klocale->translate("Pilot User: "), this);
	currentLabel->adjustSize();
	currentLabel->move(10, 52);
	fUserName = new QLineEdit(this);
	fUserName->setText(config->readEntry("UserName", "$USER"));
	fUserName->resize(200, fUserName->height());
	fUserName->move(110, 50);

	currentLabel = new QLabel(klocale->translate("Sync Options:"), this);
	currentLabel->adjustSize();
	currentLabel->move(10, 120);
	fSyncFiles = new QCheckBox(klocale->translate("Sync &Files"), this);
	fSyncFiles->adjustSize();
	fSyncFiles->setChecked(config->readNumEntry("SyncFiles", 1));
	fSyncFiles->move(110, 120);

	fOverwriteRemote = new QCheckBox(
		klocale->translate("Local &overrides Pilot."), this);
	fOverwriteRemote->adjustSize();
	fOverwriteRemote->setChecked(
		config->readNumEntry("OverwriteRemote", 0));
	fOverwriteRemote->move(110, 140);

	fStartDaemonAtLogin = new QCheckBox(
		klocale->translate("Start Hot-Sync Daemon at login. "
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
		klocale->translate("Start KPilot at Hot-Sync."), this);
	fStartKPilotAtHotSync->adjustSize();
	fStartKPilotAtHotSync->setChecked(
		config->readNumEntry("StartKPilotAtHotSync", 0));
	fStartKPilotAtHotSync->move(10, 200);

	fDockDaemon = new QCheckBox(
		klocale->translate("Show Daemon in KPanel. "
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


	w->addTab(this,klocale->translate(name));
}


KPilotOptionsGeneral::~KPilotOptionsGeneral()
{
	FUNCTIONSETUP;

}


void KPilotOptionsGeneral::commitChanges(KConfig *config)
{
	FUNCTIONSETUP;

	config->setGroup(groupName());
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

// ----------------------------------------------------
//
// KPilotOptions
//
// The actual dialog that contains all the pages.

KPilotOptions::KPilotOptions(QWidget* parent)
  : QTabDialog(parent, 
  	//"optionsDialog", 
	klocale->translate("KPilot Options"),
	true)
{
	FUNCTIONSETUP;

	resize(400, 400);
	setCancelButton();
	setupWidget();
	setCaption(klocale->translate("KPilot Options"));
	connect(this, SIGNAL(applyButtonPressed()), 
		this, SLOT(commitChanges()));
	connect(this, SIGNAL(cancelButtonPressed()), 
		this, SLOT(cancelChanges()));
}

KPilotOptions::~KPilotOptions()
{
	FUNCTIONSETUP;
}

void
KPilotOptions::commitChanges()
{
	FUNCTIONSETUP;

    KConfig* config = kapp->getConfig();

	if (generalPage!=NULL)
	{
		generalPage->commitChanges(config);
	}

	if (privacyPage!=NULL)
	{
		privacyPage->commitChanges(config);
	}

	if (addressPage!=NULL)
	{
		addressPage->commitChanges(config);
	}

	config->sync(); // So that the daemon can get at the changes.

	setResult(1);
}

void
KPilotOptions::cancelChanges()
{
	FUNCTIONSETUP;

	setResult(0);
}



void
KPilotOptions::setupWidget()
{
FUNCTIONSETUP;

KConfig* config = kapp->getConfig();

config->setGroup(0L);

generalPage=new KPilotOptionsGeneral(this,config,"&General");

#if 0
currentLabel = new QLabel(klocale->translate("%FN = First Name"), currentWidget);
currentLabel->setFont(QFont("fixed", 10));
currentLabel->adjustSize();
currentLabel->move(25, 100);

currentLabel = new QLabel(klocale->translate("%LN = Last Name"), currentWidget);
currentLabel->setFont(QFont("fixed", 10));
currentLabel->adjustSize();
currentLabel->move(25, 120);

currentLabel = new QLabel(klocale->translate("%CO = Company"), currentWidget);
currentLabel->setFont(QFont("fixed", 10));
currentLabel->adjustSize();
currentLabel->move(25, 140);

currentLabel = new QLabel(klocale->translate("%P1 = Phone Number 1"), currentWidget);
currentLabel->setFont(QFont("fixed", 10));
currentLabel->adjustSize();
currentLabel->move(25, 160);

currentLabel = new QLabel(klocale->translate("%P2 = Phone Number 2"), currentWidget);
currentLabel->setFont(QFont("fixed", 10));
currentLabel->adjustSize();
currentLabel->move(25, 180);

currentLabel = new QLabel(klocale->translate("%P3 = Phone Number 3"), currentWidget);
currentLabel->setFont(QFont("fixed", 10));
currentLabel->adjustSize();
currentLabel->move(25, 200);

currentLabel = new QLabel(klocale->translate("%P4 = Phone Number 4"), currentWidget);
currentLabel->setFont(QFont("fixed", 10));
currentLabel->adjustSize();
currentLabel->move(25, 220);

currentLabel = new QLabel(klocale->translate("%P5 = Phone 5 (E-mail)"), currentWidget);
currentLabel->setFont(QFont("fixed", 10));
currentLabel->adjustSize();
currentLabel->move(25, 240);

currentLabel = new QLabel(klocale->translate("%AD = Address"), currentWidget);
currentLabel->setFont(QFont("fixed", 10));
currentLabel->adjustSize();
currentLabel->move(25, 260);

currentLabel = new QLabel(klocale->translate("%CI = City"), currentWidget);
currentLabel->setFont(QFont("fixed", 10));
currentLabel->adjustSize();
currentLabel->move(25, 280);

currentLabel = new QLabel(klocale->translate("%ST = State"), currentWidget);
currentLabel->setFont(QFont("fixed", 10));
currentLabel->adjustSize();
currentLabel->move(225, 100);

currentLabel = new QLabel(klocale->translate("%ZI = Zip Code"), currentWidget);
currentLabel->setFont(QFont("fixed", 10));
currentLabel->adjustSize();
currentLabel->move(225, 120);

currentLabel = new QLabel(klocale->translate("%CT = Country"), currentWidget);
currentLabel->setFont(QFont("fixed", 10));
currentLabel->adjustSize();
currentLabel->move(225, 140);

currentLabel = new QLabel(klocale->translate("%TI = Title"), currentWidget);
currentLabel->setFont(QFont("fixed", 10));
currentLabel->adjustSize();
currentLabel->move(225, 160);

currentLabel = new QLabel(klocale->translate("%C1 = Custom 1"), currentWidget);
currentLabel->setFont(QFont("fixed", 10));
currentLabel->adjustSize();
currentLabel->move(225, 180);

currentLabel = new QLabel(klocale->translate("%C2 = Custom 2"), currentWidget);
currentLabel->setFont(QFont("fixed", 10));
currentLabel->adjustSize();
currentLabel->move(225, 200);

currentLabel = new QLabel(klocale->translate("%C3 = Custom 3"), currentWidget);
currentLabel->setFont(QFont("fixed", 10));
currentLabel->adjustSize();
currentLabel->move(225, 220);

currentLabel = new QLabel(klocale->translate("%C4 = Custom 4"), currentWidget);
currentLabel->setFont(QFont("fixed", 10));
currentLabel->adjustSize();
currentLabel->move(225, 240);

//     currentLabel = new QLabel("%NO = Note", currentWidget);
//     currentLabel->setFont(QFont("fixed", 10));
//     currentLabel->adjustSize();
//     currentLabel->move(225, 260);

currentLabel = new QLabel(klocale->translate("%IG = Ignore (used\n          for import)"), currentWidget);
currentLabel->setFont(QFont("fixed", 10));
currentLabel->adjustSize();
currentLabel->move(225, 260);


addTab(currentWidget, klocale->translate("A&ddress Settings"));
#endif

addressPage=new KPilotOptionsAddress(this,config);
if (addressPage!=NULL)
{
addTab(addressPage,klocale->translate("&Address"));
}
privacyPage=new KPilotOptionsPrivacy(this,config);
if (privacyPage!=NULL)
{
addTab(privacyPage,klocale->translate("&Privacy"));
}
}
