// kpilotOptions.cc
//
// Copyright (C) 1998-2000 Dan Pilone
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
// This is the version of kpilotOptions.cc for KDE 2 / KPilot 4.
//

// $Revision$



#include "options.h"

#include <stream.h>
#include <stdlib.h>

#include <qlabel.h>
#include <qchkbox.h>
#include <qgrpbox.h>
#include <qbttngrp.h>
#include <qradiobt.h>
#include <qlineedit.h>
#include <qcombobox.h>

#include <qlayout.h>


#include <klocale.h>
#include <kapp.h>
#include <kconfig.h>
#include <kwin.h>
#include <kmessagebox.h>
#include <kglobal.h>


#include "kpilotOptions.moc"


static char *id="$Id$";



// --------------------------------------------------
//
// KPilotOptionsPrivacy
//
// Privacy and Secrecy settings
//
//
KPilotOptionsPrivacy::KPilotOptionsPrivacy(setupDialog *p,KConfig *c) :
	setupDialogPage(i18n("DB Specials"),p,c)
{
	FUNCTIONSETUP;
	QLabel *l1,*l2;

	const int labelCol = 0;
	const int fieldCol = 1;
	const int nCols=2;
	const int nRows=3;


	QGridLayout *grid = new QGridLayout(this,nRows,nCols,10);

	fuseSecret=new QCheckBox(i18n("Show Secrets"),this);

	fuseSecret->adjustSize();
	grid->addWidget(fuseSecret,0,fieldCol);
	fuseSecret->setChecked(c->readNumEntry("ShowSecrets"));



	l1=new QLabel(i18n("Backup Only:"),this);
	fBackupOnly=new QLineEdit(this);
	l1->adjustSize();
	l1->setBuddy(fBackupOnly);
	grid->addWidget(l1,1,labelCol);
	grid->addWidget(fBackupOnly,1,fieldCol);

	l2=new QLabel(i18n("Skip:"),this);
	fSkipDB=new QLineEdit(this);
	l2->adjustSize();
	l2->setBuddy(fSkipDB);
	grid->addWidget(l2,2,labelCol);
	grid->addWidget(fSkipDB,2,fieldCol);

	c->setGroup(QString());
	fBackupOnly->setText(c->readEntry("BackupForSync",
		"Arng,PmDB,lnch"));
	fSkipDB->setText(c->readEntry("SkipSync",
		"AvGo"));


	grid->setRowStretch(3,100);
	grid->activate();
}


int KPilotOptionsPrivacy::commitChanges(KConfig *config)
{
	FUNCTIONSETUP;

	config->setGroup(QString::null);

	config->writeEntry("ShowSecrets",fuseSecret->isChecked());
	config->writeEntry("BackupForSync",fBackupOnly->text());
	config->writeEntry("SkipSync",fSkipDB->text());

	return 0;
}


/* static */ QString KPilotOptionsAddress::fGroupName=
	QString("Address Widget");

