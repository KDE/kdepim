/* pilotCOmponent.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/
#ifndef __PILOT_COMPONENT_H
#define __PILOT_COMPONENT_H

/**
  * Base class for any module to KPilot
  */
#include <qwidget.h>
#include <qstring.h>

struct CategoryAppInfo;
class QComboBox;

class PilotComponent : public QWidget
{
Q_OBJECT

public:
	PilotComponent(QWidget* parent, 
		const QString& dbPath);

	/**
	* Load data from files, etc. Always called
	* before the component is made visible the first time.
	*/
	virtual void initialize() = 0;

	/**
	* Get ready for a hotsync -- write any unflushed records
	* to disk, close windows, whatever. Append commands to send
	* to the pilot onto the char * array.
	*/
	virtual void preHotSync(char* ) { } ;

	/**
	* Reload data (possibly changed by the hotsync) etc. etc.
	*/
	virtual void postHotSync() { } ;

	/**
	* Save data to local disk (?), return true if succesful.
	*/
	virtual bool saveData() = 0;


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

	const QString& dbPath() const { return fDBPath; } ;

private:
	QString fDBPath;
} ;

#endif



// $Log$
// Revision 1.5  2001/02/06 08:05:20  adridg
// Fixed copyright notices, added CVS log, added surrounding #ifdefs. No code changes.
//
