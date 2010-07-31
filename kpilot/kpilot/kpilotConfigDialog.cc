/* KPilot
**
** Copyright (C) 2001 by Dan Pilone
** Copyright (C) 2002-2004 by Adriaan de Groot
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <pi-version.h>

#include <tqcombobox.h>
#include <tqcheckbox.h>
#include <tqradiobutton.h>
#include <tqpushbutton.h>
#include <tqbuttongroup.h>
#include <tqlineedit.h>
#include <tqtabwidget.h>
#include <tqspinbox.h>
#include <tqfile.h>

#include <kmessagebox.h>
#include <kcharsets.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <kurl.h>
#include <kio/netaccess.h>

#include "kpilotConfig.h"
#include "kpilotSettings.h"

#include "kpilotConfigDialog_device.h"
#include "kpilotConfigDialog_sync.h"
#include "kpilotConfigDialog_startup.h"
#include "kpilotConfigDialog_viewers.h"
#include "kpilotConfigDialog_backup.h"
#include "kpilotConfigDialog.moc"
#include "syncAction.h"
#include "dbSelectionDialog.h"

/* virtual */ TQString ConfigPage::maybeSaveText() const
{
	return i18n("<qt>The settings for configuration page <i>%1</i> have been changed. Do you "
		"want to save the changes before continuing?</qt>").arg(this->conduitName());
}

DeviceConfigPage::DeviceConfigPage(TQWidget * w, const char *n ) : ConfigPage( w, n )
{
	FUNCTIONSETUP;

	fConfigWidget = new DeviceConfigWidget( w );
	// Fill the encodings list
	{
		TQStringList l = KGlobal::charsets()->descriptiveEncodingNames();
		for ( TQStringList::Iterator it = l.begin(); it != l.end(); ++it )
		{
			fConfigWidget->fPilotEncoding->insertItem(*it);
		}
	}

	fConfigWidget->resize(fConfigWidget->size());
	fWidget = fConfigWidget;

#if PILOT_LINK_NUMBER < PILOT_LINK_0_10_0
	fConfigWidget->fPilotDevice->setMaxLength(13);
#endif


#define CM(a,b) connect(fConfigWidget->a,b,this,TQT_SLOT(modified()));
	CM(fPilotDevice, TQT_SIGNAL(textChanged(const TQString &)));
	CM(fPilotSpeed, TQT_SIGNAL(activated(int)));
	CM(fPilotEncoding, TQT_SIGNAL(textChanged(const TQString &)));
	CM(fUserName, TQT_SIGNAL(textChanged(const TQString &)));
	CM(fWorkaround, TQT_SIGNAL(activated(int)));
#undef CM

	fConduitName = i18n("Device");
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

	switch(KPilotSettings::workarounds())
	{
	case KPilotSettings::eWorkaroundNone :
		fConfigWidget->fWorkaround->setCurrentItem(0);
		break;
	case KPilotSettings::eWorkaroundUSB :
		fConfigWidget->fWorkaround->setCurrentItem(1);
		break;
	default:
		WARNINGKPILOT << "Unknown workaround number "
			<< (int) KPilotSettings::workarounds()
			<< endl;
		KPilotSettings::setWorkarounds(KPilotSettings::eWorkaroundNone);
		fConfigWidget->fWorkaround->setCurrentItem(0);
	}
	unmodified();
}

/* virtual */ bool DeviceConfigPage::validate()
{
	int r = KMessageBox::Yes;

#if PILOT_LINK_NUMBER < PILOT_LINK_0_10_0
	TQString d = fConfigWidget->fPilotDevice->text();

	if (d.length() > 13)
	{
	r = KMessageBox::questionYesNo(
		fConfigWidget,
		i18n("<qt>The device name you entered (<i>%1</i>) "
			"is longer than 13 characters. This is "
			"probably unsupported and can cause problems. "
			"Are you sure you want to use this device name?</qt>")
			.arg(d),
		i18n("Device Name too Long"), i18n("Use"), i18n("Do Not Use")
		) ;
	}
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

	switch(fConfigWidget->fWorkaround->currentItem())
	{
	case 0 : KPilotSettings::setWorkarounds(KPilotSettings::eWorkaroundNone); break;
	case 1 : KPilotSettings::setWorkarounds(KPilotSettings::eWorkaroundUSB); break;
	default :
		WARNINGKPILOT << "Unknown workaround number "
			<< fConfigWidget->fWorkaround->currentItem()
			<< endl;
		KPilotSettings::setWorkarounds(KPilotSettings::eWorkaroundNone);

	}
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
		WARNINGKPILOT << "Unknown port type " << i << endl;
	}
}

