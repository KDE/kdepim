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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

static const char *kpilotconfigdialog_id =
	"$Id$";

#include "options.h"

#include <pi-version.h>

#include <qcombobox.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qlineedit.h>
#include <qtabwidget.h>

#include <kmessagebox.h>
#include <kcharsets.h>

#include "kpilotConfig.h"
#include "kpilotSettings.h"

#include "kpilotConfigDialog_base.h"
#include "kpilotConfigDialog2_base.h"
#include "kpilotConfigDialog3_base.h"
#include "kpilotConfigDialog.moc"
#include "syncAction.h"
#include "dbSelectionDialog.h"

DeviceConfigPage::DeviceConfigPage(QWidget * w, const char *n ) : ConduitConfigBase( w, n )
{
	FUNCTIONSETUP;

	fConfigWidget = new DeviceConfigWidget( w );
	// Fill the encodings list
	{
		QStringList l = KGlobal::charsets()->descriptiveEncodingNames();
		for ( QStringList::Iterator it = l.begin(); it != l.end(); ++it )
		{
			fConfigWidget->fPilotEncoding->insertItem(*it);
		}
	}

	fConfigWidget->resize(fConfigWidget->tabWidget->size());
	fWidget = fConfigWidget;

#if defined(PILOT_LINK_VERSION) && defined(PILOT_LINK_MAJOR) && defined(PILOT_LINK_MINOR)
#if (PILOT_LINK_VERSION * 100 + PILOT_LINK_MAJOR * 10 + PILOT_LINK_MINOR) < 100
	fConfigWidget->fPilotDevice->setMaxLength(13);
#endif
#endif


#define CM(a,b) connect(fConfigWidget->a,b,this,SLOT(modified()));
	CM(fPilotDevice, SIGNAL(textChanged(const QString &)));
	CM(fPilotSpeed, SIGNAL(activated(int)));
	CM(fPilotEncoding, SIGNAL(textChanged(const QString &)));
	CM(fUserName, SIGNAL(textChanged(const QString &)));
#if 0
	CM(fStartDaemonAtLogin, SIGNAL(toggled(bool)));
	CM(fKillDaemonOnExit, SIGNAL(toggled(bool)));
	CM(fDockDaemon, SIGNAL(toggled(bool)));
	CM(fQuitAfterSync, SIGNAL(toggled(bool)));

	CM(fInternalEditors, SIGNAL(toggled(bool)));
	CM(fUseSecret, SIGNAL(toggled(bool)));
	CM(fAddressGroup, SIGNAL(clicked(int)));
	CM(fUseKeyField, SIGNAL(toggled(bool)));

#endif
#undef CM

	(void) kpilotconfigdialog_id;
}

void DeviceConfigPage::load()
{
	FUNCTIONSETUP;
	KPilotSettings::self()->readConfig();

	/* General tab in the setup dialog */
	fConfigWidget->fPilotDevice->setText(KPilotSettings::pilotDevice());
	fConfigWidget->fPilotSpeed->setCurrentItem(KPilotSettings::pilotSpeed());
	getEncoding();
	fConfigWidget->fUserName->setText(KPilotSettings::userName());

#if 0
	fConfigWidget->fStartDaemonAtLogin->setChecked(KPilotSettings::startDaemonAtLogin());
	fConfigWidget->fDockDaemon->setChecked(KPilotSettings::dockDaemon());
	fConfigWidget->fKillDaemonOnExit->setChecked(KPilotSettings::killDaemonAtExit());
	fConfigWidget->fQuitAfterSync->setChecked(KPilotSettings::quitAfterSync());

	/* Sync tab */
	int synctype=KPilotSettings::syncType();
	if (synctype < SyncAction::eSyncModeLastRadiobutton)
		fConfigWidget->fSyncMode->setButton(synctype);
	else
	{
		fConfigWidget->fSyncMode->setButton(SyncAction::eSyncModeLastRadiobutton);
		fConfigWidget->fSpecialSync->setCurrentItem(synctype-SyncAction::eSyncModeLastRadiobutton);
	}

	fConfigWidget->fFullBackupCheck->setChecked(KPilotSettings::fullSyncOnPCChange());
	fConfigWidget->fConflictResolution->setCurrentItem(KPilotSettings::conflictResolution());

	/* Viewers tab */
	fConfigWidget->fInternalEditors->setChecked(KPilotSettings::internalEditors());
	fConfigWidget->fUseSecret->setChecked(KPilotSettings::showSecrets());
	fConfigWidget->fAddressGroup->setButton(KPilotSettings::addressDisplayMode());
	fConfigWidget->fUseKeyField->setChecked(KPilotSettings::useKeyField());

	/* Backup tab */
	fConfigWidget->fBackupOnly->setText(KPilotSettings::skipBackupDB());
	fConfigWidget->fSkipDB->setText(KPilotSettings::skipRestoreDB());
#endif
	unmodified();
}

