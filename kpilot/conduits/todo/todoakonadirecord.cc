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

#include <kcal/todo.h>

TodoAkonadiRecord::TodoAkonadiRecord( const Akonadi::Item& i, const QDateTime& dt )
	: AkonadiRecord( i, dt )
{
}

TodoAkonadiRecord::TodoAkonadiRecord( const QString& id ) : AkonadiRecord( id )
{
}

TodoAkonadiRecord::~TodoAkonadiRecord()
{
}

void TodoAkonadiRecord::addCategory( const QString& category )
{
	KCal::Todo a = item().payload<KCal::Todo>();
	if( !a.categories().contains( category ) )
	{
		QStringList categories = a.categories();
		categories.append( category );
		a.setCategories( categories );
	}
	
	item().setPayload<KCal::Todo>( a );
}

int TodoAkonadiRecord::categoryCount() const
{
	KCal::Todo a = item().payload<KCal::Todo>();
	return a.categories().size();
}

bool TodoAkonadiRecord::containsCategory( const QString& category ) const
{
	KCal::Todo a = item().payload<KCal::Todo>();
	return a.categories().contains( category );
}

QStringList TodoAkonadiRecord::categories() const
{
	KCal::Todo a = item().payload<KCal::Todo>();
	return a.categories();
}

QString TodoAkonadiRecord::toString() const
{
	return QString();
}
