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
#include "abbrowser-setup.moc"

#include <qtabwidget.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qbuttongroup.h>

#include <kconfig.h>
#include <kurlrequester.h>

#include "kaddressbookConduit.h"
#include "abbrowser-factory.h"

AbbrowserWidgetSetup::AbbrowserWidgetSetup(QWidget *w, const char *n,
	const QStringList & a) :
	ConduitConfig(w,n,a)
{
	FUNCTIONSETUP;

	fConfigWidget = new AbbrowserWidget(widget());
	setTabWidget(fConfigWidget->tabWidget);
	addAboutPage(false,AbbrowserConduitFactory::about());

	fConfigWidget->tabWidget->adjustSize();
	fConfigWidget->resize(fConfigWidget->tabWidget->size());
}

AbbrowserWidgetSetup::~AbbrowserWidgetSetup()
{
	FUNCTIONSETUP;
}

/* virtual */ void AbbrowserWidgetSetup::commitChanges()
{
	FUNCTIONSETUP;

	if (!fConfig) return;
	KConfigGroupSaver s(fConfig,AbbrowserConduitFactory::group());

	// General page
	fConfig->writeEntry(AbbrowserConduitFactory::fAbookType,
		fConfigWidget->fSyncDestination->id(
			fConfigWidget->fSyncDestination->selected()));
	fConfig->writeEntry(AbbrowserConduitFactory::fAbookFile,
		fConfigWidget->fAbookFile->url());
	fConfig->writeEntry(AbbrowserConduitFactory::fSyncMode,
		fConfigWidget->fSyncMode->id(fConfigWidget->fSyncMode->selected()));
	fConfig->writeEntry(AbbrowserConduitFactory::fArchive,
		fConfigWidget->fArchive->isChecked());

	// Conflicts page
	fConfig->writeEntry(AbbrowserConduitFactory::fResolution,
		fConfigWidget->fConflictStrategy->id(
			fConfigWidget->fConflictStrategy->selected()));
	fConfig->writeEntry(AbbrowserConduitFactory::fSmartMerge,
		fConfigWidget->fSmartMerge->isChecked());
	fConfig->writeEntry(AbbrowserConduitFactory::fFirstSync,
		fConfigWidget->fFirstTimeSync->isChecked());

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
}

/* virtual */ void AbbrowserWidgetSetup::readSettings()
{
	FUNCTIONSETUP;

	if (!fConfig) return;
	KConfigGroupSaver s(fConfig,AbbrowserConduitFactory::group());

	// General page
	fConfigWidget->fSyncDestination->setButton(
		fConfig->readNumEntry(AbbrowserConduitFactory::fAbookType, 0));
#ifdef DEBUG
        DEBUGCONDUIT << fname << ": abookType=" 
		<< fConfig->readNumEntry(AbbrowserConduitFactory::fAbookType, 0)
		<< endl;
#endif
	fConfigWidget->fAbookFile->setURL(
		fConfig->readEntry(AbbrowserConduitFactory::fAbookFile));
#ifdef DEBUG
	DEBUGCONDUIT << fname << ": ABookFile="
		<< fConfig->readEntry(AbbrowserConduitFactory::fAbookFile)
		<< endl;
#endif
	fConfigWidget->fSyncMode->setButton(
		fConfig->readNumEntry(AbbrowserConduitFactory::fSyncMode, 0));
	fConfigWidget->fArchive->setChecked(
		fConfig->readBoolEntry(AbbrowserConduitFactory::fArchive, true));

	// Conflicts page
	fConfigWidget->fConflictStrategy->setButton(
		fConfig->readNumEntry(AbbrowserConduitFactory::fResolution, 0));
	fConfigWidget->fSmartMerge->setChecked(
		fConfig->readBoolEntry(AbbrowserConduitFactory::fSmartMerge, true));
	fConfigWidget->fFirstTimeSync->setChecked(
		fConfig->readBoolEntry(AbbrowserConduitFactory::fFirstSync, false));

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
}