void DeviceConfigPage::getEncoding()
{
	FUNCTIONSETUP;
	TQString e = KPilotSettings::encoding();
	if (e.isEmpty())
		fConfigWidget->fPilotEncoding->setCurrentText(CSL1("ISO8859-15"));
	else
		fConfigWidget->fPilotEncoding->setCurrentText(e);
}

void DeviceConfigPage::setEncoding()
{
	FUNCTIONSETUP;

	TQString enc = fConfigWidget->fPilotEncoding->currentText();
	if (enc.isEmpty())
	{
		WARNINGKPILOT << "Empty encoding. Will ignore it." << endl;
	}
	else
	{
		KPilotSettings::setEncoding(enc);
	}
}

SyncConfigPage::SyncConfigPage(TQWidget * w, const char *n ) : ConfigPage( w, n )
{
	FUNCTIONSETUP;

	fConfigWidget = new SyncConfigWidget( w );
	fConfigWidget->resize(fConfigWidget->size());
	fWidget = fConfigWidget;

#define CM(a,b) connect(fConfigWidget->a,b,this,TQT_SLOT(modified()));
	CM(fSpecialSync, TQT_SIGNAL(activated(int)));
	CM(fFullSyncCheck, TQT_SIGNAL(toggled(bool)));
	CM(fScreenlockSecure, TQT_SIGNAL(toggled(bool)));
	CM(fConflictResolution, TQT_SIGNAL(activated(int)));
#undef CM

	fConduitName = i18n("HotSync");
}

#define MENU_ITEM_COUNT (4)
static SyncAction::SyncMode::Mode syncTypeMap[MENU_ITEM_COUNT] = {
	SyncAction::SyncMode::eHotSync,
	SyncAction::SyncMode::eFullSync,
	SyncAction::SyncMode::eCopyPCToHH,
	SyncAction::SyncMode::eCopyHHToPC
	} ;

void SyncConfigPage::load()
{
	FUNCTIONSETUP;
	KPilotSettings::self()->readConfig();

	/* Sync tab */
	int synctype=KPilotSettings::syncType();
	if (synctype<0) synctype=(int) SyncAction::SyncMode::eHotSync;
	for (unsigned int i=0; i<MENU_ITEM_COUNT; ++i)
	{
		if (syncTypeMap[i] == synctype)
		{
			fConfigWidget->fSpecialSync->setCurrentItem(i);
			synctype=-1;
			break;
		}
	}
	if (synctype != -1)
	{
		fConfigWidget->fSpecialSync->setCurrentItem(0); /* HotSync */
	}

	fConfigWidget->fFullSyncCheck->setChecked(KPilotSettings::fullSyncOnPCChange());
	fConfigWidget->fConflictResolution->setCurrentItem(KPilotSettings::conflictResolution());
	fConfigWidget->fScreenlockSecure->setChecked(KPilotSettings::screenlockSecure());

	unmodified();
}

/* virtual */ void SyncConfigPage::commit()
{
	FUNCTIONSETUP;

	/* Sync tab */
	int synctype = -1;
	unsigned int selectedsync = fConfigWidget->fSpecialSync->currentItem();
	if (selectedsync < MENU_ITEM_COUNT)
	{
		synctype = syncTypeMap[selectedsync];
	}
	if (synctype < 0)
	{
		synctype = SyncAction::SyncMode::eHotSync;
	}

	KPilotSettings::setSyncType(synctype);
	KPilotSettings::setFullSyncOnPCChange(fConfigWidget->fFullSyncCheck->isChecked());
	KPilotSettings::setConflictResolution(fConfigWidget->fConflictResolution->currentItem());
	KPilotSettings::setScreenlockSecure(fConfigWidget->fScreenlockSecure->isChecked());

	KPilotConfig::updateConfigVersion();
	KPilotSettings::self()->writeConfig();
	unmodified();
}


