#ifndef CUDCOUNTER_H
#define CUDCOUNTER_H
/* cudcounter.h			KPilot
**
** Copyright (C) 2007 by Bertjan Broeksema <b.broeksema@kdemail.net>
** Copyright (C) 2007 by Jason "vanRijn" Kasper <vr@movingparts.net>
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <QString>

#include "kpilot_export.h"

/**
* Create-Update-Delete tracking of the plugin,
* used for reporting purposes (in a consistent manner).  The intent
* is that this class is used by the conduit as it is syncing data.
* For this to be useful (and be used properly), the conduit needs
* to tell us how many creates, updates, and deletes it has made to
* a data store (PC or HH).  It also needs to tell us how many
* records it started with and how many records it has at the
* conclusion of its processing.  Using this information, we can
* report on it consistently as well as analyze the activity taken
* by the conduit and offer rollback functionality if we think the
* conduit has behaved improperly.
*/
class KPILOT_EXPORT CUDCounter {
public:
	CUDCounter();

	/** How many @p t items did we start with? */
	void setStartCount( unsigned int t );

	/** How many @p t items did we end with? */
	void setEndCount( unsigned int t );

	/** Track the creation of @p c items */
	void created( unsigned int c=1 );

	/** Track updates to @p u items */
	void updated( unsigned int u=1 );

	/** Track the destruction of @p d items */
	void deleted( unsigned int d=1 );

	unsigned int countCreated() const { return fC; }
	unsigned int countUpdated() const { return fU; }
	unsigned int countDeleted() const { return fD; }
	unsigned int countStart() const { return fStart; }
	unsigned int countEnd() const { return fEnd; }

	/**
	 * Returns the sum of created records, updated records and deleted records.
	 */
	int volatilityCount() const;

	/**
	 * Returns 100 if startcount is 0 otherwise volatilityCount() / startCount.
	 */
	int volatilityPercent() const;

	/**
	 * percentage of changes.  unfortunately, we have to rely on our
	 * developers (hi, self!) to correctly set total number of records
	 * conduits start with, so add a little protection...
	 */
	unsigned int percentCreated() const { return (fEnd   > 0 ? fC/fEnd   : 0); }
	unsigned int percentUpdated() const { return (fEnd   > 0 ? fU/fEnd   : 0); }
	unsigned int percentDeleted() const { return (fStart > 0 ? fD/fStart : 0); }
	
	/** 
	 * Measurement Of Objects -- report numbers of
	 * objects created, updated, deleted. This
	 * string is already i18n()ed.
	 */
	QString moo() const;

private:
	unsigned int fC;
	unsigned int fU;
	unsigned int fD;
	unsigned int fStart;
	unsigned int fEnd;
};
#endif
