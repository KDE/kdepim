/* conduitConfigDialog.cc                KPilot
**
** Copyright (C) 2001 by Dan Pilone
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

ConfigWizard::ConfigWizard(QWidget *parent, const char *n) :
	KWizard(parent, n)
{
	page1=new ConfigWizard_base1(this);
	addPage( page1, i18n("Select connection type") );
	page2=new ConfigWizard_base2(this);
	addPage( page2, i18n("Pilot info") );
	page3=new ConfigWizard_base3(this);
	addPage( page3, i18n("Application to sync with") );
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
	int devicetype( page1->fConnectionType->selectedId() );
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
	APPEND_CONDUIT("internal_fileinstall");
	APPEND_CONDUIT("todo-conduit");
	APPEND_CONDUIT("vcal-conduit");
	switch (app) {
		case eAppEvolution:
			conduits.remove("knotes-conduit");
			// TODO: Once the Evolution abook resource is finished, enable it...
			conduits.remove("abbrowser_conduit");
			// TODO: settings for conduits
			break;
		case eAppKDE:
		case eAppKontact:
		default:
			APPEND_CONDUIT("knotes-conduit");
			APPEND_CONDUIT("abbrowser_conduit");
			// TODO: settings for conduits
			break;
	}
	KPilotSettings::setInstalledConduits( conduits );
#undef APPEND_CONDUIT
	
	KPilotSettings::self()->writeConfig();
	QDialog::accept();
}

void ConfigWizard::probeHandheld()
{
	// TODO
	KMessageBox::information(this, "Probing the handheld, not yet implemented", "Probing");
}

