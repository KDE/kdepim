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

#include <kapplication.h>
#include <kconfig.h>

#include "kaddressbookConduit.h"

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
	fConfigWidget->fConflictStrategy->setCurrentItem(
		fConfig->readNumEntry(AbbrowserConduitFactory::syncMode(),0));
	fConfigWidget->fFirstTimeSync->setChecked(
		fConfig->readBoolEntry(AbbrowserConduitFactory::firstSync(),false));
	fConfigWidget->fOtherPhone->setCurrentItem(
		fConfig->readNumEntry(AbbrowserConduitFactory::otherField(),0));
}


// $Log$
// Revision 1.5  2002/06/30 16:23:23  kainhofe
// Started rewriting the addressbook conduit to use libkabc instead of direct dcop communication with abbrowser. Palm->PC is enabled (but still creates duplicate addresses), the rest is completely untested and thus disabled for now
//
// Revision 1.4  2002/05/15 17:15:32  gioele
// kapp.h -> kapplication.h
// I have removed KDE_VERSION checks because all that files included "options.h"
// which #includes <kapplication.h> (which is present also in KDE_2).
// BTW you can't have KDE_VERSION defined if you do not include
// - <kapplication.h>: KDE3 + KDE2 compatible
// - <kdeversion.h>: KDE3 only compatible
//
// Revision 1.3  2002/04/16 18:22:12  adridg
// Wishlist fix from David B: handle formatted names when syncing
//
// Revision 1.2  2001/12/20 22:55:21  adridg
// Making conduits save their configuration and doing syncs
//
// Revision 1.1  2001/10/31 23:54:45  adridg
// CVS_SILENT: Ongoing conduits ports
//
