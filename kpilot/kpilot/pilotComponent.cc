/* pilotComponent.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/


#include "options.h"

#include <time.h>

#include <pi-appinfo.h>

#include <qwidget.h>
#include <qcombobox.h>
#include <qtextcodec.h>

#include <kdebug.h>

#include "kpilotConfig.h"
#include "pilotAppCategory.h"
#include "pilotComponent.moc"

static const char *pilotComponent_id =
	"$Id$";

// This is a pilot constant and should probably be defined
// in a more sensible place but I'm lazy right now.
//
#define MAX_CATEGORIES	(15)

PilotComponent::PilotComponent(QWidget * parent,
	const char *id,
	const QString & path) :
	QWidget(parent, id),
	fDBPath(path),
	shown(false)
{
	FUNCTIONSETUP;

	if (parent)
	{
		resize(parent->geometry().width(),
			parent->geometry().height());
	}

	(void) pilotComponent_id;
}



int PilotComponent::findSelectedCategory(QComboBox * fCatList,
	struct CategoryAppInfo *info, bool AllIsUnfiled)
{
	FUNCTIONSETUP;

	// Semantics of currentCatID are:
	//
	// >=0          is a specific category based on the text ->
	//              category number mapping defined by the Pilot,
	// ==-1         means "All" category selected when
	//              AllIsUnfiled is true.
	// == 0         == Unfiled means "All" category selected when
	//              AllIsUnfiled is false.
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
	if (fCatList->currentItem() == 0)
	{
		currentCatID = (-1);
#ifdef DEBUG
		DEBUGKPILOT << fname << ": Category 'All' selected.\n";
#endif
	}
	else
	{
		QString selectedCategory =
			fCatList->text(fCatList->currentItem());

#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": List item "
			<< fCatList->currentItem()
			<< " (of "
			<< fCatList->count()
			<< ") "
			<< " selected, text=" << selectedCategory << endl;
#endif

		currentCatID = 0;
		while (strcmp(info->name[currentCatID],
				selectedCategory.latin1()) &&
			(currentCatID < MAX_CATEGORIES))
		{
#ifdef DEBUG
			DEBUGKPILOT << fname
				<< ": Didn't match category "
				<< currentCatID
				<< "=" << info->name[currentCatID] << endl;
#endif

			currentCatID++;
		}

		if (!(currentCatID < MAX_CATEGORIES))
		{
			currentCatID = 0;
			while (strcmp(info->name[currentCatID],
					selectedCategory.latin1()) &&
				(currentCatID < MAX_CATEGORIES))
			{
				currentCatID++;
			}
		}

		if (!(currentCatID < MAX_CATEGORIES))
		{
			currentCatID = 0;
			while (strcmp(info->name[currentCatID],
					selectedCategory.ascii()) &&
				(currentCatID < MAX_CATEGORIES))
			{
				currentCatID++;
			}
		}

		if (!(currentCatID < MAX_CATEGORIES))
		{
			currentCatID = 0;
			while ((info->name[currentCatID][0]) &&
				(currentCatID < MAX_CATEGORIES))
			{
				if (selectedCategory ==
					QString::fromLatin1(info->
						name[currentCatID]))
				{
#ifdef DEBUG
					DEBUGKPILOT << fname
						<< ": Matched "
						<< currentCatID << endl;
#endif

					break;
				}
				currentCatID++;
			}
		}

		if (currentCatID < MAX_CATEGORIES)
		{
#ifdef DEBUG
			DEBUGKPILOT << fname
				<< ": Matched category "
				<< currentCatID
				<< "=" << info->name[currentCatID] << endl;
#endif
		}
		else
		{
#ifdef DEBUG			// necessary for Tru64 unix
			kdWarning() << k_funcinfo
				<< ": Selected category didn't match "
				"any name!\n";
			kdWarning() << k_funcinfo
				<< ": Number of listed categories "
				<< fCatList->count() << endl;
			kdWarning() << k_funcinfo
				<< ": Selected category ("
				<< selectedCategory
				<< ") " << endl;
			kdWarning() << k_funcinfo
				<< ": Categories expand to " << endl;
#endif
			currentCatID = 0;
			while ((info->name[currentCatID][0]) &&
				(currentCatID < MAX_CATEGORIES))
			{
#ifdef DEBUG
				kdWarning() << k_funcinfo
					<< ": Category ["
					<< currentCatID
					<< "] = "
					<< (info->name[currentCatID]) << endl;
#endif
				currentCatID++;
			}

			currentCatID = (-1);
		}
	}

	if ((currentCatID == -1) && AllIsUnfiled)
		currentCatID = 0;
	return currentCatID;
}


void PilotComponent::populateCategories(QComboBox * c,
	struct CategoryAppInfo *info)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGKPILOT << fname
		<< ": Combo box @"
		<< (long) c << " and info @" << (long) info << endl;
#endif

	c->clear();

	if (!info)
		goto CategoryAll;

	// Fill up the categories list box with
	// the categories defined by the user.
	// These presumably are in the language
	// the user uses, so no translation is necessary.
	//
	//
	for (int i = 0; i < 15; i++)
	{
		if (info->name[i][0])
		{
#ifdef DEBUG
			DEBUGKPILOT << fname
				<< ": Adding category: "
				<< info->name[i]
				<< " with ID: " << (int) info->ID[i] << endl;
#endif

			c->insertItem(PilotAppCategory::codec()->toUnicode(info->name[i]));
		}
	}

CategoryAll:
	c->insertItem(i18n("All"), 0);
}


void PilotComponent::slotShowComponent()
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGKPILOT << fname << ": Showing component @" << (long) this << endl;
#endif

	emit showComponent(this);
}

/* virtual */ bool PilotComponent::preHotSync(QString &)
{
	FUNCTIONSETUP;

	return true;
}

void PilotComponent::markDBDirty(const QString db)
{
	FUNCTIONSETUP;
	KPilotConfig::addDirtyDatabase(db);
	KPilotConfig::sync();
}

void PilotComponent::showKPilotComponent( bool toShow )
{
	if ( toShow != shown )
	{
		shown = toShow;
		if (shown) showComponent();
		else hideComponent();
	}
}