/* virtual */ bool DeviceConfigPage::validate()
{
	int r = KMessageBox::Yes;

#if defined(PILOT_LINK_VERSION) && defined(PILOT_LINK_MAJOR) && defined(PILOT_LINK_MINOR)
#if (PILOT_LINK_VERSION * 100 + PILOT_LINK_MAJOR * 10 + PILOT_LINK_MINOR) < 100
	QString d = fConfigWidget->fPilotDevice->text();

	if (d.length() > 13)
	{
	r = KMessageBox::questionYesNo(
		fConfigWidget,
		i18n("<qt>The device name you entered (<i>%1</i>) "
			"is longer than 13 characters. This is "
			"probably unsupported and can cause problems. "
			"Are you sure you want to use this device name?</qt>")
			.arg(d),
		i18n("Device Name too Long")
		) ;
	}
#endif
#endif

	return KMessageBox::Yes == r;
}

/* virtual */ void DeviceConfigPage::commit()
{
	FUNCTIONSETUP;

	// General page
	KPilotSettings::setPilotDevice(fConfigWidget->fPilotDevice->text());
	KPilotSettings::setPilotSpeed(fConfigWidget->fPilotSpeed->currentItem());
	setEncoding();
	KPilotSettings::setUserName(fConfigWidget->fUserName->text());

#if 0
	KPilotSettings::setStartDaemonAtLogin(fConfigWidget->fStartDaemonAtLogin->isChecked());
	KPilotSettings::setDockDaemon(fConfigWidget->fDockDaemon->isChecked());
	KPilotSettings::setKillDaemonAtExit(fConfigWidget->fKillDaemonOnExit->isChecked());
	KPilotSettings::setQuitAfterSync(fConfigWidget->fQuitAfterSync->isChecked());

	/* Sync tab */
	int syncmode=fConfigWidget->fSyncMode->id(fConfigWidget->fSyncMode->selected());
	if (syncmode==SyncAction::eSyncModeLastRadiobutton)
		syncmode+=fConfigWidget->fSpecialSync->currentItem();
	KPilotSettings::setSyncType(syncmode);
	KPilotSettings::setFullSyncOnPCChange(fConfigWidget->fFullBackupCheck->isChecked());
	KPilotSettings::setConflictResolution(fConfigWidget->fConflictResolution->currentItem());

	/* Viewers tab */
	KPilotSettings::setInternalEditors( fConfigWidget->fInternalEditors->isChecked());
	KPilotSettings::setShowSecrets(fConfigWidget->fUseSecret->isChecked());
	KPilotSettings::setAddressDisplayMode(fConfigWidget->fAddressGroup->id(
		fConfigWidget->fAddressGroup->selected()));
	KPilotSettings::setUseKeyField(fConfigWidget->fUseKeyField->isChecked());

	/* Backup tab */
	KPilotSettings::setSkipBackupDB(fConfigWidget->fBackupOnly->text());
	KPilotSettings::setSkipRestoreDB(fConfigWidget->fSkipDB->text());
#endif

	KPilotConfig::updateConfigVersion();
	KPilotSettings::self()->writeConfig();
	unmodified();
}

/* slot */ void DeviceConfigPage::changePortType(int i)
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

void DeviceConfigPage::getEncoding()
{
	FUNCTIONSETUP;
	QString e = KPilotSettings::encoding();
	if (e.isEmpty())
		fConfigWidget->fPilotEncoding->setCurrentItem(0);
	else
		fConfigWidget->fPilotEncoding->setCurrentText(e);
}

