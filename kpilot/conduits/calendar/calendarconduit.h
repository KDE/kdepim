#ifndef CALENDARCONDUIT_H
#define CALENDARCONDUIT_H
/* calendarconduit.h			KPilot
**
** Copyright (C) 2008 by Bertjan Broeksema <b.broeksema@kdemail.net>
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

#include "recordconduit.h"

namespace boost
{
	template<class T> class shared_ptr;
}

namespace KCal
{
	class Event;
}

class PilotDateEntry;

typedef boost::shared_ptr<KCal::Event> EventPtr;

class CalendarConduit : public RecordConduit
{
public:
	explicit CalendarConduit( KPilotLink *o, const QVariantList &a = QVariantList() );

	~CalendarConduit();

	/* virtual */ void loadSettings();

	/* virtual */ bool initDataProxies();
	
	/**
	 * Compares @p pcRecord with @p hhRec and returns true if they are considered
	 * equal.
	 */
	/* virtual */ bool equal( const Record *pcRec, const HHRecord *hhRec ) const;
	
	/**
	 * Creates a new Record object with the same data as @p hhRec.
	 */
	/* virtual */ Record* createPCRecord( const HHRecord *hhRec );
	
	/**
	 * Creates a new HHRecord object with the same data as @p pcRec.
	 */
	/* virtual */ HHRecord* createHHRecord( const Record *pcRec );
	
	/**
	 * Copies the field values of @p from to @p to. The method should only touch
	 * data that can be synced between the two records and leave the rest of the
	 * records data unchanged. After calling this method
	 *
	 * RecordConduit::equal( pcRecord, hhRecord ) must return true.
	 */
	/* virtual */ void _copy( const Record *from, HHRecord *to );
	
	/**
	 * Copies the field values of @p from to @p to. The method should only touch
	 * data that can be synced between the two records and leave the rest of the
	 * records data unchanged. After calling this method
	 *
	 * RecordConduit::equal( pcRecord, hhRecord ) must return true.
	 */
	/* virtual */ void _copy( const HHRecord *from, Record *to  );
	
	/**
	 * Give implementing conduits the change to clean up after a successful sync.
	 */
	/* virtual */ void syncFinished();
	
	/**
	 * This method is called when the conduit is run in Test Mode. The 
	 * implementing class can do whatever it wants to do for test purposes.
	 */
	/* virtual */ void test() {}

private:
	void setAlarms( PilotDateEntry* de, const EventPtr& e ) const;
	
	void setAlarms( EventPtr e, const PilotDateEntry& de ) const;

	void setExceptions( PilotDateEntry* de, const EventPtr& e ) const;
	
	void setExceptions( EventPtr e, const PilotDateEntry& de ) const;

	void setRecurrence( PilotDateEntry* de, const EventPtr& e ) const;
	
	void setRecurrence( EventPtr e, const PilotDateEntry& de ) const;
	
	void setStartEndTimes( PilotDateEntry* de, const EventPtr& e ) const;
	
	void setStartEndTimes( EventPtr e, const PilotDateEntry& de ) const;

private:
	class Private;
	Private* d;
};

#endif