BackupConfigPage::BackupConfigPage(TQWidget * w, const char *n ) : ConfigPage( w, n )
{
	FUNCTIONSETUP;

	fConfigWidget = new BackupConfigWidget( w );
	fConfigWidget->resize(fConfigWidget->size());
	fWidget = fConfigWidget;

	connect(fConfigWidget->fBackupOnlyChooser, TQT_SIGNAL( clicked() ),
		TQT_SLOT( slotSelectNoBackupDBs() ) );
	connect(fConfigWidget->fSkipDBChooser, TQT_SIGNAL(clicked()),
		TQT_SLOT(slotSelectNoRestoreDBs()));

#define CM(a,b) connect(fConfigWidget->a,b,this,TQT_SLOT(modified()));
	CM(fBackupOnly, TQT_SIGNAL(textChanged(const TQString &)));
	CM(fSkipDB, TQT_SIGNAL(textChanged(const TQString &)));
	CM(fBackupFrequency, TQT_SIGNAL(activated(int)));
#undef CM

	fConduitName = i18n("Backup");
}

void BackupConfigPage::load()
{
	FUNCTIONSETUP;
	KPilotSettings::self()->readConfig();

	/* Backup tab */
	fConfigWidget->fBackupOnly->setText(KPilotSettings::skipBackupDB().join(CSL1(",")));
	fConfigWidget->fSkipDB->setText(KPilotSettings::skipRestoreDB().join(CSL1(",")));
	fConfigWidget->fRunConduitsWithBackup->setChecked(KPilotSettings::runConduitsWithBackup());

	int backupfreq=KPilotSettings::backupFrequency();

	fConfigWidget->fBackupFrequency->setCurrentItem(backupfreq);

	unmodified();
}

/* virtual */ void BackupConfigPage::commit()
{
	FUNCTIONSETUP;

	/* Backup tab */
	KPilotSettings::setSkipBackupDB(
		TQStringList::split(CSL1(","),fConfigWidget->fBackupOnly->text()));
	KPilotSettings::setSkipRestoreDB(
		TQStringList::split(CSL1(","),fConfigWidget->fSkipDB->text()));
	KPilotSettings::setRunConduitsWithBackup(fConfigWidget->fRunConduitsWithBackup->isChecked());
	KPilotSettings::setBackupFrequency(fConfigWidget->fBackupFrequency->currentItem());

	KPilotConfig::updateConfigVersion();
	KPilotSettings::self()->writeConfig();
	unmodified();
}

void BackupConfigPage::slotSelectNoBackupDBs()
{
	FUNCTIONSETUP;

	TQStringList selectedDBs(TQStringList::split(',', fConfigWidget->fBackupOnly->text() ));

	TQStringList deviceDBs=KPilotSettings::deviceDBs();
	TQStringList addedDBs=KPilotSettings::addedDBs();
	KPilotDBSelectionDialog*dlg=new KPilotDBSelectionDialog(selectedDBs, deviceDBs, addedDBs, 0, "NoBackupDBs");
	if (dlg && (dlg->exec()==TQDialog::Accepted) )
	{
		fConfigWidget->fBackupOnly->setText(
			dlg->getSelectedDBs().join(CSL1(",")));
		KPilotSettings::setAddedDBs( dlg->getAddedDBs() );
	}
	KPILOT_DELETE(dlg);
}

void BackupConfigPage::slotSelectNoRestoreDBs()
{
	FUNCTIONSETUP;

	TQStringList selectedDBs(TQStringList::split(',', fConfigWidget->fSkipDB->text() ));

	TQStringList deviceDBs=KPilotSettings::deviceDBs();
	TQStringList addedDBs=KPilotSettings::addedDBs();
	KPilotDBSelectionDialog*dlg=new KPilotDBSelectionDialog(selectedDBs, deviceDBs, addedDBs, 0, "NoRestoreDBs");
	if (dlg && (dlg->exec()==TQDialog::Accepted) )
	{
		fConfigWidget->fSkipDB->setText(
			dlg->getSelectedDBs().join(CSL1(",")));
		KPilotSettings::setAddedDBs( dlg->getAddedDBs() );
	}
	KPILOT_DELETE(dlg);
}



ViewersConfigPage::ViewersConfigPage(TQWidget * w, const char *n ) : ConfigPage( w, n )
{
	FUNCTIONSETUP;

	fConfigWidget = new ViewersConfigWidget( w );
	fConfigWidget->resize(fConfigWidget->size());
	fWidget = fConfigWidget;

#define CM(a,b) connect(fConfigWidget->a,b,this,TQT_SLOT(modified()));
	CM(fInternalEditors, TQT_SIGNAL(toggled(bool)));
	CM(fUseSecret, TQT_SIGNAL(toggled(bool)));
	CM(fAddressGroup, TQT_SIGNAL(clicked(int)));
	CM(fUseKeyField, TQT_SIGNAL(toggled(bool)));
#undef CM

	fConduitName = i18n("Viewers");
}

