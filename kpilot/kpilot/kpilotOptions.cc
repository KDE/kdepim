/* kpilotOptions.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This file defines the dialog used by KPilot to setup
** the Pilot device etc.
**
** You can compile this with -DSTANDALONE to get an app
** that can modify KPilot's config files without anything else.
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

#include <stream.h>
#include <stdlib.h>

// for reading the passwd file
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>

#include <qlabel.h>
#include <qchkbox.h>
#include <qgrpbox.h>
#include <qbttngrp.h>
#include <qvbuttongroup.h>
#include <qradiobt.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qcstring.h>
#include <qlayout.h>


#include <klocale.h>
#include <kapp.h>
#include <kconfig.h>
#include <kwin.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <kio/netaccess.h>
#include <kstddirs.h>
#include <kdebug.h>

#ifndef STANDALONE
#include "kpilotConfig.h"
#endif


#include "kpilotOptions.moc"


static const char *kpilotoptions_id="$Id$";


static const char *daemondesktop="Utilities/kpilotdaemon.desktop";
static const char *daemonautostart="kpilotdaemon.desktop";

// --------------------------------------------------
//
// KPilotOptionsPrivacy
//
// Privacy and Secrecy settings
//
//
KPilotOptionsPrivacy::KPilotOptionsPrivacy(setupDialog *p,KConfig& c) :
	setupDialogPage(i18n("DB Specials"),p)
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
	fuseSecret->setChecked(c.readBoolEntry("ShowSecrets"));



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

	c.setGroup(QString::null);
	fBackupOnly->setText(c.readEntry("BackupForSync",
		"Arng,PmDB,lnch"));
	fSkipDB->setText(c.readEntry("SkipSync",
		"AvGo"));


	grid->setRowStretch(3,100);
	grid->activate();
}


int KPilotOptionsPrivacy::commitChanges(KConfig& config)
{
	FUNCTIONSETUP;

	config.setGroup(QString::null);

	config.writeEntry("ShowSecrets",(bool)fuseSecret->isChecked());
	config.writeEntry("BackupForSync",fBackupOnly->text());
	config.writeEntry("SkipSync",fSkipDB->text());

	return 0;
}


/* static */ QString KPilotOptionsAddress::fGroupName=
	QString("Address Widget");