void DeviceConfigPage::setEncoding()
{
	FUNCTIONSETUP;

	QString enc = fConfigWidget->fPilotEncoding->currentText();
	if (enc.isEmpty())
	{
		kdWarning() << k_funcinfo << "Empty encoding. Will ignore it"<<endl;
	}
	else
	{
		KPilotSettings::setEncoding(enc);
	}
}

SyncConfigPage::SyncConfigPage(QWidget * w, const char *n ) : ConduitConfigBase( w, n )
{
	FUNCTIONSETUP;

	fConfigWidget = new SyncConfigWidget( w );
	fConfigWidget->resize(fConfigWidget->tabWidget->size());
	fWidget = fConfigWidget;

	connect(fConfigWidget->fBackupOnlyChooser, SIGNAL( clicked() ),
		SLOT( slotSelectNoBackupDBs() ) );
	connect(fConfigWidget->fSkipDBChooser, SIGNAL(clicked()),
		SLOT(slotSelectNoRestoreDBs()));

#define CM(a,b) connect(fConfigWidget->a,b,this,SLOT(modified()));
	CM(fSyncMode, SIGNAL(clicked(int)));
	CM(fSpecialSync, SIGNAL(textChanged(const QString &)));
	CM(fFullBackupCheck, SIGNAL(toggled(bool)));
	CM(fConflictResolution, SIGNAL(activated(int)));

	CM(fBackupOnly, SIGNAL(textChanged(const QString &)));
	CM(fSkipDB, SIGNAL(textChanged(const QString &)));
#undef CM
}

void SyncConfigPage::load()
{
	FUNCTIONSETUP;
	KPilotSettings::self()->readConfig();

	/* Sync tab */
	int synctype=KPilotSettings::syncType();
	if (synctype < SyncAction::eSyncModeLastRadiobutton)
		fConfigWidget->fSyncMode->setButton(synctype);
	else
	{
		fConfigWidget->fSyncMode->setButton(SyncAction::eSyncModeLastRadiobutton);
		fConfigWidget->fSpecialSync->setCurrentItem(synctype-SyncAction::eSyncModeLastRadiobutton);
	}

	fConfigWidget->fFullBackupCheck->setChecked(KPilotSettings::fullSyncOnPCChange());
	fConfigWidget->fConflictResolution->setCurrentItem(KPilotSettings::conflictResolution());

	/* Backup tab */
	fConfigWidget->fBackupOnly->setText(KPilotSettings::skipBackupDB());
	fConfigWidget->fSkipDB->setText(KPilotSettings::skipRestoreDB());

	unmodified();
}

/* virtual */ bool SyncConfigPage::validate()
{
	return true;
}

/* virtual */ void SyncConfigPage::commit()
{
	FUNCTIONSETUP;

	/* Sync tab */
	int syncmode=fConfigWidget->fSyncMode->id(fConfigWidget->fSyncMode->selected());
	if (syncmode==SyncAction::eSyncModeLastRadiobutton)
		syncmode+=fConfigWidget->fSpecialSync->currentItem();
	KPilotSettings::setSyncType(syncmode);
	KPilotSettings::setFullSyncOnPCChange(fConfigWidget->fFullBackupCheck->isChecked());
	KPilotSettings::setConflictResolution(fConfigWidget->fConflictResolution->currentItem());

	/* Backup tab */
	KPilotSettings::setSkipBackupDB(fConfigWidget->fBackupOnly->text());
	KPilotSettings::setSkipRestoreDB(fConfigWidget->fSkipDB->text());

	KPilotConfig::updateConfigVersion();
	KPilotSettings::self()->writeConfig();
	unmodified();
}

void SyncConfigPage::slotSelectNoBackupDBs()
{
	FUNCTIONSETUP;

	QStringList selectedDBs(QStringList::split(',', fConfigWidget->fBackupOnly->text() ));

	QStringList deviceDBs=KPilotSettings::deviceDBs();
	QStringList addedDBs=KPilotSettings::addedDBs();
	KPilotDBSelectionDialog*dlg=new KPilotDBSelectionDialog(selectedDBs, deviceDBs, addedDBs, 0, "NoBackupDBs");
	if (dlg && (dlg->exec()==QDialog::Accepted) )
	{
		fConfigWidget->fBackupOnly->setText(
			dlg->getSelectedDBs().join(","));
		KPilotSettings::setAddedDBs( dlg->getAddedDBs() );
	}
	KPILOT_DELETE(dlg);
}

