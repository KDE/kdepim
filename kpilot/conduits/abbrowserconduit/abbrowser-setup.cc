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

#include <kconfig.h>

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

	fConfig->writeEntry(AbbrowserConduitFactory::smartMerge(),
		fConfigWidget->fSmartMerge->isChecked());
	fConfig->writeEntry(AbbrowserConduitFactory::conflictResolution(),
		fConfigWidget->fConflictStrategy->currentItem());
	fConfig->writeEntry(AbbrowserConduitFactory::archiveDeletedRecs(),
		fConfigWidget->fArchive->isChecked());
	fConfig->writeEntry(AbbrowserConduitFactory::streetType(),
		fConfigWidget->fAddress->currentItem());
	fConfig->writeEntry(AbbrowserConduitFactory::faxType(),
		fConfigWidget->fFax->currentItem());
	fConfig->writeEntry(AbbrowserConduitFactory::syncMode(),
		fConfigWidget->fSyncMode->currentItem());
	fConfig->writeEntry(AbbrowserConduitFactory::firstSync(),
		fConfigWidget->fFirstTimeSync->isChecked());
	fConfig->writeEntry(AbbrowserConduitFactory::otherField(),
		fConfigWidget->fOtherPhone->currentItem());
		
	fConfig->writeEntry(AbbrowserConduitFactory::custom().arg(0),
		fConfigWidget->fCustom0->currentItem());
	fConfig->writeEntry(AbbrowserConduitFactory::custom().arg(1),
		fConfigWidget->fCustom1->currentItem());
	fConfig->writeEntry(AbbrowserConduitFactory::custom().arg(2),
		fConfigWidget->fCustom2->currentItem());
	fConfig->writeEntry(AbbrowserConduitFactory::custom().arg(3),
		fConfigWidget->fCustom3->currentItem());
}

/* virtual */ void AbbrowserWidgetSetup::readSettings()
{
	FUNCTIONSETUP;

	if (!fConfig) return;

	KConfigGroupSaver s(fConfig,AbbrowserConduitFactory::group());

	fConfigWidget->fSmartMerge->setChecked(
		fConfig->readBoolEntry(AbbrowserConduitFactory::smartMerge(),true));
	fConfigWidget->fConflictStrategy->setCurrentItem(
		fConfig->readNumEntry(AbbrowserConduitFactory::conflictResolution(),0));
	fConfigWidget->fArchive->setChecked(
		fConfig->readBoolEntry(AbbrowserConduitFactory::archiveDeletedRecs(),true));
	fConfigWidget->fAddress->setCurrentItem(
		fConfig->readNumEntry(AbbrowserConduitFactory::streetType(),0));
	fConfigWidget->fFax->setCurrentItem(
		fConfig->readNumEntry(AbbrowserConduitFactory::faxType(),0));
	fConfigWidget->fSyncMode->setCurrentItem(
		fConfig->readNumEntry(AbbrowserConduitFactory::syncMode(),0));
	fConfigWidget->fFirstTimeSync->setChecked(
		fConfig->readBoolEntry(AbbrowserConduitFactory::firstSync(),false));
	fConfigWidget->fOtherPhone->setCurrentItem(
		fConfig->readNumEntry(AbbrowserConduitFactory::otherField(),0));
		
	fConfigWidget->fCustom0->setCurrentItem(
		fConfig->readNumEntry(AbbrowserConduitFactory::custom().arg(0)));
	fConfigWidget->fCustom1->setCurrentItem(
		fConfig->readNumEntry(AbbrowserConduitFactory::custom().arg(1)));
	fConfigWidget->fCustom2->setCurrentItem(
		fConfig->readNumEntry(AbbrowserConduitFactory::custom().arg(2)));
	fConfigWidget->fCustom3->setCurrentItem(
		fConfig->readNumEntry(AbbrowserConduitFactory::custom().arg(3)));
}

