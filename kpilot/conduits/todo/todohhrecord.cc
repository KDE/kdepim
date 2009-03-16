/* todohhrecord.cc			KPilot
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

#include "todohhrecord.h"

#include <QtCore/QDateTime>

#include "options.h"

TodoHHRecord::TodoHHRecord( PilotRecord *record, const QString &category )
	: HHRecord( record, category )
{
	FUNCTIONSETUPL(5);
	DEBUGKPILOT << "id: [" << id() << "], description: [" << toString() << "]";

}

bool TodoHHRecord::equal( const HHRecord* other ) const
{
	FUNCTIONSETUP;
	
	const TodoHHRecord* hhOther = static_cast<const TodoHHRecord*>( other );
	
	PilotTodoEntry entryOther = hhOther->todoEntry();
	PilotTodoEntry entryThis = todoEntry();
	
	bool descriptionEqual = entryThis.getDescription() == entryOther.getDescription();
	bool noteEqual = entryThis.getNote() == entryOther.getNote();
	bool categoriesEqual = category() == other->category();
	bool dueDateEqual = readTm( entryThis.getDueDate() ) == readTm( entryOther.getDueDate() );
	bool completeEqual = entryThis.getComplete() == entryOther.getComplete();
	bool priorityEqual = entryThis.getPriority() == entryOther.getPriority();
	
	return descriptionEqual
		&& noteEqual
		&& categoriesEqual
		&& dueDateEqual
		&& completeEqual
		&& priorityEqual;
}

void TodoHHRecord::setTodoEntry( const PilotTodoEntry& entry, bool keepPrevCategory )
{
	FUNCTIONSETUP;
	
	PilotRecord* record = entry.pack();
	
	if( keepPrevCategory )
	{
		record->setCategory( fRecord->category() );
	}
	
	KPILOT_DELETE( fRecord );
	fRecord = record;
}

PilotTodoEntry TodoHHRecord::todoEntry() const
{
	FUNCTIONSETUP;
	
	return PilotTodoEntry( fRecord );
}

QString TodoHHRecord::toString() const
{
	PilotTodoEntry tde = todoEntry();
	return tde.getDescription();
}