void SyncConfigPage::slotSelectNoRestoreDBs()
{
	FUNCTIONSETUP;

	QStringList selectedDBs(QStringList::split(',', fConfigWidget->fSkipDB->text() ));

	QStringList deviceDBs=KPilotSettings::deviceDBs();
	QStringList addedDBs=KPilotSettings::addedDBs();
	KPilotDBSelectionDialog*dlg=new KPilotDBSelectionDialog(selectedDBs, deviceDBs, addedDBs, 0, "NoRestoreDBs");
	if (dlg && (dlg->exec()==QDialog::Accepted) )
	{
		fConfigWidget->fSkipDB->setText(
			dlg->getSelectedDBs().join(","));
		KPilotSettings::setAddedDBs( dlg->getAddedDBs() );
	}
	KPILOT_DELETE(dlg);
}



KPilotConfigPage::KPilotConfigPage(QWidget * w, const char *n ) : ConduitConfigBase( w, n )
{
	FUNCTIONSETUP;

	fConfigWidget = new KPilotConfigWidget( w );
	fConfigWidget->resize(fConfigWidget->tabWidget->size());
	fWidget = fConfigWidget;

#define CM(a,b) connect(fConfigWidget->a,b,this,SLOT(modified()));
	CM(fStartDaemonAtLogin, SIGNAL(toggled(bool)));
	CM(fKillDaemonOnExit, SIGNAL(toggled(bool)));
	CM(fDockDaemon, SIGNAL(toggled(bool)));
	CM(fQuitAfterSync, SIGNAL(toggled(bool)));

	CM(fInternalEditors, SIGNAL(toggled(bool)));
	CM(fUseSecret, SIGNAL(toggled(bool)));
	CM(fAddressGroup, SIGNAL(clicked(int)));
	CM(fUseKeyField, SIGNAL(toggled(bool)));
#undef CM
}

void KPilotConfigPage::load()
{
	FUNCTIONSETUP;
	KPilotSettings::self()->readConfig();

	fConfigWidget->fStartDaemonAtLogin->setChecked(KPilotSettings::startDaemonAtLogin());
	fConfigWidget->fDockDaemon->setChecked(KPilotSettings::dockDaemon());
	fConfigWidget->fKillDaemonOnExit->setChecked(KPilotSettings::killDaemonAtExit());
	fConfigWidget->fQuitAfterSync->setChecked(KPilotSettings::quitAfterSync());

	/* Viewers tab */
	fConfigWidget->fInternalEditors->setChecked(KPilotSettings::internalEditors());
	fConfigWidget->fUseSecret->setChecked(KPilotSettings::showSecrets());
	fConfigWidget->fAddressGroup->setButton(KPilotSettings::addressDisplayMode());
	fConfigWidget->fUseKeyField->setChecked(KPilotSettings::useKeyField());

	unmodified();
}

/* virtual */ bool KPilotConfigPage::validate()
{
	return true;
}

/* virtual */ void KPilotConfigPage::commit()
{
	FUNCTIONSETUP;

	KPilotSettings::setStartDaemonAtLogin(fConfigWidget->fStartDaemonAtLogin->isChecked());
	KPilotSettings::setDockDaemon(fConfigWidget->fDockDaemon->isChecked());
	KPilotSettings::setKillDaemonAtExit(fConfigWidget->fKillDaemonOnExit->isChecked());
	KPilotSettings::setQuitAfterSync(fConfigWidget->fQuitAfterSync->isChecked());

	/* Viewers tab */
	KPilotSettings::setInternalEditors( fConfigWidget->fInternalEditors->isChecked());
	KPilotSettings::setShowSecrets(fConfigWidget->fUseSecret->isChecked());
	KPilotSettings::setAddressDisplayMode(fConfigWidget->fAddressGroup->id(
		fConfigWidget->fAddressGroup->selected()));
	KPilotSettings::setUseKeyField(fConfigWidget->fUseKeyField->isChecked());

	KPilotConfig::updateConfigVersion();
	KPilotSettings::self()->writeConfig();
	unmodified();
}

