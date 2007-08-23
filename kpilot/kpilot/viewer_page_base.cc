/* component_page_base.cc			KPilot
**
** Copyright (C) 2007 Bertjan Broeksema <b.broeksema@kdemail.net>
**
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
#include "viewer_page_base.h"

#include "ui_viewer_page_base.h"

#include "options.h"
#include "pilotDatabase.h"
#include "pilotLocalDatabase.h"
#include "pilotAppInfo.h"

#include "viewer_page_base.moc"

class ViewerPageBase::Private
{
public:
	Private() : fDatabase( 0L ), fAppInfo( 0L ), fWidgetsInitialized( false ) {}
	
	~Private()
	{
		KPILOT_DELETE( fAppInfo );
		KPILOT_DELETE( fDatabase );
	}
	
	/** Database members */
	QString fDbPath;
	QString fDbName;
	PilotDatabase *fDatabase;
	PilotAppInfoBase *fAppInfo;
	
	/** Ui members */
	bool fWidgetsInitialized;
	Ui::ViewerPageBase fWidgetUi;
	QString fInfoLabel;
};

ViewerPageBase::ViewerPageBase( QWidget *parent
	, const QString &dbPath
	, const QString &dbName
	, const QString &infoLabel ) : ComponentPageBase( parent ), fP( new Private )
{
	fP->fDbPath = dbPath;
	fP->fDbName = dbName;
	fP->fInfoLabel = infoLabel;
}

ViewerPageBase::~ViewerPageBase()
{
	delete fP;
}

const QString& ViewerPageBase::dbPath() const
{
	return fP->fDbPath;
}

PilotDatabase* ViewerPageBase::database() const
{
	return fP->fDatabase;
}

void ViewerPageBase::showPage()
{
	FUNCTIONSETUP;
	
	if( !fP->fWidgetsInitialized )
	{
		fP->fWidgetUi.setupUi( this );
		fP->fWidgetUi.fRecordInfoLabel->setText( fP->fInfoLabel );
		fP->fWidgetsInitialized = true;
	}
	
	fP->fDatabase = new PilotLocalDatabase( fP->fDbPath, fP->fDbName );
	fP->fAppInfo = loadAppInfo();
	
	populateCategories();
}

void ViewerPageBase::populateCategories()
{
	FUNCTIONSETUP;
	
	fP->fWidgetUi.fCategories->clear();
	fP->fWidgetUi.fCategories->insertItem( 0
		, i18nc( "This adds the Category all", "All" ), QVariant( -1 ) );

	if( fP->fAppInfo )
	{
		// Fill up the categories list box with
		// the categories defined by the user.
		// These presumably are in the language
		// the user uses, so no translation is necessary.
		for( unsigned int i = 0; i < Pilot::CATEGORY_COUNT; i++ )
		{
			QString catName = fP->fAppInfo->categoryName( i );
			if( !catName.isEmpty() )
			{
				DEBUGKPILOT << "Adding category: " << catName;
				QVariant v( i );
				fP->fWidgetUi.fCategories->addItem( catName, v );
			}
		}
	}
}

void ViewerPageBase::hidePage()
{
	FUNCTIONSETUP;
	
	// Clear the ui
	fP->fWidgetUi.fCategories->clear();
	fP->fWidgetUi.fCategoryList->clear();
	fP->fWidgetUi.fRecordInfo->clear();
	
	// Free some memory
	KPILOT_DELETE( fP->fAppInfo );
	KPILOT_DELETE( fP->fDatabase );
}

