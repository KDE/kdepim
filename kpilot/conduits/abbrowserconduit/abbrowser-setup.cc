/* abbrowser-setup.cc                      KPilot
**
** Copyright (C) 2001 by Dan Pilone
** Copyright (C) 2002-2003 Reinhold Kainhofer
**
** This file defines the setup dialog for the abbrowser-conduit plugin.
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
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qbuttongroup.h>

#include <kconfig.h>
#include <kurlrequester.h>

#include "kaddressbookConduit.h"
#include "abbrowser-factory.h"
#include "abbrowser-setup.h"

AbbrowserWidgetSetup::AbbrowserWidgetSetup(QWidget *w, const char *n) :
	ConduitConfigBase(w,n),
	fConfigWidget(new AbbrowserWidget(w))
{
	FUNCTIONSETUP;

	fConduitName=i18n("Addressbook");
	UIDialog::addAboutPage(fConfigWidget->tabWidget,AbbrowserConduitFactory::about());
	fWidget=fConfigWidget;
#define CM(a,b) connect(fConfigWidget->a,b,this,SLOT(modified()));
	CM(fSyncDestination,SIGNAL(clicked(int)));
	CM(fAbookFile,SIGNAL(textChanged(const QString &)));
	CM(fArchive,SIGNAL(toggled(bool)));
	CM(fConflictResolution,SIGNAL(activated(int)));
	CM(fSmartMerge,SIGNAL(toggled(bool)));
	CM(fOtherPhone,SIGNAL(activated(int)));
	CM(fAddress,SIGNAL(activated(int)));
	CM(fFax,SIGNAL(activated(int)));
	CM(fCustom0,SIGNAL(activated(int)));
	CM(fCustom1,SIGNAL(activated(int)));
	CM(fCustom2,SIGNAL(activated(int)));
	CM(fCustom3,SIGNAL(activated(int)));
#undef CM
}

AbbrowserWidgetSetup::~AbbrowserWidgetSetup()
{
	FUNCTIONSETUP;
}

/* virtual */ void AbbrowserWidgetSetup::commit(KConfig *fConfig)
{
	FUNCTIONSETUP;

	if (!fConfig) return;
	KConfigGroupSaver s(fConfig,AbbrowserConduitFactory::group());

	// General page
	fConfig->writeEntry(AbbrowserConduitFactory::fAbookType,
		fConfigWidget->fSyncDestination->id(
			fConfigWidget->fSyncDestination->selected()));
	fConfig->writePathEntry(AbbrowserConduitFactory::fAbookFile,
		fConfigWidget->fAbookFile->url());
	fConfig->writeEntry(AbbrowserConduitFactory::fArchive,
		fConfigWidget->fArchive->isChecked());

	// Conflicts page
	fConfig->writeEntry(AbbrowserConduitFactory::fResolution,
		fConfigWidget->fConflictResolution->currentItem()+SyncAction::eCROffset);
	fConfig->writeEntry(AbbrowserConduitFactory::fSmartMerge,
		fConfigWidget->fSmartMerge->isChecked());

	// Fields page
	fConfig->writeEntry(AbbrowserConduitFactory::fOtherField,
		fConfigWidget->fOtherPhone->currentItem());
	fConfig->writeEntry(AbbrowserConduitFactory::fStreetType,
		fConfigWidget->fAddress->currentItem());
	fConfig->writeEntry(AbbrowserConduitFactory::fFaxType,
		fConfigWidget->fFax->currentItem());

	// Custom fields page
	fConfig->writeEntry(AbbrowserConduitFactory::custom(0),
		fConfigWidget->fCustom0->currentItem());
	fConfig->writeEntry(AbbrowserConduitFactory::custom(1),
		fConfigWidget->fCustom1->currentItem());
	fConfig->writeEntry(AbbrowserConduitFactory::custom(2),
		fConfigWidget->fCustom2->currentItem());
	fConfig->writeEntry(AbbrowserConduitFactory::custom(3),
		fConfigWidget->fCustom3->currentItem());

	unmodified();
}

/* virtual */ void AbbrowserWidgetSetup::load(KConfig *fConfig)
{
	FUNCTIONSETUP;

	if (!fConfig) return;
	KConfigGroupSaver s(fConfig, AbbrowserConduitFactory::group());

	// General page
	fConfigWidget->fSyncDestination->setButton(
		fConfig->readNumEntry(AbbrowserConduitFactory::fAbookType, 0));
	fConfigWidget->fAbookFile->setURL(
		fConfig->readPathEntry(AbbrowserConduitFactory::fAbookFile));
	fConfigWidget->fArchive->setChecked(
		fConfig->readBoolEntry(AbbrowserConduitFactory::fArchive, true));

	// Conflicts page
	fConfigWidget->fConflictResolution->setCurrentItem(
		fConfig->readNumEntry(AbbrowserConduitFactory::fResolution,
		SyncAction::eUseGlobalSetting)-SyncAction::eCROffset);
	fConfigWidget->fSmartMerge->setChecked(
		fConfig->readBoolEntry(AbbrowserConduitFactory::fSmartMerge, true));

	// Fields page
	fConfigWidget->fOtherPhone->setCurrentItem(
		fConfig->readNumEntry(AbbrowserConduitFactory::fOtherField, 0));
	fConfigWidget->fAddress->setCurrentItem(
		fConfig->readNumEntry(AbbrowserConduitFactory::fStreetType, 0));
	fConfigWidget->fFax->setCurrentItem(
		fConfig->readNumEntry(AbbrowserConduitFactory::fFaxType, 0));

	// Custom fields page
	fConfigWidget->fCustom0->setCurrentItem(
		fConfig->readNumEntry(AbbrowserConduitFactory::custom(0)));
	fConfigWidget->fCustom1->setCurrentItem(
		fConfig->readNumEntry(AbbrowserConduitFactory::custom(1)));
	fConfigWidget->fCustom2->setCurrentItem(
		fConfig->readNumEntry(AbbrowserConduitFactory::custom(2)));
	fConfigWidget->fCustom3->setCurrentItem(
		fConfig->readNumEntry(AbbrowserConduitFactory::custom(3)));

	unmodified();
}

/* static */ ConduitConfigBase *AbbrowserWidgetSetup::create(QWidget *w, const char *n)
{
	return new AbbrowserWidgetSetup(w,n);
}