KPilotOptionsAddress::KPilotOptionsAddress(setupDialog *w,KConfig& c) :
	setupDialogPage(i18n("Address"),w)
{
	FUNCTIONSETUP;

	QLabel *currentLabel;
	QVBoxLayout *vl;	// Layout for page as a whole
	QGridLayout *grid;	// Layout inside group box

	// The address widget uses a different group from all
	// the rest of the `standard' KPilot options -- perhaps
	// in preparation to making it a separate conduit entirely.
	//
	//
	c.setGroup(fGroupName);

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
	fIncomingFormat->setText(c.readEntry("IncomingFormat", 
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
	fOutgoingFormat->setText(c.readEntry("OutgoingFormat", 
		"%LN,%FN,%CO,%P1,%P2,%P3,%P4,%P5,"
		"%AD,%CI,%ST,%ZI,%CT,%TI,"
		"%C1,%C2,%C3,%C4"));
	fOutgoingFormat->resize(250, fIncomingFormat->height());
	currentLabel->setBuddy(fOutgoingFormat);
	grid->addWidget(fOutgoingFormat,2,1);

	fUseKeyField = new QCheckBox(i18n("Use &Key Field"), 
		formatGroup);
	fUseKeyField->adjustSize();
	fUseKeyField->setChecked(c.readBoolEntry("UseKeyField", false));
	grid->addWidget(fUseKeyField,3,1);


	formatGroup->adjustSize();
	vl->addWidget(formatGroup);




	displayGroup=new QVButtonGroup(i18n("Address Display"),
		this,"bg");

	fNormalDisplay=new QRadioButton(i18n("Last,First"),
		displayGroup);
	fNormalDisplay->adjustSize();

	fCompanyDisplay=new QRadioButton(i18n("Company,Last"),
		displayGroup);
	fCompanyDisplay->adjustSize();

	displayGroup->adjustSize();
	vl->addWidget(displayGroup);





	setRadio(c.readNumEntry("AddressDisplay",0));

}



	

int KPilotOptionsAddress::commitChanges(KConfig& c)
{
	FUNCTIONSETUP;

	c.setGroup(fGroupName);

	c.writeEntry("AddressDisplay",getRadio());
#ifdef DEBUG
	if (debug_level & UI_TEDIOUS) 
	{ 
		kdDebug() << fname 
			<< ": Selected display mode " 
			<< getRadio() << endl ; 
	}
#endif

	c.writeEntry("IncomingFormat", fIncomingFormat->text());
	c.writeEntry("OutgoingFormat", fOutgoingFormat->text());
	c.writeEntry("UseKeyField", (int)fUseKeyField->isChecked());

	return 0;
}

/* static */ int KPilotOptionsAddress::getDisplayMode(KConfig& c)
{
	FUNCTIONSETUP;

	c.setGroup(fGroupName);

	return c.readNumEntry("AddressDisplay",0);
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

KPilotOptionsGeneral::KPilotOptionsGeneral(setupDialog *w,KConfig& config) :
	setupDialogPage(i18n("General"),w)
{
	FUNCTIONSETUP;

	QLabel *currentLabel;
	int value;

	// t is used for extra storage while determining
	// if certain options (like autostart) are 
	// actually possible.
	//
	//
	QString t;

	// read the environment settings from the pilotlink setup
	// and use them, if there is no settings in the config file.
	// FIXME: if this dialog is cancelled the environment should still
	//        be read from the main/sync application
	// Currently the application doesn't start, if the user cancelles
	// the dialog. If the user doesn't cancel it,
	// the setting gets written in the config.
	// 04.Nov.2000 habenich
	char *envPilotPort = ::getenv("PILOTPORT");
	char *envPilotRate = ::getenv("PILOTRATE");
	int pilotRate = 0;
	if (envPilotRate) {
	  if (!strncmp(envPilotRate, "115200", 6)) {
	    pilotRate = 4;
	  }
	  else if (!strncmp(envPilotRate, "57600", 5)) {
	    pilotRate = 3;
	  }
	  else if (!strncmp(envPilotRate, "38400", 5)) {
	    pilotRate = 2;
	  }
	  else if (!strncmp(envPilotRate,"19200", 5)) {
	    pilotRate = 1;
	  }
	  // 9600 is default so no need to search on this
	  //else if (strncmp(envPilotRate, "9600", 4)) {
	  //  pilotRate = 0;
	  //}
	}
	QCString pilotPort;
	// this construct is only for security
	// of buffer overflow by environment vars being too long
	if (envPilotPort) {
	  // max length of the device name = "/dev/xxxyyyzzz" = 14 < 20
	  // change this length
	  // if the users get strange errors from environment settings
	  QCString tmp(envPilotPort, 20);
	  pilotPort = tmp;
	}
	else {
	  pilotPort = "/dev/pilot";
	}
	// look in the password file for the Real User Name.
	// if there is none, use the environment setting.
	// if there is none, use ""
	QCString userName("");
	struct passwd *myPasswdEntry = getpwuid(getuid());
	if (myPasswdEntry 
	    && myPasswdEntry->pw_gecos 
	    && strlen(myPasswdEntry->pw_gecos)>0 ) {
	  QCString tmp(myPasswdEntry->pw_gecos, 256);
	  userName = tmp;
	}
	else { // use the environment
	  char *envUser = ::getenv("USER");
	  if (envUser) {
	    QCString tmp(envUser,256);
	    userName = tmp;
	  }
	}

	const int labelCol=0;
	const int fieldCol=1;
	const int nRows=8;
	const int nCols=4;

	QGridLayout *grid=new QGridLayout(this,nRows,nCols,10);
	
	config.setGroup(QString::null);
	currentLabel = new QLabel(i18n("Pilot Device: "), this);
	currentLabel->adjustSize();

	fPilotDevice = new QLineEdit(this);
	fPilotDevice->setText(config.readEntry("PilotDevice", pilotPort));
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
	value=config.readNumEntry("PilotSpeed", pilotRate);

#ifdef DEBUG
	if (debug_level & UI_TEDIOUS)
	{
		kdDebug() << fname << ": Read pilot speed "
			<< value << " from config." << endl;
	}
#endif
	fPilotSpeed->setCurrentItem(value);
	grid->addWidget(currentLabel,0,labelCol+2);
	grid->addWidget(fPilotSpeed,0,fieldCol+2);

	currentLabel = new QLabel(i18n("Pilot User: "), this);
	currentLabel->adjustSize();
	fUserName = new QLineEdit(this);
	fUserName->setText(config.readEntry("UserName", userName));
	fUserName->resize(200, fUserName->height());
	grid->addWidget(currentLabel,1,labelCol);
	grid->addMultiCellWidget(fUserName,1,1,fieldCol,fieldCol+2);

	currentLabel = new QLabel(i18n("Startup Options:"), this);
	currentLabel->adjustSize();
	grid->addWidget(currentLabel,2,labelCol);

	fStartDaemonAtLogin = new QCheckBox(
		i18n("Start Hot-Sync Daemon at login. "), 
		this);
	fStartDaemonAtLogin->adjustSize();
	fStartDaemonAtLogin->setChecked(
		config.readBoolEntry("StartDaemonAtLogin", false));
	grid->addMultiCellWidget(fStartDaemonAtLogin,2,2,fieldCol,fieldCol+2);
	// Utilities/KPilotDaemon
	t = locate("apps",daemondesktop);
	if (t.isNull())
	{
		fStartDaemonAtLogin->setEnabled(false);
	}
			

	fStartKPilotAtHotSync = new QCheckBox(
		i18n("Start KPilot at Hot-Sync."), this);
	fStartKPilotAtHotSync->adjustSize();
	fStartKPilotAtHotSync->setChecked(
		config.readBoolEntry("StartKPilotAtHotSync", false));
	grid->addMultiCellWidget(fStartKPilotAtHotSync,3,3,fieldCol,fieldCol+2);

	fDockDaemon = new QCheckBox(
		i18n("Show Daemon in KPanel. "
			"(Only available with KWM.)"), 
				this);
	fDockDaemon->adjustSize();
	fDockDaemon->setChecked(config.readBoolEntry("DockDaemon", false));
	grid->addMultiCellWidget(fDockDaemon,4,4,fieldCol,fieldCol+2);

	fKillDaemonOnExit = new QCheckBox(
		i18n("Stop Daemon on exit"),this);
	fKillDaemonOnExit->setChecked(
		config.readBoolEntry("StopDaemonAtExit",false));
	grid->addMultiCellWidget(fKillDaemonOnExit,5,5,fieldCol,fieldCol+2);

	grid->activate();
}

KPilotOptionsGeneral::~KPilotOptionsGeneral()
{
	FUNCTIONSETUP;
}




int KPilotOptionsGeneral::commitChanges(KConfig& config)
{
	FUNCTIONSETUP;

	QString dest,src;

	// WARNING: shouldn't this be QCString with a limit to 256 chars ?
	dest = getenv("HOME");
	dest+="/Desktop/Autostart/";
	dest+=daemonautostart;

	src = locate("apps",daemondesktop);
	if (src.isNull())
	{
		if (fStartDaemonAtLogin->isEnabled() &&
			fStartDaemonAtLogin->isChecked())
		{
			KMessageBox::sorry(this,
				i18n("Can't find the KPilotDaemon link file\n"
					"needed to autostart the daemon.\n"
					"Autostart has been disabled."));
			fStartDaemonAtLogin->setChecked(false);
		}
	}

	config.setGroup(QString::null);
	config.writeEntry("PilotDevice", fPilotDevice->text());
	config.writeEntry("PilotSpeed", fPilotSpeed->currentItem());
	config.writeEntry("UserName", fUserName->text());
	config.writeEntry("StartDaemonAtLogin", 
		(bool)fStartDaemonAtLogin->isChecked());
	config.writeEntry("StopDaemonAtExit",
		(bool)fKillDaemonOnExit->isChecked());
	config.writeEntry("StartKPilotAtHotSync", 
		(bool)fStartKPilotAtHotSync->isChecked());
	config.writeEntry("DockDaemon", 
		(bool)fDockDaemon->isChecked());

	if (fStartDaemonAtLogin->isChecked())
	{
#ifdef DEBUG
		if (debug_level & UI_MAJOR)
		{
			kdDebug() << fname
				<< ": Copying autostart file"
				<< endl;
		}
		KIO::NetAccess::copy(src,dest);
#endif
	}
	else
	{
#ifdef DEBUG
		if (debug_level & UI_MAJOR)
		{
			kdDebug() << fname
				<< ": Deleting daemon autostart file (ignore errors)"
				<< endl;
		}
#endif
		KIO::NetAccess::del(dest);
	}

	return 0;
}



KPilotOptionsSync::KPilotOptionsSync(setupDialog *s,KConfig& config) :
	setupDialogPage(i18n("Sync"),s)
{
	FUNCTIONSETUP;

	QGridLayout *grid = new QGridLayout(this,5,3,SPACING);

	fSyncFiles = new QCheckBox(i18n("Sync &Files"), this);
	fSyncFiles->setChecked(config.readBoolEntry("SyncFiles", false));
	grid->addWidget(fSyncFiles,0,1);

	fOverwriteRemote = new QCheckBox(
		i18n("Local &overrides Pilot."), this);
	fOverwriteRemote->setChecked(
		config.readBoolEntry("OverwriteRemote", false));
	grid->addWidget(fOverwriteRemote,1,1);

	fForceFirstTime = new QCheckBox(
		i18n("Force first-time sync every time"),this);
	fForceFirstTime -> setChecked(
		config.readBoolEntry("ForceFirst",false));
	grid->addWidget(fForceFirstTime,2,1);

	fSyncLastPC = new QCheckBox(
		i18n("Do full backup when changing PCs"),this);
	fSyncLastPC -> setChecked(
		config.readBoolEntry("SyncLastPC",true));
	grid->addWidget(fSyncLastPC,3,1);

	fPreferFastSync = new QCheckBox(
		i18n("Prefer Fast-Sync to Hot-Sync"),this);
	fPreferFastSync -> setChecked(
		config.readBoolEntry("PreferFastSync",false));
	grid->addWidget(fPreferFastSync,4,1);

	grid->setRowStretch(5,100);
	grid->setColStretch(2,100);
	grid->addColSpacing(2,SPACING);
}


/* virtual */ int KPilotOptionsSync::commitChanges(KConfig& c)
{
	FUNCTIONSETUP;

	c.writeEntry("SyncFiles", (bool)fSyncFiles->isChecked());
	c.writeEntry("OverwriteRemote", 
		(bool)fOverwriteRemote->isChecked());
	c.writeEntry("ForceFirst",(bool)fForceFirstTime->isChecked());
	c.writeEntry("SyncLastPC",(bool)fSyncLastPC->isChecked());
	c.writeEntry("PreferFastSync",(bool)fPreferFastSync->isChecked());

	c.sync();

	return 0;

	/* NOTREACHED */
	(void) kpilotoptions_id;
}

// ----------------------------------------------------
//
// KPilotOptions
//
// The actual dialog that contains all the pages.

/* static */ int KPilotOptions::fConfigVersion = 401 ;

/* static */ bool KPilotOptions::isNewer(KConfig& c)
{
	FUNCTIONSETUP;

	int r=setupDialog::getConfigurationVersion(c);
	if (r<fConfigVersion)
	{
		kdWarning() << __FUNCTION__
			<< ": Old configuration file (version "
			<< r
			<< ") found." << endl;
	}
	return r < fConfigVersion ;
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


#ifdef STANDALONE
	KConfig& config = *(KGlobal::config());
#else
	KConfig& config=KPilotConfig::getConfig();
#endif

	addPage(new KPilotOptionsGeneral(this,config));
	addPage(new  KPilotOptionsAddress(this,config));
	addPage(new KPilotOptionsPrivacy(this,config));
	addPage(new KPilotOptionsSync(this,config));
#ifdef USE_STANDALONE
	addPage(new setupInfoPage(this));
#endif
	
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
		"http://www.slac.com/pilone/kpilot_home/");

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

// $Log$
// Revision 1.23  2001/03/04 11:23:04  adridg
// Changed for bug 21392
//
// Revision 1.22  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
// Revision 1.21  2001/02/19 12:31:23  adridg
// Removed broken bug-reporting address
//
// Revision 1.20  2001/02/08 08:13:44  habenich
// exchanged the common identifier "id" with source unique <sourcename>_id for --enable-final build
//
// Revision 1.19  2001/02/05 20:58:48  adridg
// Fixed copyright headers for source releases. No code changed
//
// Revision 1.18  2000/12/31 16:44:00  adridg
// Patched up the debugging stuff again
//
// Revision 1.17  2000/12/21 00:42:50  adridg
// Mostly debugging changes -- added FUNCTIONSETUP and more #ifdefs. KPilot should now compile -DNDEBUG or with DEBUG undefined
//
// Revision 1.16  2000/11/26 18:17:03  adridg
// Groundwork for FastSync
//
// Revision 1.15  2000/11/17 08:31:59  adridg
// Minor changes
//
// Revision 1.14  2000/11/14 23:03:28  adridg
// Feature creep: ForceFirst and SyncLastPC
//
// Revision 1.13  2000/11/14 06:32:26  adridg
// Ditched KDE1 stuff
//
// Revision 1.12  2000/11/10 08:32:33  adridg
// Fixed spurious config new() and delete()
//
// Revision 1.11  2000/11/04 14:04:38  habenich
// using pilot-link environment for speed and device\nusing passwd file entry for real name
//
// Revision 1.10  2000/10/26 10:10:09  adridg
// Many fixes
//
// Revision 1.9  2000/10/19 19:21:33  adridg
// Added decent error messages
//
// Revision 1.8  2000/08/01 06:48:15  adridg
// Ported autostart-daemon to KDE2
//
// Revision 1.7  2000/07/30 10:01:55  adridg
// Completed KDE2 layout
//
