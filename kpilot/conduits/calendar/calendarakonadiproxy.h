#ifndef CALENDARAKONADIPROXY_H
#define CALENDARAKONADIPROXY_H
/* CalendarAkonadiProxy.h			KPilot
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

#include "akonadidataproxy.h"

#include <kcal/event.h>

#include <boost/shared_ptr.hpp>

typedef boost::shared_ptr<KCal::Incidence> IncidencePtr;

class CalendarAkonadiProxy : public AkonadiDataProxy
{
public:
	CalendarAkonadiProxy( const IDMapping& mapping );

	/**
	 * Adds the given category to the record and might do some internal things
	 * needed for category handling in the datastore.
	 * 
	 * All other categories that might have been set to this record should be
	 * unchanged.
	 */
	/* virtual */ void addCategory( Record* rec, const QString& category );
	
	/**
	 * Sets the given category as the only category to the record and might do
	 * some internal things needed for category handling in the datastore.
	 * 
	 * All other categories that might have been set to this record should be
	 * removed.
	 */
	/* virtual */ void setCategory( Record* rec, const QString& category );

protected:
	/**
	 * Creates a new akonadi record for @param i and sets the last sync time
	 * @param dt to the record.
	 */
  /* virtual */ AkonadiRecord* createAkonadiRecord( const Akonadi::Item& i
                                            , const QDateTime& dt ) const;
	
	/**
	 * Creates a dummy record with given id. These are for the records that where
	 * deleted from the collection after the last sync. The returned record
	 * <em>must</em> must return true on both modified() and isDeleted().
	 */
	/* virtual */ AkonadiRecord* createDeletedAkonadiRecord( const QString& id ) const;
	
	/**
	 * Checks if the Item has payload that is supported by this proxy.
	 */
	/* virtual */ bool hasValidPayload( const Akonadi::Item& i ) const;
};

#endif
