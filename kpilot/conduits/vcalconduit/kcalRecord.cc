/* kcalRecord.cc                       KPilot
**
** Copyright (C) 2006 by Adriaan de Groot <groot@kde.org>
** Copyright (C) 2002-2003 by Reinhold Kainhofer
** Copyright (C) 2001 by Dan Pilone
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

#include "options.h"

#include <libkcal/calendar.h>
#include <libkcal/calendarlocal.h>
#include <libkcal/recurrence.h>
#include <libkcal/vcalformat.h>

#include "pilot.h"
#include "pilotRecord.h"
#include "kcalRecord.h"

void KCalSync::setCategory(PilotRecordBase *de,
	const KCal::Incidence *e,
	const CategoryAppInfo &info)
{
	FUNCTIONSETUP;

	if (!de || !e)
	{
		return;
	}

	QString deCategory;
	QStringList eventCategories = e->categories();
	if (eventCategories.size() < 1)
	{
		// This event has no categories.
		de->setCategory(Pilot::Unfiled);
		return;
	}

	// Quick check: does the record (not unfiled) have an entry
	// in the categories list? If so, use that.
	if (de->category() != Pilot::Unfiled)
	{
		deCategory = Pilot::categoryName(&info,de->category());
		if (eventCategories.contains(deCategory))
		{
			// Found, so leave the category unchanged.
			return;
		}
	}

	QStringList availableHandheldCategories = Pilot::categoryNames(&info);

	// Either the record is unfiled, and should be filed, or
	// it has a category set which is not in the list of
	// categories that the event has. So go looking for
	// a category that is available both for the event
	// and on the handheld.
	for ( QStringList::ConstIterator it = eventCategories.begin();
		it != eventCategories.end(); ++it )
	{
		// Odd, an empty category string.
		if ( (*it).isEmpty() )
		{
			continue;
		}

		if (availableHandheldCategories.contains(*it))
		{
			// Since the string is in the list of available categories,
			// this *can't* fail.
			int c = Pilot::findCategory(&info,*it,false);
			Q_ASSERT( Pilot::validCategory(c) );
			de->setCategory(c);
			return;
		}
	}

	de->setCategory(Pilot::Unfiled);
}

void KCalSync::setCategory(KCal::Incidence *e,
	const PilotRecordBase *de,
	const CategoryAppInfo &info)
{
	FUNCTIONSETUP;

	if (!e || !de)
	{
		DEBUGKPILOT << fname << ": error.  unable to set kcal category. e: ["
			<< (void *)e << "], de: [" << (void *)de << "]" << endl;
		return;
	}

	QStringList cats=e->categories();
	int cat = de->category();
	QString newcat = Pilot::categoryName(&info,cat);

	DEBUGKPILOT << fname << ": palm category id: [" << cat <<
		"], label: [" << newcat << "]" << endl;

	if ( Pilot::validCategory(cat) && (cat != Pilot::Unfiled))
	{
		if (!cats.contains(newcat))
		{
			// if this event only has one category associated with it, then we can
			// safely assume that what we should be doing here is changing it to match
			// the palm.  if there's already more than one category in the event, however, we
			// won't cause data loss--we'll just append what the palm has to the
			// event's categories
			if (cats.count() <=1)
			{
				cats.clear();
			}

			cats.append( newcat );
			e->setCategories(cats);
		}
	}

	DEBUGKPILOT << fname << ": kcal categories now: [" << cats.join(",") << "]" << endl;
}
