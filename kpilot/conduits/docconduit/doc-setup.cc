/* KPilot
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

#include <qtabwidget.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qcombobox.h>

#include <kconfig.h>
#include <kurlrequester.h>
#include <kcharsets.h>

#include "doc-setupdialog.h"
#include "doc-factory.h"
#include "doc-setup.h"
#include "docconduitSettings.h"


DOCWidgetConfig::DOCWidgetConfig(QWidget * w, const char *n):
	ConduitConfigBase(w, n),
	fConfigWidget(new DOCWidget(w))
{
	FUNCTIONSETUP;

	fWidget=fConfigWidget;

	QStringList l = KGlobal::charsets()->descriptiveEncodingNames();
	for ( QStringList::Iterator it = l.begin(); it != l.end(); ++it)
	{
		fConfigWidget->fEncoding->insertItem(*it);
	}

	fConfigWidget->fTXTDir->setMode(KFile::Directory);
	fConfigWidget->fPDBDir->setMode(KFile::Directory);
	UIDialog::addAboutPage(fConfigWidget->tabWidget,DOCConduitFactory::about());

	fConduitName=i18n("Palm DOC");

#define CMOD(a,b) connect(fConfigWidget->a,SIGNAL(b),this,SLOT(modified()))
	CMOD(fTXTDir,textChanged(const QString &));
	CMOD(fPDBDir,textChanged(const QString &));
	CMOD(fkeepPDBLocally,clicked());
	CMOD(fConflictResolution,clicked(int));
	CMOD(fConvertBookmarks,stateChanged(int));
	CMOD(fBookmarksBmk,stateChanged(int));
	CMOD(fBookmarksInline,stateChanged(int));
	CMOD(fBookmarksEndtags,stateChanged(int));
	CMOD(fCompress,stateChanged(int));
	CMOD(fSyncDirection,clicked(int));
	CMOD(fNoConversionOfBmksOnly,stateChanged(int));
	CMOD(fAlwaysUseResolution,stateChanged(int));
	CMOD(fPCBookmarks,clicked(int));
	CMOD(fEncoding,textChanged(const QString &));
#undef CMOD

	fConfigWidget->adjustSize();
}

/* virtual */ void DOCWidgetConfig::commit()
{
	FUNCTIONSETUP;

	DOCConduitSettings::setTXTDirectory( fConfigWidget->fTXTDir->url() );
	DOCConduitSettings::setPDBDirectory( fConfigWidget->fPDBDir->url() );

	DOCConduitSettings::setKeepPDBsLocally( fConfigWidget->fkeepPDBLocally->isChecked());
	DOCConduitSettings::setConflictResolution( fConfigWidget->fConflictResolution->id(
		fConfigWidget->fConflictResolution->selected()) );
	DOCConduitSettings::setConvertBookmarks(fConfigWidget->fConvertBookmarks->isChecked());
	DOCConduitSettings::setBmkFileBookmarks(fConfigWidget->fBookmarksBmk->isChecked());
	DOCConduitSettings::setInlineBookmarks(fConfigWidget->fBookmarksInline->isChecked());
	DOCConduitSettings::setEndtagBookmarks(fConfigWidget->fBookmarksEndtags->isChecked());
	DOCConduitSettings::setCompress(fConfigWidget->fCompress->isChecked());
	DOCConduitSettings::setSyncDirection(fConfigWidget->fSyncDirection->id(
		fConfigWidget->fSyncDirection->selected()));
	DOCConduitSettings::setIgnoreBmkChanges(fConfigWidget->fNoConversionOfBmksOnly->isChecked());
	DOCConduitSettings::setAlwaysShowResolutionDialog(fConfigWidget->fAlwaysUseResolution->isChecked());
	DOCConduitSettings::setBookmarksToPC( fConfigWidget->fPCBookmarks->id(
		fConfigWidget->fPCBookmarks->selected()) );
	DOCConduitSettings::setEncoding( fConfigWidget->fEncoding->currentText() );

	DOCConduitSettings::self()->writeConfig();
	unmodified();
}

/* virtual */ void DOCWidgetConfig::load()
{
	FUNCTIONSETUP;
	DOCConduitSettings::self()->readConfig();

	fConfigWidget->fTXTDir->setURL( DOCConduitSettings::tXTDirectory() );
	fConfigWidget->fPDBDir->setURL( DOCConduitSettings::pDBDirectory() );
	fConfigWidget->fkeepPDBLocally->setChecked( DOCConduitSettings::keepPDBsLocally() );
	fConfigWidget->fConflictResolution->setButton(DOCConduitSettings::conflictResolution() );
	fConfigWidget->fConvertBookmarks->setChecked(DOCConduitSettings::convertBookmarks() );
	fConfigWidget->fBookmarksBmk->setChecked(DOCConduitSettings::bmkFileBookmarks() );
	fConfigWidget->fBookmarksInline->setChecked(DOCConduitSettings::inlineBookmarks() );
	fConfigWidget->fBookmarksEndtags->setChecked(DOCConduitSettings::endtagBookmarks() );
	fConfigWidget->fCompress->setChecked(DOCConduitSettings::compress() );
	fConfigWidget->fSyncDirection->setButton(DOCConduitSettings::syncDirection() );

	fConfigWidget->fNoConversionOfBmksOnly->setChecked( DOCConduitSettings::ignoreBmkChanges() );
	fConfigWidget->fAlwaysUseResolution->setChecked( DOCConduitSettings::alwaysShowResolutionDialog() );

	fConfigWidget->fPCBookmarks->setButton(DOCConduitSettings::bookmarksToPC() );
	fConfigWidget->fEncoding->setCurrentText(DOCConduitSettings::encoding() );
	unmodified();
}