KPilotOptionsAddress::KPilotOptionsAddress(setupDialog *w,KConfig *c) :
	setupDialogPage(i18n("Address"),w,c)
{
	FUNCTIONSETUP;

	QLabel *currentLabel;
	QVBoxLayout *vl;	// Layout for page as a whole
	QGridLayout *grid;	// Layout inside group box
	QVBoxLayout *advl;	// Layout for address display

	// The address widget uses a different group from all
	// the rest of the `standard' KPilot options -- perhaps
	// in preparation to making it a separate conduit entirely.
	//
	//
	c->setGroup(fGroupName);

	vl=new QVBoxLayout(this,10);


	formatGroup=new QGroupBox(i18n("Address Formats"),this);
	grid=new QGridLayout(formatGroup,3,2,10);

	// Start adding elements from the second row 
	// (row 1) to allow space for the group box label.
	// This is probably a bug -- and doesn't happen
	// with VBox layouts.
	//
	//
	grid->addRowSpacing(0,10);
	currentLabel = new QLabel(i18n("Import Format:"),
		formatGroup);
	currentLabel->adjustSize();
	grid->addWidget(currentLabel,1,0);

	fIncomingFormat = new QLineEdit(formatGroup);
	fIncomingFormat->setText(c->readEntry("IncomingFormat", 
		"%LN,%FN,%CO,%P1,%P2,%P3,%P4,%P5,"
		"%AD,%CI,%ST,%ZI,%CT,%TI,"
		"%C1,%C2,%C3,%C4"));
	// fIncomingFormat->resize(250, fIncomingFormat->height());
	currentLabel->setBuddy(fIncomingFormat);
	grid->addWidget(fIncomingFormat,1,1);

	currentLabel = new QLabel(i18n("Export Format:"),
		formatGroup);
	currentLabel->adjustSize();
	grid->addWidget(currentLabel,2,0);

	fOutgoingFormat = new QLineEdit(formatGroup);
	fOutgoingFormat->setText(c->readEntry("OutgoingFormat", 
		"%LN,%FN,%CO,%P1,%P2,%P3,%P4,%P5,"
		"%AD,%CI,%ST,%ZI,%CT,%TI,"
		"%C1,%C2,%C3,%C4"));
	fOutgoingFormat->resize(250, fIncomingFormat->height());
	currentLabel->setBuddy(fOutgoingFormat);
	grid->addWidget(fOutgoingFormat,2,1);

	fUseKeyField = new QCheckBox(i18n("Use &Key Field"), 
		formatGroup);
	fUseKeyField->adjustSize();
	fUseKeyField->setChecked(c->readNumEntry("UseKeyField", 0));
	grid->addWidget(fUseKeyField,3,1);


	formatGroup->adjustSize();
	vl->addWidget(formatGroup);




	displayGroup=new QButtonGroup(i18n("Address Display"),
		this,"bg");
	advl=new QVBoxLayout(displayGroup,10);

	fNormalDisplay=new QRadioButton(i18n("Last,First"),
		displayGroup);
	fNormalDisplay->adjustSize();
	advl->addWidget(fNormalDisplay);

	fCompanyDisplay=new QRadioButton(i18n("Company,Last"),
		displayGroup);
	fCompanyDisplay->adjustSize();
	advl->addWidget(fCompanyDisplay);

	displayGroup->adjustSize();
	vl->addWidget(displayGroup);





	setRadio(c->readNumEntry("AddressDisplay",0));

}



	

int KPilotOptionsAddress::commitChanges(KConfig *c)
{
	FUNCTIONSETUP;

	c->setGroup(fGroupName);

	c->writeEntry("AddressDisplay",getRadio());
	if (debug_level & UI_TEDIOUS) { cerr << fname << 
		": Selected display mode " << getRadio() << '\n' ; }

	c->writeEntry("IncomingFormat", fIncomingFormat->text());
	c->writeEntry("OutgoingFormat", fOutgoingFormat->text());
	c->writeEntry("UseKeyField", (int)fUseKeyField->isChecked());

	return 0;
}