void ViewersConfigPage::load()
{
	FUNCTIONSETUP;
	KPilotSettings::self()->readConfig();

	fConfigWidget->fInternalEditors->setChecked( false /* KPilotSettings::internalEditors() */ );
	fConfigWidget->fUseSecret->setChecked(KPilotSettings::showSecrets());
	fConfigWidget->fAddressGroup->setButton(KPilotSettings::addressDisplayMode());
	fConfigWidget->fUseKeyField->setChecked(KPilotSettings::useKeyField());
	unmodified();
}

/* virtual */ void ViewersConfigPage::commit()
{
	FUNCTIONSETUP;

	KPilotSettings::setInternalEditors( fConfigWidget->fInternalEditors->isChecked());
	KPilotSettings::setShowSecrets(fConfigWidget->fUseSecret->isChecked());
	KPilotSettings::setAddressDisplayMode(fConfigWidget->fAddressGroup->id(
		fConfigWidget->fAddressGroup->selected()));
	KPilotSettings::setUseKeyField(fConfigWidget->fUseKeyField->isChecked());
	KPilotConfig::updateConfigVersion();
	KPilotSettings::self()->writeConfig();
	unmodified();
}









StartExitConfigPage::StartExitConfigPage(TQWidget * w, const char *n ) : ConfigPage( w, n )
{
	FUNCTIONSETUP;

	fConfigWidget = new StartExitConfigWidget( w );
	fConfigWidget->resize(fConfigWidget->size());
	fWidget = fConfigWidget;

#define CM(a,b) connect(fConfigWidget->a,b,this,TQT_SLOT(modified()));
	CM(fStartDaemonAtLogin, TQT_SIGNAL(toggled(bool)));
	CM(fKillDaemonOnExit, TQT_SIGNAL(toggled(bool)));
	CM(fDockDaemon, TQT_SIGNAL(toggled(bool)));
	CM(fQuitAfterSync, TQT_SIGNAL(toggled(bool)));
#undef CM

	fConduitName = i18n("Startup and Exit");
}

void StartExitConfigPage::load()
{
	FUNCTIONSETUP;
	KPilotSettings::self()->readConfig();

	fConfigWidget->fStartDaemonAtLogin->setChecked(KPilotSettings::startDaemonAtLogin());
	fConfigWidget->fDockDaemon->setChecked(KPilotSettings::dockDaemon());
	fConfigWidget->fKillDaemonOnExit->setChecked(KPilotSettings::killDaemonAtExit());
	fConfigWidget->fQuitAfterSync->setChecked(KPilotSettings::quitAfterSync());
	unmodified();
}


/* virtual */ void StartExitConfigPage::commit()
{
	FUNCTIONSETUP;

	TQString autostart = KGlobalSettings::autostartPath();
	TQString desktopfile = CSL1("kpilotdaemon.desktop");
	TQString desktopcategory = CSL1("kde/");
	TQString location = KGlobal::dirs()->findResource("xdgdata-apps",desktopcategory + desktopfile);
	if (location.isEmpty()) // Fallback to KDE 3.0?
	{
		location = KGlobal::dirs()->findResource("apps",desktopfile);
	}

#ifdef DEBUG
	DEBUGKPILOT << fname << ": Autostart=" << autostart << endl;
	DEBUGKPILOT << fname << ": desktop=" << desktopfile << endl;
	DEBUGKPILOT << fname << ": location=" << location << endl;
#endif

	KPilotSettings::setStartDaemonAtLogin(fConfigWidget->fStartDaemonAtLogin->isChecked());
	if (KPilotSettings::startDaemonAtLogin())
	{
		if (!location.isEmpty())
		{
			KURL src;
			src.setPath(location);
			KURL dst;
			dst.setPath(autostart+desktopfile);
			KIO::NetAccess::file_copy(src,dst,-1 /* 0666? */,true /* overwrite */);
		}
	}
	else
	{
		TQFile::remove(autostart+desktopfile);
	}
	KPilotSettings::setDockDaemon(fConfigWidget->fDockDaemon->isChecked());
	KPilotSettings::setKillDaemonAtExit(fConfigWidget->fKillDaemonOnExit->isChecked());
	KPilotSettings::setQuitAfterSync(fConfigWidget->fQuitAfterSync->isChecked());
	KPilotConfig::updateConfigVersion();
	KPilotSettings::self()->writeConfig();
	unmodified();
}

