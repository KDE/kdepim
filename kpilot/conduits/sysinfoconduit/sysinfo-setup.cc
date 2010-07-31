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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <tqtabwidget.h>
#include <tqradiobutton.h>
#include <tqcheckbox.h>
#include <tqbuttongroup.h>
#include <tqlistview.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kurlrequester.h>

#include "sysinfo-setup_dialog.h"

#include "sysinfo-factory.h"
#include "sysinfo-setup.h"
#include "sysinfoSettings.h"


typedef struct { const char *name; bool (*accessor)(); void (*mutator)(bool); } sysinfoEntry_t;

const sysinfoEntry_t sysinfoEntries[] =
{
	{ I18N_NOOP("HardwareInfo"), SysinfoSettings::hardwareInfo, SysinfoSettings::setHardwareInfo },
	{ I18N_NOOP("UserInfo"), SysinfoSettings::userInfo, SysinfoSettings::setUserInfo },
	{ I18N_NOOP("MemoryInfo"), SysinfoSettings::memoryInfo, SysinfoSettings::setMemoryInfo },
	{ I18N_NOOP("StorageInfo"), SysinfoSettings::storageInfo, SysinfoSettings::setStorageInfo },
	{ I18N_NOOP("DatabaseList"), SysinfoSettings::databaseList, SysinfoSettings::setDatabaseList },
	{ I18N_NOOP("RecordNumbers"), SysinfoSettings::recordNumbers, SysinfoSettings::setRecordNumbers},
	{ I18N_NOOP("SyncInfo"), SysinfoSettings::syncInfo, SysinfoSettings::setSyncInfo },
	{ I18N_NOOP("KDEVersion"), SysinfoSettings::kDEVersion, SysinfoSettings::setKDEVersion },
	{ I18N_NOOP("PalmOSVersion"), SysinfoSettings::palmOSVersion, SysinfoSettings::setPalmOSVersion },
	{ I18N_NOOP("DebugInformation"), SysinfoSettings::debugInformation, SysinfoSettings::setDebugInformation },
	{ 0L, 0L, 0L }
} ;


/*
** The QCheckListItems used in the list of parts to print have
** several text fields with special meanings.
**    0: The text displayed in the list.
**    1: The index of the item in the sysinfoEntries array.
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
#define updateSetting(i) { TQCheckListItem *ubbu=(i); \
	ubbu->setText(PART_SETTING,(ubbu->isOn() ? CSL1("1") : TQString::null)); }


SysInfoWidgetConfig::SysInfoWidgetConfig(TQWidget *w, const char *n) :
	ConduitConfigBase(w,n),
	fConfigWidget(new SysInfoWidget(w))
{
	FUNCTIONSETUP;

	KAboutData *fAbout = new KAboutData("SysInfoConduit",
		I18N_NOOP("KPilot System Information conduit"),
		KPILOT_VERSION,
		I18N_NOOP("Retrieves System, Hardware, and User Info from the Handheld and stores them to a file."),
		KAboutData::License_GPL,
		"(C) 2003, Reinhold Kainhofer");
	fAbout->addAuthor("Reinhold Kainhofer",
		I18N_NOOP("Primary Author"), "reinhold@kainhofer.com", "http://reinhold.kainhofer.com/");

	ConduitConfigBase::addAboutPage(fConfigWidget->tabWidget,fAbout);
	fWidget=fConfigWidget;

	TQObject::connect(fConfigWidget->fOutputFile,TQT_SIGNAL(textChanged(const TQString&)),
		this,TQT_SLOT(modified()));
	TQObject::connect(fConfigWidget->fTemplateFile,TQT_SIGNAL(textChanged(const TQString&)),
		this,TQT_SLOT(modified()));
	TQObject::connect(fConfigWidget->fOutputType,TQT_SIGNAL(clicked(int)),
		this,TQT_SLOT(modified()));
	fConduitName=i18n("System Information");
}

void SysInfoWidgetConfig::commit()
{
	FUNCTIONSETUP;

	SysinfoSettings::setOutputFile(
		fConfigWidget->fOutputFile->url() );
	SysinfoSettings::setTemplateFile(
		fConfigWidget->fTemplateFile->url() );
	SysinfoSettings::setOutputFormat(
		fConfigWidget->fOutputType->id(fConfigWidget->fOutputType->selected()));

	TQListViewItem *i = fConfigWidget->fPartsList->firstChild();
	TQCheckListItem *ci = dynamic_cast<TQCheckListItem *>(i);
	while(ci)
	{
#ifdef DEBUG
		DEBUGKPILOT << fname << ": Saving " << ci->text(PART_NAME)
			<< (ci->isOn() ? " on" : " off") << endl;
#endif
		int index=ci->text(PART_KEY).toInt();
		if (0<=index && index<=10)
		{
			const sysinfoEntry_t *p = sysinfoEntries+index;
			p->mutator(ci->isOn());
		}
		updateSetting(ci);
		i=i->nextSibling();
		ci = dynamic_cast<TQCheckListItem *>(i);
	}
	SysinfoSettings::self()->writeConfig();
	unmodified();
}

void SysInfoWidgetConfig::load()
{
	FUNCTIONSETUP;
	SysinfoSettings::self()->readConfig();

	const sysinfoEntry_t *p = sysinfoEntries;
	TQCheckListItem *i = 0L;
	while (p && p->name)
	{
		i = new TQCheckListItem(fConfigWidget->fPartsList,i18n(p->name),TQCheckListItem::CheckBox);
		// by default let the sysinfo conduit write out all available information
		i->setOn( p->accessor() );
		i->setText(PART_KEY, TQString::number(p-sysinfoEntries)); // store index there
		updateSetting(i);
#ifdef DEBUG
		DEBUGKPILOT << fname << ": Loaded " << p->name
			<< (i->isOn() ? " on" : " off") << endl;
#endif

		p++;
	}
  fConfigWidget->fOutputFile->setURL( SysinfoSettings::outputFile() );
	fConfigWidget->fTemplateFile->setURL( SysinfoSettings::templateFile() );
	fConfigWidget->fOutputType->setButton( SysinfoSettings::outputFormat() );
	unmodified();
}

/* virtual */ bool SysInfoWidgetConfig::isModified() const
{
	FUNCTIONSETUP;
	if (fModified) return true;

	TQListViewItem *i = fConfigWidget->fPartsList->firstChild();
	TQCheckListItem *ci = dynamic_cast<TQCheckListItem *>(i);

	while(ci)
	{
		bool current = ci->isOn();
		bool original = !ci->text(PART_SETTING).isEmpty();
#ifdef DEBUG
		DEBUGKPILOT << fname << ": Checking " << ci->text(PART_KEY)
			<<  " was " << (original ? " on" : " off")
			<< " now " << (current ? " on" : " off") << endl;
#endif

		if (current!=original) return true;
		i=i->nextSibling();
		ci = dynamic_cast<TQCheckListItem *>(i);
	}
	return false;
}
