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

#include "kpilotConfig.h"

#include "kpilotConfigDialog_base.h"
#include "kpilotConfigDialog.moc"
#include "syncAction.h"
#include "dbSelectionDialog.h"

// Vaguely copied from the JPilot source
// list of supported codecs taken from QTextEncoding's doc
/* I allow entries of the form
	Description (actual codec name)
where only the parts in brackets will be used in the QTextCodec. */
#define ENCODING_COUNT	(42)
static const char *encodings[ENCODING_COUNT+11] = {
	I18N_NOOP("Western (ISO8859-1)"),
	I18N_NOOP("Central European (ISO8859-2)"),
	I18N_NOOP("Central European (ISO8859-3)"),
	I18N_NOOP("Baltic (ISO8859-4)"),
	I18N_NOOP("Cyrillic (ISO8859-5)"),
	I18N_NOOP("Arabic (ISO8859-6)"),
	I18N_NOOP("Greek (ISO8859-7)"),
	I18N_NOOP("Hebrew, visually ordered (ISO8859-8)"),
	I18N_NOOP("Hebrew, logically ordered (ISO8859-8-i)"),
	I18N_NOOP("Turkish (ISO8859-9)"),
	"ISO8859-10",
	"ISO8859-13",
	"ISO8859-14",
	I18N_NOOP("Western Euro (ISO8859-15)"),
	"Apple Roman",
	I18N_NOOP("Arabic (CP1256)"),
	I18N_NOOP("Baltic (CP1257)"),
	I18N_NOOP("Central European (CP1250)"),
	I18N_NOOP("Chinese (Big5)"),	// Chinese, hopefully the same as gtkrc.zh_TW.Big5
	I18N_NOOP("Chinese (Big5-HKSCS)"),
	I18N_NOOP("Chinese (GB18030)"),
	I18N_NOOP("Chinese (GB2312)"),
	I18N_NOOP("Chinese (GBK)"),
	"CP1258",
	"CP874",
	I18N_NOOP("Cyrillic (CP1251)"),
	I18N_NOOP("Greek (CP1253)"),
	I18N_NOOP("Hebrew (CP1255)"),
	"IBM 850",
	"IBM 866",
	I18N_NOOP("Japanese (eucJP)"),
	I18N_NOOP("Japanese (JIS7)"),
	I18N_NOOP("Japanese (Shift-JIS)"),
	I18N_NOOP("Korean (eucKR)"),
	I18N_NOOP("Russian (KOI8-R)"),
	I18N_NOOP("Tamil (TSCII)"),
	I18N_NOOP("Thai (TIS-620)"),
	I18N_NOOP("Turkish (CP1254)"),
	I18N_NOOP("Ukrainian (KOI8-U)"),
	I18N_NOOP("Unicode, 8-bit (utf8)"),
	I18N_NOOP("Unicode (utf16)"),
	I18N_NOOP("Western (CP1252)")
};

KPilotConfigDialog::KPilotConfigDialog(QWidget * w, const char *n,
	bool m) : UIDialog(w, n, m)
{
	FUNCTIONSETUP;

	fConfigWidget = new KPilotConfigWidget(widget());
	// Fill the encodings list
	for (int i=0; i<ENCODING_COUNT; i++)
	{
		fConfigWidget->fPilotEncoding->insertItem(i18n(encodings[i]));
	}
	fConfigWidget->tabWidget->adjustSize();
	fConfigWidget->resize(fConfigWidget->tabWidget->size());
	setTabWidget(fConfigWidget->tabWidget);

#if defined(PILOT_LINK_VERSION) && defined(PILOT_LINK_MAJOR) && defined(PILOT_LINK_MINOR)
#if (PILOT_LINK_VERSION * 100 + PILOT_LINK_MAJOR * 10 + PILOT_LINK_MINOR) < 100
	fConfigWidget->fPilotDevice->setMaxLength(13);
#endif
#endif

	disableUnusedOptions();
	readConfig();


	addAboutPage(false);
//	connect( fSyncFile, SIGNAL( toggled(bool) ), fAbookFile, SLOT( setEnabled(bool) ) );
	connect(fConfigWidget->fBackupOnlyChooser, SIGNAL( clicked() ),
		SLOT( slotSelectNoBackupDBs() ) );
	connect(fConfigWidget->fSkipDBChooser, SIGNAL(clicked()),
		SLOT(slotSelectNoRestoreDBs()));

	(void) kpilotconfigdialog_id;
}

