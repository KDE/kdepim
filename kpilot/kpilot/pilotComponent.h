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

struct CategoryAppInfo;
class QComboBox;
class KPilotLink;

class PilotComponent : public QWidget
    {
    Q_OBJECT

    public:
    PilotComponent(QWidget* parent);
      
      virtual void initialize() = 0; // Load data from files, etc.
      //    virtual bool doHotSync(KPilotLink*) = 0; // True if successful
      virtual void preHotSync(char* ) { } // Prepare for hotsync
      // and append any commands to send to the pilot onto the char* array
      virtual void postHotSync() { } // Reload data, etc, etc
    virtual bool saveData() = 0; // true if everything is saved
//     virtual bool hotSyncNeeded() = 0; // True if component wants to hot-sync
//     virtual void enableHotSync(bool) = 0; // If false, hotSyncNeeded() will always be false.

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

    };

#endif



// $Log:$
