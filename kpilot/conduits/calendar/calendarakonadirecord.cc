/* calendarakonadirecord.cc			KPilot
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

#include "calendarakonadirecord.h"

#include <boost/shared_ptr.hpp>
#include <kcal/event.h>

#include "options.h"

typedef boost::shared_ptr<KCal::Incidence> IncidencePtr;

CalendarAkonadiRecord::CalendarAkonadiRecord( const Akonadi::Item& i, const QDateTime& dt )
	: AkonadiRecord( i, dt )
{
	FUNCTIONSETUPL(5);
	DEBUGKPILOT << toString();
}

CalendarAkonadiRecord::CalendarAkonadiRecord( const QString& id ) : AkonadiRecord( id )
{
	Akonadi::Item item;
	item.setPayload<IncidencePtr>( IncidencePtr( new KCal::Event() ) );
	item.setMimeType( "application/x-vnd.akonadi.calendar.event" );
	setItem( item );
	// Set item changes the Id of the record to the item id which is invalid in case
	// of deleted records.
	setId(id);
}

CalendarAkonadiRecord::~CalendarAkonadiRecord()
{
}

void CalendarAkonadiRecord::addCategory( const QString& category )
{
	boost::shared_ptr<KCal::Event> event
		= boost::dynamic_pointer_cast<KCal::Event, KCal::Incidence>
			(
				item().payload<IncidencePtr>()
			);
	
	if( !event->categories().contains( category ) )
	{
		QStringList categories = event->categories();
		categories.append( category );
		event->setCategories( categories );
	}
	
	// This isn't needed when using pointers. And it is really a bad idea to have
	// another IncidencePtr handling the same raw pointer.
	// item().setPayload<IncidencePtr>( IncidencePtr( event ) );
}

int CalendarAkonadiRecord::categoryCount() const
{
	FUNCTIONSETUP;
	
	boost::shared_ptr<KCal::Event> event
		= boost::dynamic_pointer_cast<KCal::Event, KCal::Incidence>
			(
				item().payload<IncidencePtr>() 
			);
	
	return event->categories().size();
}

bool CalendarAkonadiRecord::containsCategory( const QString& category ) const
{
	FUNCTIONSETUP;
	
	boost::shared_ptr<KCal::Event> event
		= boost::dynamic_pointer_cast<KCal::Event, KCal::Incidence>
			(
				item().payload<IncidencePtr>()
			);
	
	return event->categories().contains( category );
}

QStringList CalendarAkonadiRecord::categories() const
{
	FUNCTIONSETUP;
	
	boost::shared_ptr<KCal::Event> event
		= boost::dynamic_pointer_cast<KCal::Event, KCal::Incidence>
			(
				item().payload<IncidencePtr>()
			);
	
	return event->categories();
}

QString CalendarAkonadiRecord::description() const
{
	boost::shared_ptr<KCal::Event> event
		= boost::dynamic_pointer_cast<KCal::Event, KCal::Incidence>
			(
				item().payload<IncidencePtr>()
			);

	return event->summary();
}

QString CalendarAkonadiRecord::toString() const
{
	boost::shared_ptr<KCal::Event> event
		= boost::dynamic_pointer_cast<KCal::Event, KCal::Incidence>
			(
				item().payload<IncidencePtr>()
			);
	QString desc = 
		QString("CalendarAkonadiRecord. Summary: [%1], "
			"Start: [%2] (timespec: %3), End: [%4] (timespec: %5)")
		.arg(event->summary())
		.arg(event->dtStart().dateTime().toLocalTime().toString())
		.arg(event->dtStart().dateTime().timeSpec())
		.arg(event->dtEnd().dateTime().toLocalTime().toString())
		.arg(event->dtEnd().dateTime().timeSpec());

	return desc;
}

bool CalendarAkonadiRecord::isValid() const
{
	FUNCTIONSETUPL(5);
	boost::shared_ptr<KCal::Event> event
		= boost::dynamic_pointer_cast<KCal::Event, KCal::Incidence>
			(
				item().payload<IncidencePtr>()
			);
	bool myCheck = (!event->summary().isEmpty() &&
			event->dtStart().dateTime().isValid() &&
			event->dtEnd().dateTime().isValid());
	bool parentCheck = AkonadiRecord::isValid();
	return myCheck && parentCheck;
}
