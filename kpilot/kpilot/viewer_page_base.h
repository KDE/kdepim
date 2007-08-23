#ifndef VIEWER_PAGE_BASE_H
#define VIEWER_PAGE_BASE_H
/* viewer_page_base.h			KPilot
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

#include "component_page_base.h"

class PilotAppInfoBase;
class PilotDatabase;
class PilotRecord;

class ViewerPageBase : public ComponentPageBase
{
Q_OBJECT

public:
	ViewerPageBase( QWidget *parent
		, const QString &dbPath
		, const QString &dbName
		, const QString &infoLabel );
	
	virtual ~ViewerPageBase();
	
	const QString& dbPath() const;

protected:

	PilotDatabase* database() const;

	/**
	 * Loads the appInfo block.
	 */
	virtual PilotAppInfoBase* loadAppInfo() = 0;
	
	/**
	 * Loads the appInfo block.
	 */
	virtual QString getListHeader( PilotRecord* rec ) { return QString(""); }

public slots:
	/**
	 * Called when the component is shown in kpilot. It should load everything it
	 * needs and populate the widgets.
	 */
	virtual void showPage();
	
	/**
	 * Called when the component is hidden in kpilot. It shoul clean up to save
	 * memory.
	 */
	virtual void hidePage();

protected slots:
	
	/**
	 * Changes the filter of the category list with the value of the combobox at
	 * @p index.
	 */
	void changeFilter( int index );

private: // methods
	void populateCategories();
	
	void populateRecords();

private: // members
	class Private;
	Private *fP;
};

#endif