KPilotConfigDialog::~KPilotConfigDialog()
{
	FUNCTIONSETUP;
}

void KPilotConfigDialog::disableUnusedOptions()
{
	FUNCTIONSETUP;
}

void KPilotConfigDialog::readConfig()
{
	FUNCTIONSETUP;

	KPilotConfigSettings & c = KPilotConfig::getConfig();
	c.resetGroup();

	/* General tab in the setup dialog */
	fConfigWidget->fPilotDevice->setText(c.getPilotDevice());
	fConfigWidget->fPilotSpeed->setCurrentItem(c.getPilotSpeed());
	getEncoding(c);
	fConfigWidget->fUserName->setText(c.getUser());
	fConfigWidget->fStartDaemonAtLogin->setChecked(c.getStartDaemonAtLogin());
	fConfigWidget->fDockDaemon->setChecked(c.getDockDaemon());
	fConfigWidget->fKillDaemonOnExit->setChecked(c.getKillDaemonOnExit());

	/* Sync tab */
	int synctype=c.getSyncType();
	if (synctype < SyncAction::eSyncModeLastRadiobutton)
		fConfigWidget->fSyncMode->setButton(synctype);
	else
	{
		fConfigWidget->fSyncMode->setButton(SyncAction::eSyncModeLastRadiobutton);
		fConfigWidget->fSpecialSync->setCurrentItem(synctype-SyncAction::eSyncModeLastRadiobutton);
	}

	fConfigWidget->fFullBackupCheck->setChecked(c.getFullSyncOnPCChange());
	fConfigWidget->fConflictResolution->setCurrentItem(c.getConflictResolution());
	fConfigWidget->fSyncFiles->setChecked(c.getSyncFiles());
	fConfigWidget->fSyncWithKMail->setChecked(c.getSyncWithKMail());

	/* Viewers tab */
	fConfigWidget->fInternalEditors->setChecked(c.getInternalEditors());
	fConfigWidget->fUseSecret->setChecked(c.getShowSecrets());
	c.setAddressGroup();
	fConfigWidget->fAddressGroup->setButton(c.getAddressDisplayMode());
	fConfigWidget->fUseKeyField->setChecked(c.getUseKeyField());
	c.resetGroup();

	/* Backup tab */
	fConfigWidget->fBackupOnly->setText(c.getBackupOnly());
	fConfigWidget->fSkipDB->setText(c.getSkip());
}

