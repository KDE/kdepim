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
#include "sysinfo-setup.moc"

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


SysInfoWidgetConfig::SysInfoWidgetConfig(QWidget *w, const char *n) :
	ConduitConfigBase(w,n),
	fConfigWidget(new SysInfoWidget(w))
{
	FUNCTIONSETUP;
	UIDialog::addAboutPage(fConfigWidget->tabWidget,SysInfoConduitFactory::about());
	fWidget=fConfigWidget;
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
		fConfig->writeEntry(ci->text(1),ci->isOn());
		i=i->nextSibling();
		ci = dynamic_cast<QCheckListItem *>(i);
	}
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
		i->setOn(fConfig->readBoolEntry(p->key,false));
		i->setText(1,QString::fromLatin1(p->key));
		p++;
	}
}

SysInfoWidgetSetup::SysInfoWidgetSetup(QWidget *w, const char *n,
	const QStringList & a) :
	ConduitConfig(w,n,a)
{
	FUNCTIONSETUP;

	fConfigBase = new SysInfoWidgetConfig(widget(),"ConfigWidget");
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

