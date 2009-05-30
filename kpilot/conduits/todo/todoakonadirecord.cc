/* todoakonadirecord.cc			KPilot
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

#include "todoakonadirecord.h"

#include <boost/shared_ptr.hpp>
#include <kcal/todo.h>

#include "options.h"

typedef boost::shared_ptr<KCal::Incidence> IncidencePtr;

TodoAkonadiRecord::TodoAkonadiRecord( const Akonadi::Item& i, const QDateTime& dt )
	: AkonadiRecord( i, dt )
{
	FUNCTIONSETUPL(5);
	DEBUGKPILOT << toString();
}

TodoAkonadiRecord::TodoAkonadiRecord( const QString& id ) : AkonadiRecord( id )
{
	Akonadi::Item item;
	item.setPayload<IncidencePtr>( IncidencePtr( new KCal::Todo() ) );
	item.setMimeType( "application/x-vnd.akonadi.calendar.todo" );
	setItem( item );
	// Set item changes the Id of the record to the item id which is invalid in case
	// of deleted records.
	setId(id);
}

TodoAkonadiRecord::~TodoAkonadiRecord()
{
}

void TodoAkonadiRecord::addCategory( const QString& category )
{
	KCal::Todo* todo = static_cast<KCal::Todo*>( item().payload<IncidencePtr>().get() );
	
	if( !todo->categories().contains( category ) )
	{
		QStringList categories = todo->categories();
		categories.append( category );
		todo->setCategories( categories );
	}
	
	// This isn't needed when using pointers.
	// item().setPayload<IncidencePtr>( IncidencePtr( todo ) );
}

int TodoAkonadiRecord::categoryCount() const
{
	FUNCTIONSETUP;
	
	boost::shared_ptr<KCal::Todo> todo
		 = boost::dynamic_pointer_cast<KCal::Todo, KCal::Incidence>( item().payload<IncidencePtr>() );
	
	DEBUGKPILOT << this << " TodoPointer: " <<  todo;
	
	return todo->categories().size();
}

bool TodoAkonadiRecord::containsCategory( const QString& category ) const
{
	FUNCTIONSETUP;
	
	boost::shared_ptr<KCal::Todo> todo
		 = boost::dynamic_pointer_cast<KCal::Todo, KCal::Incidence>( item().payload<IncidencePtr>() );
	
	DEBUGKPILOT << todo;
	return todo->categories().contains( category );
}

QStringList TodoAkonadiRecord::categories() const
{
	FUNCTIONSETUP;
	
	boost::shared_ptr<KCal::Todo> todo
		 = boost::dynamic_pointer_cast<KCal::Todo, KCal::Incidence>( item().payload<IncidencePtr>() );
	
	DEBUGKPILOT << todo;
	return todo->categories();
}

QString TodoAkonadiRecord::description() const
{
	boost::shared_ptr<KCal::Todo> todo
		= boost::dynamic_pointer_cast<KCal::Todo, KCal::Incidence>(
		item().payload<IncidencePtr>() );

	return todo->summary();
}

QString TodoAkonadiRecord::toString() const
{
	boost::shared_ptr<KCal::Todo> todo
		= boost::dynamic_pointer_cast<KCal::Todo, KCal::Incidence>(
		item().payload<IncidencePtr>() );

	QString desc =
		QString("TodoAkonadiRecord. Summary: [%1]")
		.arg(todo->summary());

	return desc;
}
