/* kpilotConfigDialog.cc                KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file defines a specialization of KPilotDeviceLink
** that can actually handle some HotSync tasks, like backup
** and restore. It does NOT do conduit stuff.
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
** Bug reports and questions can be sent to kde-pim@kde.org
*/

static const char *kpilotconfigdialog_id = 
	"$Id: $";

#include "options.h"

#include <qcombobox.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qtabwidget.h>

#include "kpilotConfig.h"

#include "kpilotConfigDialog_base.h"
#include "kpilotConfigDialog.moc"

KPilotConfigDialog::KPilotConfigDialog(QWidget * w, const char *n,
	bool m) : UIDialog(w, n, m)
{
	FUNCTIONSETUP;

	fConfigWidget = new KPilotConfigWidget(widget());
	fConfigWidget->tabWidget->adjustSize();
	fConfigWidget->resize(fConfigWidget->tabWidget->size());
	setTabWidget(fConfigWidget->tabWidget);

	disableUnusedOptions();
	readConfig();

	changePortType(fConfigWidget->fPortType->currentItem());

	QObject::connect(fConfigWidget->fPortType, SIGNAL(activated(int)),
		this, SLOT(changePortType(int)));

	addAboutPage(false);

	(void) kpilotconfigdialog_id;
}

KPilotConfigDialog::~KPilotConfigDialog()
{
	FUNCTIONSETUP;
}

void KPilotConfigDialog::disableUnusedOptions()
{
	FUNCTIONSETUP;

	fConfigWidget->fOverwriteRemote->setEnabled(false);
	fConfigWidget->fForceFirstTime->setEnabled(false);
	fConfigWidget->fFullBackupCheck->setEnabled(false);
	fConfigWidget->fPreferFastSync->setEnabled(false);
}

void KPilotConfigDialog::readConfig()
{
	FUNCTIONSETUP;

	KPilotConfigSettings & c = KPilotConfig::getConfig();
	c.resetGroup();

	(void) c.getPilotDevice(fConfigWidget->fPilotDevice);
	(void) c.getPilotType(fConfigWidget->fPortType);
	(void) c.getPilotSpeed(fConfigWidget->fPilotSpeed);
	(void) c.getUser(fConfigWidget->fUserName);
	(void) c.getStartDaemonAtLogin(fConfigWidget->fStartDaemonAtLogin);
	(void) c.getKillDaemonOnExit(fConfigWidget->fKillDaemonOnExit);
	(void) c.getDockDaemon(fConfigWidget->fDockDaemon);

	(void) c.getShowSecrets(fConfigWidget->fUseSecret);
	(void) c.getBackupOnly(fConfigWidget->fBackupOnly);
	(void) c.getSkip(fConfigWidget->fSkipDB);

	(void) c.getSyncFiles(fConfigWidget->fSyncFiles);

	c.setAddressGroup();
	(void) c.getUseKeyField(fConfigWidget->fUseKeyField);
	setAddressDisplay(c.getAddressDisplayMode());

	c.resetGroup();
}

/* virtual */ void KPilotConfigDialog::commitChanges()
{
	FUNCTIONSETUP;

	KPilotConfigSettings & c = KPilotConfig::getConfig();
	c.resetGroup();

	// General page
	c.setPilotDevice(fConfigWidget->fPilotDevice);
	c.setPilotType(fConfigWidget->fPortType);
	c.setPilotSpeed(fConfigWidget->fPilotSpeed);
	c.setUser(fConfigWidget->fUserName);
	c.setStartDaemonAtLogin(fConfigWidget->fStartDaemonAtLogin);
	c.setKillDaemonOnExit(fConfigWidget->fKillDaemonOnExit);
	c.setDockDaemon(fConfigWidget->fDockDaemon);

	// DB specials page
	c.setShowSecrets(fConfigWidget->fUseSecret);
	c.setBackupOnly(fConfigWidget->fBackupOnly);
	c.setSkip(fConfigWidget->fSkipDB);

	// Sync page
	c.setSyncFiles(fConfigWidget->fSyncFiles);

	// Address page
	c.setAddressGroup();
	c.setUseKeyField(fConfigWidget->fUseKeyField);
	c.setAddressDisplayMode(getAddressDisplay());
	c.resetGroup();

	KPilotConfig::updateConfigVersion();

	c.sync();
}

int KPilotConfigDialog::getAddressDisplay() const
{
	FUNCTIONSETUP;

	if (fConfigWidget->fNormalDisplay->isChecked())
		return 0;
	if (fConfigWidget->fCompanyDisplay->isChecked())
		return 1;

	return 0;
}

void KPilotConfigDialog::setAddressDisplay(int i)
{
	FUNCTIONSETUP;

	switch (i)
	{
	case 0:
		fConfigWidget->fNormalDisplay->setChecked(true);
		break;
	case 1:
		fConfigWidget->fCompanyDisplay->setChecked(true);
		break;
	default:
		fConfigWidget->fNormalDisplay->setChecked(true);
		break;
	}
}


/* slot */ void KPilotConfigDialog::changePortType(int i)
{
	FUNCTIONSETUP;

	switch (i)
	{
	case 0:
		fConfigWidget->fPilotSpeed->setEnabled(true);
		break;
	case 1:
	case 2:
		fConfigWidget->fPilotSpeed->setEnabled(false);
		break;
	default:
		kdWarning() << k_funcinfo
			<< ": Unknown port type " << i << endl;
	}
}

// $Log$
// Revision 1.4  2001/09/30 19:51:56  adridg
// Some last-minute layout, compile, and __FUNCTION__ (for Tru64) changes.
//
// Revision 1.3  2001/09/29 16:26:18  adridg
// The big layout change
//
