/* pilotComponent.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This file defines a base class for components -- internal conduits --
** in KPilot. This includes a number of general utility functions.
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


#include "options.h"

#include <time.h>
#include <iostream.h>

#ifndef _PILOT_APPINFO_H_
#include <pi-appinfo.h>
#endif

#ifndef QWIDGET_H
#include <qwidget.h>
#endif

#ifndef QCOMBOBOX_H
#include <qcombobox.h>
#endif

#ifndef _KDEBUG_H
#include <kdebug.h>
#endif

#include "pilotComponent.moc"


static const char *pilotComponent_id =
	"$Id$";

// This is a pilot constant and should probably be defined
// in a more sensible place but I'm lazy right now.
//
#define MAX_CATEGORIES	(15)

PilotComponent::PilotComponent(QWidget* parent,
	const char *id,
	const QString &path) : 
	QWidget(parent,id),
	fDBPath(path)
{
	(void) pilotComponent_id;
}



int PilotComponent::findSelectedCategory(QComboBox *fCatList,
	struct CategoryAppInfo *info,
	bool AllIsUnfiled)
{
	FUNCTIONSETUP;

	// Semantics of currentCatID are: 
	//
	// >=0		is a specific category based on the text -> 
	//		category number mapping defined by the Pilot, 
	// ==-1 	means "All" category selected when 
	//		AllIsUnfiled is true.
	// == 0		== Unfiled means "All" category selected when 
	//		AllIsUnfiled is false.
	//
	//
	int currentCatID = 0;

	// If a category is deleted after others have been added, none of the
	// category numbers are changed.  So we need to find the category number
	// for this category (this category is represented by the selected
	// *text*).
	//
	//
	// The top entry in the list is "All", so if the top item is
	// selected we can indicate that we are using the "All" category.
	//
	//
	if (fCatList->currentItem()==0)
	{
		currentCatID=-1;
#ifdef DEBUG
		{
			kdDebug() << fname <<
				": Category 'All' selected.\n" ;
		}
#endif
	}
	else
	{
		QString selectedCategory=fCatList->text(fCatList->currentItem());
		DEBUGKPILOT << fname
			<< ": List item " 
			<< fCatList->currentItem()
			<< " (of "
			<< fCatList->count()
			<< ") "
			<< " selected, text="
			<< selectedCategory 
			<< endl ;

		currentCatID=0;
		while(strcmp(info->name[currentCatID], 
		       selectedCategory.local8Bit()) && 
			(currentCatID < MAX_CATEGORIES))
		{
			DEBUGKPILOT << fname
				<< ": Didn't match category "
				<< currentCatID 
				<< "="
				<< info->name[currentCatID]
				<< endl ;

			currentCatID++;
		}

		if (!(currentCatID < MAX_CATEGORIES))
		{
			currentCatID=0;
			while(strcmp(info->name[currentCatID],
				selectedCategory.latin1()) &&
				(currentCatID < MAX_CATEGORIES))
			{
				currentCatID++;
			}
		}

		if (!(currentCatID < MAX_CATEGORIES))
		{
			currentCatID=0;
			while(strcmp(info->name[currentCatID],
				selectedCategory.ascii()) &&
				(currentCatID < MAX_CATEGORIES))
			{
				currentCatID++;
			}
		}

		if (!(currentCatID < MAX_CATEGORIES))
		{
			currentCatID=0;
			while((info->name[currentCatID][0]) &&
				(currentCatID < MAX_CATEGORIES))
			{
				if (selectedCategory ==
					QString::fromLatin1(info->name[currentCatID]))
				{
					DEBUGKPILOT << fname
						<< ": Matched "
						<< currentCatID
						<< endl;

					break;
				}
				currentCatID++;
			}
		}

		if (currentCatID < MAX_CATEGORIES)
		{
			DEBUGKPILOT << fname 
				<< ": Matched category "
				<< currentCatID 
				<< "=" 
				<< info->name[currentCatID]
				<< endl ;
		}
		else
		{
#ifdef DEBUG       // necessary for Tru64 unix
			kdWarning() << __FUNCTION__ 
				<< ": Selected category didn't match "
				"any name!\n" ;
			kdWarning() << __FUNCTION__
				<< ": Number of listed categories "
				<< fCatList->count()
				<< endl;
			kdWarning() << __FUNCTION__
				<< ": Selected category ("
				<< selectedCategory
				<< ") expands to "
				<< qstringExpansion(selectedCategory)
				<< endl;
			kdWarning() << __FUNCTION__
				<< ": Categories expand to "
				<< endl;
#endif
			currentCatID=0;
			while((info->name[currentCatID][0]) &&
				(currentCatID < MAX_CATEGORIES))
			{
#ifdef DEBUG
				kdWarning() << __FUNCTION__
					<< ": Category ["
					<< currentCatID
					<< "] = "
					<< charExpansion(info->name[currentCatID])
					<< endl;
#endif
				currentCatID++;
			}

			currentCatID=-1;
		}
	}

	if ((currentCatID==-1) && AllIsUnfiled) currentCatID=0;
	return currentCatID;
}


void PilotComponent::populateCategories(QComboBox *c,
	struct CategoryAppInfo *info)
{
	FUNCTIONSETUP;
	int i;

	c->clear();

	if (!info) goto CategoryAll;

	// Fill up the categories list box with
	// the categories defined by the user. 
	// These presumably are in the language 
	// the user uses, so no translation is necessary.
	//
	//
	for(i = 0; i < 15; i++)
	{
		if(info->name[i][0])
		{
			DEBUGKPILOT << fname
				<< ": Adding category: "
				<< info->name[i]
				<< " with ID: " 
				<< (int)info->ID[i] 
				<< endl;

			c->insertItem(QString::fromLatin1(info->name[i]));
		}
	}

CategoryAll:
	c->insertItem(i18n("All"),0);
}


void PilotComponent::slotShowComponent()
{
	FUNCTIONSETUP;

	DEBUGKPILOT << fname
		<< ": Showing component @"
		<< (int) this
		<< endl;

	emit showComponent(this);
}

// $Log$
// Revision 1.18  2001/04/14 15:21:35  adridg
// XML GUI and ToolTips
//
// Revision 1.17  2001/04/11 21:39:22  adridg
// Fix for bad-categories bug
//
// Revision 1.16  2001/04/03 09:55:13  adridg
// Administrative, cleanup
//
// Revision 1.15  2001/04/01 17:32:20  adridg
// Fix infinie loop when changing categories
//
// Revision 1.14  2001/03/27 11:10:39  leitner
// ported to Tru64 unix: changed all stream.h to iostream.h, needed some
// #ifdef DEBUG because qstringExpand etc. were not defined.
//
// Revision 1.13  2001/03/24 15:59:22  adridg
// Some populateCategories changes for bug #22112
//
// Revision 1.12  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.11  2001/03/04 21:27:07  adridg
// Note to self: compile first, commit after
//
// Revision 1.10  2001/03/04 21:20:55  adridg
// Attempt to fix poor matching of category names
//
// Revision 1.9  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
// Revision 1.8  2001/02/19 04:04:52  rkrusty
// [IEM] fix for compiling with glibc 2.2.2
//
// --- pilotComponent.cc   Sun Feb 18 19:27:29 2001
// +++ pilotComponent.cc.new       Sun Feb 18 19:27:18 2001
// @@ -30,7 +30,7 @@
//
//
//  #include "options.h"
// -#include <sys/time.h>
// +#include <time.h>
//  #include <iostream.h>
//  #include <pi-appinfo.h>
//  #include <qwidget.h>
//
// Revision 1.7  2001/02/05 20:58:48  adridg
// Fixed copyright headers for source releases. No code changed
//