/* static */ int KPilotOptionsAddress::getDisplayMode(KConfig *c)
{
	FUNCTIONSETUP;

	if (c==NULL) c=KGlobal::config();
	if (c==NULL) return 0;

	c->setGroup(fGroupName);

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

KPilotOptionsGeneral::KPilotOptionsGeneral(setupDialog *w,KConfig *config) :
	setupDialogPage(i18n("General"),w,config)
{
	FUNCTIONSETUP;

	QLabel *currentLabel;
	int value;

	const int labelCol=0;
	const int fieldCol=1;
	const int nRows=8;
	const int nCols=4;

	QGridLayout *grid=new QGridLayout(this,nRows,nCols,10);
	
	config->setGroup(QString::null);
	currentLabel = new QLabel(i18n("Pilot Device: "), this);
	currentLabel->adjustSize();

	fPilotDevice = new QLineEdit(this);
	fPilotDevice->setText(config->readEntry("PilotDevice", "/dev/pilot"));
	fPilotDevice->resize(100, fPilotDevice->height());
	grid->addWidget(currentLabel,0,labelCol);
	grid->addWidget(fPilotDevice,0,fieldCol);

	currentLabel = new QLabel(i18n("Speed: "), this);
	currentLabel->adjustSize();
	fPilotSpeed = new QComboBox(this);
	fPilotSpeed->insertItem("9600");
	fPilotSpeed->insertItem("19200");
	fPilotSpeed->insertItem("38400");
	fPilotSpeed->insertItem("57600");
	fPilotSpeed->insertItem("115200");
	value=config->readNumEntry("PilotSpeed", 0);
	if (debug_level & UI_TEDIOUS)
	{
		cerr << fname << ": Read pilot speed "
			<< value << " from config." << endl;
	}
	fPilotSpeed->setCurrentItem(value);
	grid->addWidget(currentLabel,0,labelCol+2);
	grid->addWidget(fPilotSpeed,0,fieldCol+2);

	currentLabel = new QLabel(i18n("Pilot User: "), this);
	currentLabel->adjustSize();
	fUserName = new QLineEdit(this);
	QCString userName = ::getenv("USER");
	fUserName->setText(config->readEntry("UserName", userName));
	fUserName->resize(200, fUserName->height());
	grid->addWidget(currentLabel,1,labelCol);
	grid->addMultiCellWidget(fUserName,1,1,fieldCol,fieldCol+2);

	currentLabel = new QLabel(i18n("Sync Options:"), this);
	currentLabel->adjustSize();
	fSyncFiles = new QCheckBox(i18n("Sync &Files"), this);
	fSyncFiles->adjustSize();
	fSyncFiles->setChecked(config->readNumEntry("SyncFiles", 1));
	grid->addWidget(currentLabel,2,labelCol);
	grid->addMultiCellWidget(fSyncFiles,2,2,fieldCol,fieldCol+2);

	fOverwriteRemote = new QCheckBox(
		i18n("Local &overrides Pilot."), this);
	fOverwriteRemote->adjustSize();
	fOverwriteRemote->setChecked(
		config->readNumEntry("OverwriteRemote", 0));
	grid->addMultiCellWidget(fOverwriteRemote,3,3,fieldCol,fieldCol+2);

	fStartDaemonAtLogin = new QCheckBox(
		i18n("Start Hot-Sync Daemon at login. "
			"(Only available with KFM)"), this);
	fStartDaemonAtLogin->adjustSize();
	fStartDaemonAtLogin->setChecked(
		config->readNumEntry("StartDaemonAtLogin", 0));
	grid->addMultiCellWidget(fStartDaemonAtLogin,4,4,fieldCol,fieldCol+2);

// 	{
// 		KFM *fKFM=new KFM();
// 		if (fKFM==NULL)
// 		{
// 			cerr << fname << ": Can't allocate KFM object.\n";
// 		}

// 		if (fKFM==NULL || fKFM->isKFMRunning()!=true)
// 		{
// 			fStartDaemonAtLogin->setEnabled(false);
// 		}
// 		if (fKFM!=NULL)
// 		{
// 			delete fKFM;
// 			fKFM=NULL;
// 		}
// 	}
	connect(fStartDaemonAtLogin, SIGNAL(clicked()), 
		this, SLOT(slotSetupDaemon()));

	fStartKPilotAtHotSync = new QCheckBox(
		i18n("Start KPilot at Hot-Sync."), this);
	fStartKPilotAtHotSync->adjustSize();
	fStartKPilotAtHotSync->setChecked(
		config->readNumEntry("StartKPilotAtHotSync", 0));
	grid->addMultiCellWidget(fStartKPilotAtHotSync,5,5,fieldCol,fieldCol+2);

	fDockDaemon = new QCheckBox(
		i18n("Show Daemon in KPanel. "
			"(Only available with KWM.)"), 
				this);
	fDockDaemon->adjustSize();
	fDockDaemon->setChecked(config->readNumEntry("DockDaemon", 0));
	grid->addMultiCellWidget(fDockDaemon,6,6,fieldCol,fieldCol+2);

	{
// 		KWM* kwm = new KWM();
// 		if (kwm!=NULL)
// 		{
// 			if(kwm->isKWMInitialized() == false)
// 			fDockDaemon->setEnabled(false);
// 			delete kwm;
// 		}
	}

	grid->activate();
}

KPilotOptionsGeneral::~KPilotOptionsGeneral()
{
	FUNCTIONSETUP;
}




int KPilotOptionsGeneral::commitChanges(KConfig *config)
{
	FUNCTIONSETUP;

	config->setGroup(QString());
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
// 	FUNCTIONSETUP;

//   QString destDir = getenv("HOME");
  

//   if(fStartDaemonAtLogin->isChecked())
//     {
// 	if (fKFM==NULL) fKFM = new KFM;
// 	if (fKFM==NULL)
// 	{
// 		KMsgBox::message(this,i18n("Setup Daemon"),
// 			i18n("Cannot connect to KFM."),
// 			KMsgBox::STOP);
// 		return ;
// 	}

//       if(fKFM!=NULL && fKFM->isOK() == false)
// 	{
// 	  KMsgBox::message(0L, i18n("Setup Daemon"), 
// 			   i18n("Cannot connect to KFM."), 
// 			   KMsgBox::STOP);
// 	  delete fKFM;
// 	  fKFM = NULL;
// 	  return;
// 	}
//       connect(fKFM, SIGNAL(finished()), this, SLOT(slotKFMFinished()));
//       QString destDir = getenv("HOME");
//       fKFM->copy(kapp->kde_appsdir() + "/Utilities/KPilotDaemon.kdelnk", destDir + "/Desktop/Autostart/KPilotDaemon.kdelnk");
//       // memory leak..
//     }
//   else
//       {
// 	unlink(destDir + "/Desktop/Autostart/KPilotDaemon.kdelnk");
//       }
}


// ----------------------------------------------------
//
// KPilotOptions
//
// The actual dialog that contains all the pages.

/* static */ int KPilotOptions::fConfigVersion = 400 ;

/* static */ bool KPilotOptions::isNewer(KConfig *c)
{
	int r=setupDialog::getConfigurationVersion(c);
	return r<fConfigVersion;
}

KPilotOptions::KPilotOptions(QWidget* parent) :
	setupDialog(parent,
		QString::null,
		i18n("KPilot Options"),
#ifdef USE_STANDALONE
		FALSE		// quit() when closed
#else
		TRUE		// modal dialog
#endif
	)
{
	FUNCTIONSETUP;

	setConfigurationVersion(fConfigVersion);


	KConfig *config=KGlobal::config();

	addPage(new KPilotOptionsGeneral(this,config));
	addPage(new  KPilotOptionsAddress(this,config));
	addPage(new KPilotOptionsPrivacy(this,config));
	addPage(new setupInfoPage(this));
	
	setupDialog::setupWidget();
}

#ifdef USE_STANDALONE
#include <kaboutdata.h>
#include <kcmdlineargs.h>

int main(int argc, char **argv)
{
	debug_level=1023;

	KAboutData *a=new KAboutData("kpilot",
		I18N_NOOP("KPilot"),
		"4.0.0",
		I18N_NOOP("Pilot Hot-sync software for KDE"),
		KAboutData::License_GPL,
		"Copyright (c) 1998-2000 Dan Pilone",
		QString::null,
		"http://www.slac.com/pilone/kpilot_home/",
		"kpilot-list@slac.com");

	a->addAuthor("Dan Pilone",
		I18N_NOOP("Project Leader"),
		"pilone@slac.com");
	a->addAuthor("Adriaan de Groot",
		I18N_NOOP("KDE1 Maintainer"),
		"adridg@cs.kun.nl",
		"http://www.cs.kun.nl/~adridg/kpilot/");
	a->addCredit("Preston Brown",
		I18N_NOOP("KOrganizer Support"),
		"pbrown@kde.org");
	a->addCredit("Andreas Silberstorff",
		I18N_NOOP(".de localization"));
	a->addCredit("Nicolas Pettiaux",
		I18N_NOOP(".fr localization"));
	a->addCredit("Chuck Robey",
		I18N_NOOP("BSD porting"));
	a->addCredit("Christopher Molnar",
		I18N_NOOP("Moral and Mandrake Support"));
	a->addCredit("Scott Presnell",
		I18N_NOOP("Bug Fixes"));


	KCmdLineArgs::init(argc,argv,a);
	KApplication::addCmdLineOptions();

	KApplication app;
	QWidget *w=new KPilotOptions(0L);
	w->show();
	app.setMainWidget(w);

	return app.exec();

}
#endif

// $Log: $
