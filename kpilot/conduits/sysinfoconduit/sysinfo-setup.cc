/* SysInfo-setup.cc                      KPilot
**
** Copyright (C) 2003 by Reinhold Kainhofer
**
** This file defines the setup dialog for the SysInfo-conduit plugin.
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

#include "options.h"

#include <qtabwidget.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qlistview.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kurlrequester.h>

#include "sysinfo-setup_dialog.h"

#include "sysinfo-factory.h"
#include "sysinfo-setup.h"

typedef struct { const char *name; const char *key; } sysinfoEntry_t;

const sysinfoEntry_t sysinfoEntries[] =
{
	{ I18N_NOOP("Hardware information"),
		SysInfoConduitFactory::fHardwareInfo },
	{ I18N_NOOP("User information"),
		SysInfoConduitFactory::fUserInfo },
	{ I18N_NOOP("Memory information"),
		SysInfoConduitFactory::fMemoryInfo },
	{ I18N_NOOP("Storage info (SD card, memory stick, ...)"),
		SysInfoConduitFactory::fStorageInfo },
	{ I18N_NOOP("List of databases on handheld (takes long!)"),
		SysInfoConduitFactory::fDBList },
	{ I18N_NOOP("Number of addresses, todos, events and memos"),
		SysInfoConduitFactory::fRecordNumber },
	{ I18N_NOOP("Synchronization information"),
		SysInfoConduitFactory::fSyncInfo },
	{ I18N_NOOP("Version of KPilot, pilot-link and KDE"),
		SysInfoConduitFactory::fKDEVersion },
	{ I18N_NOOP("PalmOS version"),
		SysInfoConduitFactory::fPalmOSVersion },
	{ I18N_NOOP("Debug information (for KPilot developers)"),
		SysInfoConduitFactory::fDebugInfo },
	{ 0L,0L }
} ;


/*
** The QCheckListItems used in the list of parts to print have
** several text fields with special meanings.
**    0: The text displayed in the list.
**    1: The KConfig key the item came from.
**    2: This string is empty if the part was originally not checked,
**       and non-empty (probably "1") if the part was originally checked.
**       This is used to detect changes in the configuration.
** We introduce some defines for these numbers.
*/

#define PART_NAME	(0)
#define PART_KEY	(1)
#define PART_SETTING	(2)

/*
** This is a convenience define to update an item's "original setting".
*/
#define updateSetting(i) { QCheckListItem *ubbu=(i); \
	ubbu->setText(PART_SETTING,(ubbu->isOn() ? CSL1("1") : QString::null)); }


SysInfoWidgetConfig::SysInfoWidgetConfig(QWidget *w, const char *n) :
	ConduitConfigBase(w,n),
	fConfigWidget(new SysInfoWidget(w))
{
	FUNCTIONSETUP;
	UIDialog::addAboutPage(fConfigWidget->tabWidget,SysInfoConduitFactory::about());
	fWidget=fConfigWidget;

	QObject::connect(fConfigWidget->fOutputFile,SIGNAL(textChanged(const QString&)),
		this,SLOT(modified()));
	QObject::connect(fConfigWidget->fTemplateFile,SIGNAL(textChanged(const QString&)),
		this,SLOT(modified()));
	QObject::connect(fConfigWidget->fOutputType,SIGNAL(clicked(int)),
		this,SLOT(modified()));
	fConduitName=i18n("System Information");
}

void SysInfoWidgetConfig::commit(KConfig *fConfig)
{
	FUNCTIONSETUP;
	KConfigGroupSaver s(fConfig,SysInfoConduitFactory::fGroup);
	fConfig->writeEntry(SysInfoConduitFactory::fOutputFile,
		fConfigWidget->fOutputFile->url());
	fConfig->writeEntry(SysInfoConduitFactory::fTemplateFile,
		fConfigWidget->fTemplateFile->url());
	fConfig->writeEntry(SysInfoConduitFactory::fOutputType,
		fConfigWidget->fOutputType->id(fConfigWidget->fOutputType->selected()));

	QListViewItem *i = fConfigWidget->fPartsList->firstChild();
	QCheckListItem *ci = dynamic_cast<QCheckListItem *>(i);

	while(ci)
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": Saving " << ci->text(PART_KEY)
			<< (ci->isOn() ? " on" : " off") << endl;
#endif
		fConfig->writeEntry(ci->text(PART_KEY),ci->isOn());
		updateSetting(ci);
		i=i->nextSibling();
		ci = dynamic_cast<QCheckListItem *>(i);
	}
	unmodified();
}

void SysInfoWidgetConfig::load(KConfig *fConfig)
{
	FUNCTIONSETUP;
	KConfigGroupSaver s(fConfig,SysInfoConduitFactory::fGroup);
	fConfigWidget->fOutputFile->setURL(fConfig->readEntry(SysInfoConduitFactory::fOutputFile));
	fConfigWidget->fTemplateFile->setURL(fConfig->readEntry(SysInfoConduitFactory::fTemplateFile));
	fConfigWidget->fOutputType->setButton(fConfig->readNumEntry(SysInfoConduitFactory::fOutputType, 0));

	const sysinfoEntry_t *p = sysinfoEntries;
	QCheckListItem *i = 0L;
	while (p && p->name)
	{
		i = new QCheckListItem(fConfigWidget->fPartsList,i18n(p->name),QCheckListItem::CheckBox);
		// by default let the sysinfo conduit write out all available information
		i->setOn(fConfig->readBoolEntry(p->key, true));
		i->setText(PART_KEY,QString::fromLatin1(p->key));
		updateSetting(i);
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": Loaded " << p->key
			<< (i->isOn() ? " on" : " off") << endl;
#endif
		p++;
	}
	unmodified();
}

/* virtual */ bool SysInfoWidgetConfig::isModified() const
{
	FUNCTIONSETUP;
	if (fModified) return true;

	QListViewItem *i = fConfigWidget->fPartsList->firstChild();
	QCheckListItem *ci = dynamic_cast<QCheckListItem *>(i);

	while(ci)
	{
		bool current = ci->isOn();
		bool original = !ci->text(PART_SETTING).isEmpty();
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": Checking " << ci->text(PART_KEY)
			<<  " was " << (original ? " on" : " off")
			<< " now " << (current ? " on" : " off") << endl;
#endif

		if (current!=original) return true;
		i=i->nextSibling();
		ci = dynamic_cast<QCheckListItem *>(i);
	}
	return false;
}

SysInfoWidgetSetup::SysInfoWidgetSetup(QWidget *w, const char *n,
	const QStringList & a) :
	ConduitConfig(w,n,a)
{
	FUNCTIONSETUP;

	fConfigBase = new SysInfoWidgetConfig(widget(),"ConfigWidget");
	fConduitName = i18n("System Information");
}

SysInfoWidgetSetup::~SysInfoWidgetSetup()
{
	FUNCTIONSETUP;
}

/* virtual */ void SysInfoWidgetSetup::commitChanges()
{
	FUNCTIONSETUP;

	if (!fConfig) return;
	fConfigBase->commit(fConfig);
}

/* virtual */ void SysInfoWidgetSetup::readSettings()
{
	FUNCTIONSETUP;

	if (!fConfig) return;
	fConfigBase->load(fConfig);
}

