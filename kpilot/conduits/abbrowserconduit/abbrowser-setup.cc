/* abbrowser-setup.cc                      KPilot
**
** Copyright (C) 2001 by Dan Pilone
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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
** MA 02139, USA.
*/
 
/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <qtabwidget.h>
#include <qcheckbox.h>
#include <qcombobox.h>

#if KDE_VERSION < 300
#include <kapp.h>
#else
#include <kapplication.h>
#endif
#include <kconfig.h>

#include "abbrowserConduitConfig_base.h"

#include "abbrowser-factory.h"
#include "abbrowser-setup.moc"


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

	fConfig->writeEntry(AbbrowserConduitFactory::firstSync(),
		fConfigWidget->fFirstTimeSync->isChecked());
	fConfig->writeEntry(AbbrowserConduitFactory::formatName(),
		fConfigWidget->fFormatName->isChecked());
	fConfig->writeEntry(AbbrowserConduitFactory::closeAbbrowser(),
		fConfigWidget->fCloseKab->isChecked());
	fConfig->writeEntry(AbbrowserConduitFactory::smartMerge(),
		fConfigWidget->fSmartMerge->isChecked());
	fConfig->writeEntry(AbbrowserConduitFactory::conflictResolution(),
		fConfigWidget->fConflictStrategy->currentItem());
	fConfig->writeEntry(AbbrowserConduitFactory::mapOther(),
		fConfigWidget->fOtherPhone->currentItem());
	fConfig->writeEntry(AbbrowserConduitFactory::streetType(),
		fConfigWidget->fAddress->currentItem());
	fConfig->writeEntry(AbbrowserConduitFactory::faxType(),
		fConfigWidget->fFax->currentItem());
}

/* virtual */ void AbbrowserWidgetSetup::readSettings()
{
	FUNCTIONSETUP;

	if (!fConfig) return;

	KConfigGroupSaver s(fConfig,AbbrowserConduitFactory::group());

	fConfigWidget->fFirstTimeSync->setChecked(
		fConfig->readBoolEntry(AbbrowserConduitFactory::firstSync(),false));
	fConfigWidget->fCloseKab->setChecked(
		fConfig->readBoolEntry(AbbrowserConduitFactory::closeAbbrowser(),false));
	fConfigWidget->fSmartMerge->setChecked(
		fConfig->readBoolEntry(AbbrowserConduitFactory::smartMerge(),true));
	fConfigWidget->fConflictStrategy->setCurrentItem(
		fConfig->readNumEntry(AbbrowserConduitFactory::conflictResolution(),0));
	fConfigWidget->fOtherPhone->setCurrentItem(
		fConfig->readNumEntry(AbbrowserConduitFactory::mapOther(),0));
	fConfigWidget->fAddress->setCurrentItem(
		fConfig->readNumEntry(AbbrowserConduitFactory::streetType(),0));
	fConfigWidget->fFax->setCurrentItem(
		fConfig->readNumEntry(AbbrowserConduitFactory::faxType(),0));
}


// $Log$
// Revision 1.2  2001/12/20 22:55:21  adridg
// Making conduits save their configuration and doing syncs
//
// Revision 1.1  2001/10/31 23:54:45  adridg
// CVS_SILENT: Ongoing conduits ports
//
