#ifndef _KPILOT_PILOTCOMPONENT_H
#define _KPILOT_PILOTCOMPONENT_H
/* pilotComponent.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** See the .cc file for an explanation of what this file is for.
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

/**
  * Base class for any module to KPilot
  */
#include <qwidget.h>

struct CategoryAppInfo;
class QComboBox;
class QString;

class PilotComponent : public QWidget
{
Q_OBJECT
friend class KPilotInstaller;

public:
	PilotComponent(QWidget* parent,
		const char *id,
		const QString& dbPath);

	/**
	* Called when the component is shown in kpilot. It should
	* load the database and populate the widgets.
	*/
	virtual void showComponent() {}
	/**
	* Called when the component is hidden in kpilot. It should
	* unload the databases and clean up to save memory. This method
	* can be called even if the component is not visible.
	* If there are some editing dlgs open, this needs to be deferred
	* until they are all closed. Then, one can explicitly call hideComponent().
	*/
	virtual void hideComponent() {}

	/**
	* Set the shown variable to true or false, then call showComponent
	* or hideComponent.
	*/
	void showKPilotComponent( bool toShow );


	/**
	* Get ready for a hotsync -- write any unflushed records
	* to disk, close windows, whatever. Returns false if it
	* is impossible to go into a sync now (due to open windows
	* or strange state.).
	*
	* The default implementation returns true.
	*
	* If the function returns false, it can also put a string
	* stating the reason why into @p s. This string will be
	* displayed to the user:
	*     "Can't start HotSync. %1"
	* where %1 is replaced by s.
	*/
	virtual bool preHotSync(QString &s) ;

	/**
	* Reload data (possibly changed by the hotsync) etc. etc.
	*/
	virtual void postHotSync() { } ;


protected:
	/**
	* Look up the selected category from the combo box in the
	* Pilot's register of categories. We need this functon because
	* the combo box doesn't contain any reference to the category
	* ID, and we need that ID to do anything with the Pilot.
	*
	* If AllIsUnfiled is true, then when the user selects the
	* category "All" in the combo box (always the first category),
	* Unfiled (0) is returned. Otherwise if the category "All"
	* is selected -1 is returned. For all other categories
	* selected, their ID is returned. If nothing is selected,
	* behave as if "All" is selected.
	*/
	int findSelectedCategory(QComboBox *,
		CategoryAppInfo *,
		bool AllIsUnfiled=false);

	/**
	* Populate the combo box with the categories found in
	* the Pilot's application categories block. Erases
	* combo box's contents first.
	*
	* Always includes the category "All" as the first
	* entry in the combo box.
	*
	* If info is a NULL pointer, just put "All" in the combo box.
	*/
	void populateCategories(QComboBox *,
		CategoryAppInfo *info=0);

	void setDBPath(const QString &path) { fDBPath = path; } ;
	const QString& dbPath() const { return fDBPath; } ;
	void markDBDirty(const QString db);

public slots:
	void slotShowComponent();

signals:
	void showComponent(PilotComponent *);

private:
	QString fDBPath;
protected:
	bool shown;
} ;

#endif
