/* conduitConfigDialog.cc                KPilot
**
** Copyright (C) 2004 by Dan Pilone
** Written 2004 by Reinhold Kainhofer
**
** This file defines a .ui-based configuration dialog for conduits.
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

static const char *conduitconfigdialog_id =
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

#include "kpilotConfigWizard_base1.h"
#include "kpilotConfigWizard_base2.h"
#include "kpilotConfigWizard_base3.h"

#include "kpilotConfigWizard.moc"
#include "kpilotProbeDialog.h"


ConfigWizard::ConfigWizard(QWidget *parent, const char *n, int m) :
	KWizard(parent, n),
	fMode(m ? Standalone : InDialog)
{
	page1=new ConfigWizard_base1(this);
	addPage( page1, i18n("Select Connection Type") );
	page2=new ConfigWizard_base2(this);
	addPage( page2, i18n("Pilot Info") );
	page3=new ConfigWizard_base3(this);
	addPage( page3, i18n("Application to Sync With") );
	setFinishEnabled( page3, true );

	connect( page2->fProbeButton, SIGNAL( pressed() ),
		this, SLOT( probeHandheld() ) );

	KPilotSettings::self()->readConfig();
	page2->fUserName->setText( KPilotSettings::userName() );
	page2->fDeviceName->setText( KPilotSettings::pilotDevice() );
	page2->fPilotRunningPermanently->setChecked( KPilotSettings::startDaemonAtLogin() );
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
		eAppKontact,
		eAppEvolution
	} app;
	app=(eSyncApp)( page3->fAppType->selectedId() );
	bool keepPermanently( page2->fPilotRunningPermanently->isChecked() );
#ifdef DEBUG
	DEBUGCONDUIT<<fname<<"Keep permanently: "<<keepPermanently<<endl;
#endif

	KPilotSettings::setPilotDevice( devicename );
	KPilotSettings::setUserName(username);
	KPilotSettings::setEncoding("iso 8859-15");
	KPilotSettings::setDockDaemon( true );
	KPilotSettings::setKillDaemonAtExit( !keepPermanently);
	KPilotSettings::setQuitAfterSync( !keepPermanently );
	KPilotSettings::setStartDaemonAtLogin( keepPermanently );
	KPilotSettings::setSyncType(0);
	KPilotSettings::setFullSyncOnPCChange( true );
	KPilotSettings::setConflictResolution(0);

	QStringList conduits = KPilotSettings::installedConduits();
	// TODO: enable the right conduits
#define APPEND_CONDUIT(a) if (!conduits.contains(a)) conduits.append(a)
	QString applicationName(i18n("general KDE-PIM"));
	APPEND_CONDUIT("internal_fileinstall");
	APPEND_CONDUIT("todo-conduit");
	APPEND_CONDUIT("vcal-conduit");
	switch (app) {
		case eAppEvolution:
			applicationName=i18n("Gnome's PIM suite", "Evolution");
			conduits.remove("knotes-conduit");
			// TODO: Once the Evolution abook resource is finished, enable it...
			conduits.remove("abbrowser_conduit");
			// TODO: settings for conduits
			KMessageBox::information(this, i18n("KPilot cannot yet synchronize the addressbook with Evolution, so the addressbook conduit was disabled.\nWhen syncing the calendar or todo list using KPilot please quit Evolution before the sync, otherwise you will lose data."), i18n("Restrictions with Evolution"));
			break;
		case eAppKontact:
			applicationName=i18n("KDE's PIM suite", "Kontact");
		case eAppKDE:
		default:
			APPEND_CONDUIT("knotes-conduit");
			APPEND_CONDUIT("abbrowser_conduit");
			// TODO: settings for conduits
			break;
	}
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

// Devices to probe:
// Linux: /dev/pilot (symlink), /dev/ttyS* (serial + irda), /dev/tts/[012345...] (with devfs),
//        /dev/ttyUSB*, /dev/usb/tts/[012345...]
// *BSD: /dev/pilot, /dev/cuaa[01]   (serial), /dev/ucom* (usb)

void ConfigWizard::probeHandheld()
{
	ProbeDialog *probeDialog = new ProbeDialog( this );
	if ( probeDialog->exec() && probeDialog->detected() ) {
		page2->fUserName->setText( probeDialog->userName() );
		page2->fDeviceName->setText( probeDialog->device() );
	}
	KPILOT_DELETE(probeDialog);
}

