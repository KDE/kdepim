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
#include <kapplication.h>
#include <kconfig.h>
#include <kurlrequester.h>

#include "sysinfo-setup_dialog.h"

#include "sysinfo-factory.h"
#include "sysinfo-setup.moc"

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
	fConfig->writeEntry(SysInfoConduitFactory::fHardwareInfo,
		fConfigWidget->fHardwareInfo->isChecked());
	fConfig->writeEntry(SysInfoConduitFactory::fUserInfo,
		fConfigWidget->fUserInfo->isChecked());
	fConfig->writeEntry(SysInfoConduitFactory::fMemoryInfo,
		fConfigWidget->fMemoryInfo->isChecked());
	fConfig->writeEntry(SysInfoConduitFactory::fStorageInfo,
		fConfigWidget->fStorageInfo->isChecked());
	fConfig->writeEntry(SysInfoConduitFactory::fDBList,
		fConfigWidget->fDBList->isChecked());
	fConfig->writeEntry(SysInfoConduitFactory::fRecordNumber,
		fConfigWidget->fRecordNumber->isChecked());
	fConfig->writeEntry(SysInfoConduitFactory::fSyncInfo,
		fConfigWidget->fSyncInfo->isChecked());
	fConfig->writeEntry(SysInfoConduitFactory::fKDEVersion,
		fConfigWidget->fKDEVersion->isChecked());
	fConfig->writeEntry(SysInfoConduitFactory::fPalmOSVersion,
		fConfigWidget->fPalmOSVersion->isChecked());
	fConfig->writeEntry(SysInfoConduitFactory::fDebugInfo,
		fConfigWidget->fDebugInfo->isChecked());
}

void SysInfoWidgetConfig::load(KConfig *fConfig)
{
	FUNCTIONSETUP;
	KConfigGroupSaver s(fConfig,SysInfoConduitFactory::fGroup);
	fConfigWidget->fOutputFile->setURL(fConfig->readEntry(SysInfoConduitFactory::fOutputFile));
	fConfigWidget->fTemplateFile->setURL(fConfig->readEntry(SysInfoConduitFactory::fTemplateFile));
	fConfigWidget->fOutputType->setButton(fConfig->readNumEntry(SysInfoConduitFactory::fOutputType, 0));
	fConfigWidget->fHardwareInfo->setChecked(fConfig->readBoolEntry(SysInfoConduitFactory::fHardwareInfo, true));
	fConfigWidget->fUserInfo->setChecked(fConfig->readBoolEntry(SysInfoConduitFactory::fUserInfo, true));
	fConfigWidget->fMemoryInfo->setChecked(fConfig->readBoolEntry(SysInfoConduitFactory::fMemoryInfo, true));
	fConfigWidget->fStorageInfo->setChecked(fConfig->readBoolEntry(SysInfoConduitFactory::fStorageInfo, true));
	fConfigWidget->fDBList->setChecked(fConfig->readBoolEntry(SysInfoConduitFactory::fDBList, true));
	fConfigWidget->fRecordNumber->setChecked(fConfig->readBoolEntry(SysInfoConduitFactory::fRecordNumber, true));
	fConfigWidget->fSyncInfo->setChecked(fConfig->readBoolEntry(SysInfoConduitFactory::fSyncInfo, true));
	fConfigWidget->fKDEVersion->setChecked(fConfig->readBoolEntry(SysInfoConduitFactory::fKDEVersion, true));
	fConfigWidget->fPalmOSVersion->setChecked(fConfig->readBoolEntry(SysInfoConduitFactory::fPalmOSVersion, true));
	fConfigWidget->fDebugInfo->setChecked(fConfig->readBoolEntry(SysInfoConduitFactory::fDebugInfo, true));
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

