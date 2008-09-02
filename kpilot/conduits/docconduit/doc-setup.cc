/* KPilot
**
** Copyright (C) 2002 by Reinhold Kainhofer <reinhold@kainhofer.com>
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "doc-setup.h"

#include <qtabwidget.h>
#include <qcheckbox.h>
#include <q3buttongroup.h>
#include <qcombobox.h>

#include <kaboutdata.h>
#include <kcharsets.h>
#include <kconfig.h>
#include <kurlrequester.h>

#include "doc-setup.moc"
#include "docconduitSettings.h"
#include "options.h"

DOCWidgetConfig::DOCWidgetConfig(QWidget * w):
	ConduitConfigBase(w, const QVariantList &),
	fConfigWidget(new DOCWidget(w))
{
	FUNCTIONSETUP;

	fWidget=fConfigWidget;

	QStringList l = KGlobal::charsets()->descriptiveEncodingNames();
	for ( QStringList::Iterator it = l.begin(); it != l.end(); ++it)
	{
		fConfigWidget->fEncoding->insertItem(*it);
	}

	fAbout = new KAboutData("docconduit", 0,
		ki18n("Palm DOC Conduit for KPilot"), KPILOT_VERSION,
		ki18n("Configures the DOC Conduit for KPilot"),
		KAboutData::License_GPL, ki18n("(C) 2002, Reinhold Kainhofer"));

	fAbout->addAuthor(ki18n("Reinhold Kainhofer"),
		ki18n("Maintainer"), "reinhold@kainhofer.com",
		"http://reinhold.kainhofer.com");

	fConfigWidget->fTXTDir->setMode(KFile::Directory);
	fConfigWidget->fPDBDir->setMode(KFile::Directory);
	ConduitConfigBase::addAboutPage(fConfigWidget->tabWidget, fAbout);

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

	DOCConduitSettings::setTXTDirectory( fConfigWidget->fTXTDir->url().url() );
	DOCConduitSettings::setPDBDirectory( fConfigWidget->fPDBDir->url().url() );

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

	fConfigWidget->fTXTDir->setUrl( DOCConduitSettings::tXTDirectory() );
	fConfigWidget->fPDBDir->setUrl( DOCConduitSettings::pDBDirectory() );
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

