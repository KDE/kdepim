/* doc-setup.cc                      KPilot
**
** Copyright (C) 2002 by Reinhold Kainhofer
**
** This file defines the setup dialog for the doc-conduit plugin.
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
#include "doc-setup.moc"

#include <qtabwidget.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>

#include <kconfig.h>
#include <kurlrequester.h>

#include "doc-setupdialog.h"
#include "doc-factory.h"



DOCWidgetSetup::DOCWidgetSetup(QWidget * w, const char *n, const QStringList & a):
ConduitConfig(w, n, a)
{
	FUNCTIONSETUP;

	fConfigWidget = new DOCWidget(widget());
	fConfigWidget->fDOCDir->setMode(KFile::Directory);
	fConfigWidget->fPDBDir->setMode(KFile::Directory);
	
	setTabWidget(fConfigWidget->tabWidget);
	addAboutPage(false, DOCConduitFactory::about());

	fConfigWidget->tabWidget->adjustSize();
	fConfigWidget->resize(fConfigWidget->tabWidget->size());
}

DOCWidgetSetup::~DOCWidgetSetup()
{
	FUNCTIONSETUP;
}

/* virtual */ void DOCWidgetSetup::commitChanges()
{
	FUNCTIONSETUP;

	if (!fConfig)
		return;

	KConfigGroupSaver s(fConfig, DOCConduitFactory::fGroup);

	fConfig->writeEntry(DOCConduitFactory::fDOCDir,
		fConfigWidget->fDOCDir->url());
	fConfig->writeEntry(DOCConduitFactory::fPDBDir,
		fConfigWidget->fPDBDir->url());
	fConfig->writeEntry(DOCConduitFactory::fKeepPDBLocally,
		fConfigWidget->fkeepPDBLocally->isChecked());
	fConfig->writeEntry(DOCConduitFactory::fConflictResolution,
		fConfigWidget->fConflictResolution->id(fConfigWidget->
			fConflictResolution->selected()));
	fConfig->writeEntry(DOCConduitFactory::fConvertBookmarks,
		fConfigWidget->fConvertBookmarks->isChecked());
	fConfig->writeEntry(DOCConduitFactory::fBookmarksBmk,
		fConfigWidget->fBookmarksBmk->isChecked());
	fConfig->writeEntry(DOCConduitFactory::fBookmarksInline,
		fConfigWidget->fBookmarksInline->isChecked());
	fConfig->writeEntry(DOCConduitFactory::fBookmarksEndtags,
		fConfigWidget->fBookmarksEndtags->isChecked());
	fConfig->writeEntry(DOCConduitFactory::fCompress,
		fConfigWidget->fCompress->isChecked());
	fConfig->writeEntry(DOCConduitFactory::fSyncDirection,
		fConfigWidget->fSyncDirection->id(fConfigWidget->
			fSyncDirection->selected()));
	fConfig->writeEntry(DOCConduitFactory::fIgnoreBmkChanges, 
		fConfigWidget->fNoConversionOfBmksOnly->isChecked());
	fConfig->writeEntry(DOCConduitFactory::fAlwaysUseResolution,
		fConfigWidget->fAlwaysUseResolution->isChecked());
	
	fConfig->writeEntry(DOCConduitFactory::fPCBookmarks,
		fConfigWidget->fPCBookmarks->id(fConfigWidget->
			fPCBookmarks->selected()));


	fConfig->sync();
}

/* virtual */ void DOCWidgetSetup::readSettings()
{
	FUNCTIONSETUP;

	if (!fConfig)
		return;

	KConfigGroupSaver s(fConfig, DOCConduitFactory::fGroup);

	fConfigWidget->fDOCDir->setURL(fConfig->
		readEntry(DOCConduitFactory::fDOCDir, QString::null));
	fConfigWidget->fPDBDir->setURL(fConfig->
		readEntry(DOCConduitFactory::fPDBDir, QString::null));
	fConfigWidget->fkeepPDBLocally->setChecked(fConfig->
		readBoolEntry(DOCConduitFactory::fKeepPDBLocally, true));
	fConfigWidget->fConflictResolution->setButton(fConfig->
		readNumEntry(DOCConduitFactory::fConflictResolution, 0));
	fConfigWidget->fConvertBookmarks->setChecked(fConfig->
		readBoolEntry(DOCConduitFactory::fConvertBookmarks, true));
	fConfigWidget->fBookmarksBmk->setChecked(fConfig->
		readBoolEntry(DOCConduitFactory::fBookmarksBmk, true));
	fConfigWidget->fBookmarksInline->setChecked(fConfig->
		readBoolEntry(DOCConduitFactory::fBookmarksInline, true));
	fConfigWidget->fBookmarksEndtags->setChecked(fConfig->
		readBoolEntry(DOCConduitFactory::fBookmarksEndtags, true));
	fConfigWidget->fCompress->setChecked(fConfig->
		readBoolEntry(DOCConduitFactory::fCompress, true));
	fConfigWidget->fSyncDirection->setButton(fConfig->
		readNumEntry(DOCConduitFactory::fSyncDirection, 0));
		
	fConfigWidget->fNoConversionOfBmksOnly->setChecked(
		fConfig->readBoolEntry(DOCConduitFactory::fIgnoreBmkChanges, false));
	fConfigWidget->fAlwaysUseResolution->setChecked(
		fConfig->readBoolEntry(DOCConduitFactory::fAlwaysUseResolution, false));
		
	fConfigWidget->fPCBookmarks->setButton(fConfig->
		readNumEntry(DOCConduitFactory::fPCBookmarks, 0));
}