/* virtual */ bool KPilotConfigDialog::validate()
{
	int r = KMessageBox::Yes;

#if defined(PILOT_LINK_VERSION) && defined(PILOT_LINK_MAJOR) && defined(PILOT_LINK_MINOR)
#if (PILOT_LINK_VERSION * 100 + PILOT_LINK_MAJOR * 10 + PILOT_LINK_MINOR) < 100
	QString d = fConfigWidget->fPilotDevice->text();

	if (d.length() > 13)
	{
	r = KMessageBox::questionYesNo(
		this,
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

/* virtual */ void KPilotConfigDialog::commitChanges()
{
	FUNCTIONSETUP;

	KPilotConfigSettings & c = KPilotConfig::getConfig();
	c.resetGroup();

	// General page
	c.setPilotDevice(fConfigWidget->fPilotDevice->text());
	c.setPilotSpeed(fConfigWidget->fPilotSpeed->currentItem());
	setEncoding(c);
	c.setUser(fConfigWidget->fUserName->text());
	c.setStartDaemonAtLogin(fConfigWidget->fStartDaemonAtLogin->isChecked());
	c.setDockDaemon(fConfigWidget->fDockDaemon->isChecked());
	c.setKillDaemonOnExit(fConfigWidget->fKillDaemonOnExit->isChecked());

	/* Sync tab */
	int syncmode=fConfigWidget->fSyncMode->id(fConfigWidget->fSyncMode->selected());
	if (syncmode==SyncAction::eSyncModeLastRadiobutton)
		syncmode+=fConfigWidget->fSpecialSync->currentItem();
	c.setSyncType(syncmode);
	c.setFullSyncOnPCChange(fConfigWidget->fFullBackupCheck->isChecked());
	c.setConflictResolution(fConfigWidget->fConflictResolution->currentItem());
	c.setSyncFiles(fConfigWidget->fSyncFiles->isChecked());
	c.setSyncWithKMail(fConfigWidget->fSyncWithKMail->isChecked());

	/* Viewers tab */
	c.setInternalEditors( fConfigWidget->fInternalEditors->isChecked());
	c.setShowSecrets(fConfigWidget->fUseSecret->isChecked());
	c.setAddressGroup();
	c.setAddressDisplayMode(fConfigWidget->fAddressGroup->id(
		fConfigWidget->fAddressGroup->selected()));
	c.setUseKeyField(fConfigWidget->fUseKeyField->isChecked());
	c.resetGroup();

	/* Backup tab */
	c.setBackupOnly(fConfigWidget->fBackupOnly->text());
	c.setSkip(fConfigWidget->fSkipDB->text());

	KPilotConfig::updateConfigVersion();
	c.sync();
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

void KPilotConfigDialog::getEncoding(const KPilotConfigSettings &c)
{
	FUNCTIONSETUP;
	QString e = c.getEncoding();
	if (e.isEmpty())
		fConfigWidget->fPilotEncoding->setCurrentItem(0);
	else
		fConfigWidget->fPilotEncoding->setCurrentText(e);
}

void KPilotConfigDialog::setEncoding(KPilotConfigSettings &c)
{
	FUNCTIONSETUP;

	QString enc = fConfigWidget->fPilotEncoding->currentText();
	if (enc.isEmpty())
	{
		kdWarning() << k_funcinfo << "Empty encoding. Will ignore it"<<endl;
	}
	else
	{
		c.setEncoding(enc);
	}
}

void KPilotConfigDialog::slotSelectNoBackupDBs()
{
	FUNCTIONSETUP;
	KPilotConfigSettings & c = KPilotConfig::getConfig();

	QStringList selectedDBs(QStringList::split(',', fConfigWidget->fBackupOnly->text() ));
	QStringList deviceDBs(c.readListEntry("DeviceDBs"));
	QStringList addedDBs(c.readListEntry("AddedDBsNoBackup"));

	int res;
	KPilotDBSelectionDialog*dlg=new KPilotDBSelectionDialog(selectedDBs, deviceDBs, addedDBs, this, "NoBackupDBs");
	if (dlg && (dlg->exec()==QDialog::Accepted) )
	{
		fConfigWidget->fBackupOnly->setText(
			dlg->getSelectedDBs().join(","));
		c.writeEntry("AddedDBsNoBackup", dlg->getAddedDBs());
	}
	KPILOT_DELETE(dlg);
}

void KPilotConfigDialog::slotSelectNoRestoreDBs()
{
	FUNCTIONSETUP;
	KPilotConfigSettings & c = KPilotConfig::getConfig();

	QStringList selectedDBs(QStringList::split(',', fConfigWidget->fSkipDB->text() ));
	QStringList deviceDBs(c.readListEntry("DeviceDBs"));
	QStringList addedDBs(c.readListEntry("AddedDBsNoRestore"));

	KPilotDBSelectionDialog*dlg=new KPilotDBSelectionDialog(selectedDBs, deviceDBs, addedDBs, this, "NoRestoreDBs");
	if (dlg && (dlg->exec()==QDialog::Accepted) )
	{
		fConfigWidget->fSkipDB->setText(
			dlg->getSelectedDBs().join(","));
		c.writeEntry("AddedDBsNoRestore", dlg->getAddedDBs());
	}
	KPILOT_DELETE(dlg);
}
