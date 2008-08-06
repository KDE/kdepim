/* CalendarAkonadiProxy.cc			KPilot
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

#include "calendarakonadiproxy.h"

#include <kcal/event.h>

#include "calendarakonadirecord.h"

CalendarAkonadiProxy::CalendarAkonadiProxy( const IDMapping& mapping )
	: AkonadiDataProxy( mapping )
{
}

void CalendarAkonadiProxy::addCategory( Record* rec, const QString& category )
{
	CalendarAkonadiRecord* tar = static_cast<CalendarAkonadiRecord*>( rec );
	tar->addCategory( category );
}

void CalendarAkonadiProxy::setCategory( Record* rec, const QString& category )
{
	CalendarAkonadiRecord* tar = static_cast<CalendarAkonadiRecord*>( rec );
	tar->addCategory( category );
}

/* ***** Protected methods ***** */

AkonadiRecord* CalendarAkonadiProxy::createAkonadiRecord( const Akonadi::Item& i
	, const QDateTime& dt ) const
{
	return new CalendarAkonadiRecord( i, dt );
}

AkonadiRecord* CalendarAkonadiProxy::createDeletedAkonadiRecord( const QString& id ) const
{
	return new CalendarAkonadiRecord( id );
}

bool CalendarAkonadiProxy::hasValidPayload( const Akonadi::Item& i ) const
{
	if( i.hasPayload<IncidencePtr>() )
	{
		boost::shared_ptr<KCal::Event> event
			= boost::dynamic_pointer_cast<KCal::Event, KCal::Incidence>
			(
				i.payload<IncidencePtr>()
			);
			
		if( event )
		{
			return true;
		}
	}
	
	return false;
}
