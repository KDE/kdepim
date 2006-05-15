/* KPilot
**
** Copyright (C) 2004 by Reinhold Kainhofer
**
** A simple configuration wizard.
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

static const char *conduitconfigwizard_id =
	"$Id$";

//#include "options.h"

#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qlineedit.h>

#include <kmessagebox.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfigskeleton.h>

#include "kpilotConfig.h"
#include "options.h"

#include "kpilotConfigWizard_app.h"
#include "kpilotConfigWizard_user.h"
#include "kpilotConfigWizard_address.h"
#include "kpilotConfigWizard_notes.h"
#include "kpilotConfigWizard_vcal.h"


#include "kpilotConfigWizard.moc"
#include "kpilotProbeDialog.h"


ConfigWizard::ConfigWizard(QWidget *parent, const char *n, int m) :
	KWizard(parent, n),
	fMode((Mode)m)
{
//	page1=new ConfigWizard_base1(this);
//	addPage( page1, i18n("Select Connection Type") );
	page2=new ConfigWizard_base2(this);
	addPage( page2, i18n("Pilot Info") );
	page3=new ConfigWizard_base3(this);
	addPage( page3, i18n("Application to Sync With") );
	setFinishEnabled( page3, true );
	
	setHelpEnabled( page2, false );
	setHelpEnabled( page3, false );

	connect( page2->fProbeButton, SIGNAL( pressed() ),
		this, SLOT( probeHandheld() ) );

	KPilotSettings::self()->readConfig();
	page2->fUserName->setText( KPilotSettings::userName() );
	page2->fDeviceName->setText( KPilotSettings::pilotDevice() );
	page2->fPilotRunningPermanently->setChecked( KPilotSettings::startDaemonAtLogin() );

	(void) conduitconfigwizard_id;
}

ConfigWizard::~ConfigWizard()
{
}

void ConfigWizard::accept()
{
	FUNCTIONSETUP;
	QString username( page2->fUserName->text() );
	QString devicename( page2->fDeviceName->text() );
//	int devicetype( page1->fConnectionType->selectedId() );
	enum eSyncApp {
		eAppKDE=0,
		//eAppKontact,
		eAppEvolution,
		eAppNone
	} app;
	app=(eSyncApp)( page3->fAppType->selectedId() );
	bool keepPermanently( page2->fPilotRunningPermanently->isChecked() );
#ifdef DEBUG
	DEBUGCONDUIT<<fname<<"Keep permanently: "<<keepPermanently<<endl;
#endif

	KPilotSettings::setPilotDevice( devicename );
	KPilotSettings::setUserName(username);
//	KPilotSettings::setEncoding("iso 8859-15");
	KPilotSettings::setDockDaemon( true );
	KPilotSettings::setKillDaemonAtExit( !keepPermanently);
	KPilotSettings::setQuitAfterSync( !keepPermanently );
	KPilotSettings::setStartDaemonAtLogin( keepPermanently );
	KPilotSettings::setSyncType(0);
	KPilotSettings::setFullSyncOnPCChange( true );
	KPilotSettings::setConflictResolution(0);
	if ( !mDBs.isEmpty() ) 
		KPilotSettings::setDeviceDBs( mDBs );

	KPilotWizard_vcalConfig*calendarConfig = new KPilotWizard_vcalConfig("Calendar");
	KPilotWizard_vcalConfig*todoConfig = new KPilotWizard_vcalConfig("ToDo");
	KPilotWizard_addressConfig*addressConfig = new KPilotWizard_addressConfig();
	KPilotWizard_notesConfig*notesConfig = new KPilotWizard_notesConfig();
	addressConfig->readConfig();
	notesConfig->readConfig();
	todoConfig->readConfig();
	calendarConfig->readConfig();

	QStringList conduits = KPilotSettings::installedConduits();
	int version(0);
#define APPEND_CONDUIT(a) if (!conduits.contains(a)) conduits.append(a)
	QString applicationName(i18n("general KDE-PIM"));
	APPEND_CONDUIT("internal_fileinstall");
	APPEND_CONDUIT("todo-conduit");
	APPEND_CONDUIT("vcal-conduit");
	switch (app) {
		case eAppEvolution:
			applicationName=i18n("Gnome's PIM suite", "Evolution");

			// TODO: Once the Evolution abook resource is finished, enable it...
			conduits.remove("abbrowser_conduit");
			// addressConfig->setDefaults();
			// addressConfig->setAddressbookType( KPilotWizard_addressConfig::eAbookResource );
			//// addressConfig->revertToDefault("ArchiveDeleted");
			//// addressConfig->revertToDefault("ConflictResolution");

			// nothing to do for knotes conduit yet (evolution doesn't have notes)
			conduits.remove("knotes-conduit");

			// the vcalconduits use the same config file, so set the correct groups
			version = calendarConfig->conduitVersion();
			calendarConfig->setDefaults();
			calendarConfig->setCalendarType( KPilotWizard_vcalConfig::eCalendarLocal );
			calendarConfig->setCalendarFile( "$HOME/evolution/local/Calendar/calendar.ics" );
			calendarConfig->setConduitVersion( version );
			version = todoConfig->conduitVersion();
			todoConfig->setDefaults();
			todoConfig->setCalendarType( KPilotWizard_vcalConfig::eCalendarLocal );
			todoConfig->setCalendarFile( "$HOME/evolution/local/Tasks/tasks.ics" );
			todoConfig->setConduitVersion( version );

			KMessageBox::information(this, i18n("KPilot cannot yet synchronize the addressbook with Evolution, so the addressbook conduit was disabled.\nWhen syncing the calendar or to-do list using KPilot please quit Evolution before the sync, otherwise you will lose data."), i18n("Restrictions with Evolution"));
			break;
		case eAppNone:
			conduits.clear();
			APPEND_CONDUIT("internal_fileinstall");
			applicationName=i18n("Kpilot will sync with nothing","nothing (it will backup only)");
			break;
//		case eAppKontact:
		case eAppKDE:
			applicationName=i18n("KDE's PIM suite", "Kontact");
		default:
			APPEND_CONDUIT("knotes-conduit");
			APPEND_CONDUIT("abbrowser_conduit");
			// Set to the stdaddressbook, reset others to defaults
			addressConfig->setAddressbookType( KPilotWizard_addressConfig::eAbookResource );
			addressConfig->setArchiveDeleted( true );
			addressConfig->setConflictResolution( -1 );
			// nothing to do for knotes conduit yet
			// notesConfig->set...
			// the vcalconduits use the same config file, so set the correct groups
			int version = calendarConfig->conduitVersion();
			calendarConfig->setDefaults();
			calendarConfig->setCalendarType( KPilotWizard_vcalConfig::eCalendarResource );
			calendarConfig->setConduitVersion( version );
			version = todoConfig->conduitVersion();
			todoConfig->setDefaults();
			todoConfig->setCalendarType( KPilotWizard_vcalConfig::eCalendarResource );
			todoConfig->setConduitVersion( version );
			break;
	}
	addressConfig->writeConfig();
	notesConfig->writeConfig();
	todoConfig->writeConfig();
	calendarConfig->writeConfig();

	KPILOT_DELETE(addressConfig);
	KPILOT_DELETE(notesConfig);
	KPILOT_DELETE(todoConfig);
	KPILOT_DELETE(calendarConfig);

	KPilotSettings::setInstalledConduits( conduits );
#undef APPEND_CONDUIT

	QString finishMessage = i18n("KPilot is now configured to sync with %1.").arg(applicationName);
	if (fMode == InDialog)
	{
		finishMessage.append(CSL1("\n"));
		finishMessage.append(i18n(
			"The remaining options in the config dialog are advanced options and can "
			"be used to fine-tune KPilot."));
	}

	KMessageBox::information(this, finishMessage,
		i18n("Automatic Configuration Finished"));
	KPilotSettings::self()->writeConfig();
	QDialog::accept();
}

void ConfigWizard::probeHandheld()
{
	if ( KMessageBox::warningContinueCancel( this, i18n("Please put the handheld "
			"in the cradle, press the hotsync button and click on \"Continue\".\n\nSome "
			"kernel versions (Linux 2.6.x) have problems with the visor kernel module "
			"(for Sony Clie devices). Running an autodetection in that case might block "
			"the computer from doing hotsyncs until it is rebooted. In that case it might "
			"be advisable not to continue."), 
			i18n("Handheld Detection") ) == KMessageBox::Continue ) {
		ProbeDialog *probeDialog = new ProbeDialog( this );
		if ( probeDialog->exec() && probeDialog->detected() ) {
			page2->fUserName->setText( probeDialog->userName() );
			page2->fDeviceName->setText( probeDialog->device() );
			mDBs = probeDialog->dbs();
		}
		KPILOT_DELETE(probeDialog);
	}
}

