
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
#include <stream.h>
#include <pi-appinfo.h>
#include <qwidget.h>
#include <qcombobox.h>
#include <kdebug.h>
#include "pilotComponent.moc"
#include "kpilot.h"

inline
PilotComponent::PilotComponent(QWidget* parent)
  : QWidget(parent)
    {
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
		if (debug_level & UI_MINOR)
		{
			kdDebug() << fname <<
				": Category 'All' selected.\n" ;
		}
#endif
	}
	else
	{
		QString selectedCategory=fCatList->text(fCatList->currentItem());
#ifdef DEBUG
		if (debug_level & UI_MINOR)
		{
			kdDebug() << fname << 
				": List item " << fCatList->currentItem() <<
				" selected, text=" <<
				selectedCategory << endl ;
		}
#endif

		currentCatID=0;
		while(strcmp(info->name[currentCatID], 
		       selectedCategory.local8Bit()) && 
			(currentCatID < fCatList->count()))
		{
#ifdef DEBUG
			if (debug_level & UI_TEDIOUS)
			{
				kdDebug() << fname <<
					": Didn't match category " <<
					currentCatID << '=' <<
					info->name[currentCatID]
					<< endl ;
			}
#endif

			currentCatID++;
		}

		if (currentCatID < fCatList->count())
		{
#ifdef DEBUG
			if (debug_level&UI_MINOR)
			{
				kdDebug() << fname << 
					": Matched category " <<
					currentCatID << '=' <<
					info->name[currentCatID]
					<< endl ;
			}
#endif
		}
		else
		{
			kdWarning() << __FUNCTION__ 
				<< ": Selected category didn't match "
				"any name!\n" ;
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
		if(strlen(info->name[i]))
		{
#ifdef DEBUG
			if (debug_level & UI_MINOR)
			{
				kdDebug() << fname
				<< ": Adding category: "
				<< info->name[i]
				<< " with ID: " 
				<< (int)info->ID[i] 
				<< endl;
			}
#endif
			c->insertItem(info->name[i]);
		}
	}

CategoryAll:
	c->insertItem(i18n("All"),0);
}


// $Log$
// Revision 1.7  2001/02/05 20:58:48  adridg
// Fixed copyright headers for source releases. No code changed
//
