/*
	Empath - Mailer for KDE
	
	Copyright (C) 1998 Rik Hemsley rik@kde.org
	
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifdef __GNUG__
# pragma implementation "EmpathFilterList.h"
#endif

// KDE includes
#include <kglobal.h>
#include <kconfig.h>

// Local includes
#include "Empath.h"
#include "EmpathConfig.h"
#include "EmpathFilter.h"
#include "EmpathFilterList.h"

EmpathFilterList::EmpathFilterList()
{ 
	empathDebug("ctor");
}

EmpathFilterList::~EmpathFilterList()
{ 
	empathDebug("dtor");
}

	void
EmpathFilterList::save()
{
	empathDebug("save() called");
	
	empathDebug("There are " + QString().setNum(count()) + " filters to save");
	EmpathFilterListIterator it(*this);
	
	QStrList list;

	for (; it.current() ; ++it) {
		it.current()->save();
		list.append(it.current()->name().ascii());
	}
	
	empathDebug("Saving number of filters");
	KConfig * config = KGlobal::config();
	config->setGroup(EmpathConfig::GROUP_GENERAL);
	
	config->writeEntry(EmpathConfig::KEY_FILTER_LIST, list);
}

	void
EmpathFilterList::load()
{
	empathDebug("load() called");
	
	KConfig * c = KGlobal::config();
	c->setGroup(EmpathConfig::GROUP_GENERAL);
	
	QStrList list;
	c->readListEntry(EmpathConfig::KEY_FILTER_LIST, list);
	
	EmpathFilter * filter;
	
	QStrListIterator it(list);
	
	for (; it.current() ; ++it) {
		filter = new EmpathFilter(it.current());
		filter->load();
		append(filter);
	}
}

	void
EmpathFilterList::filter(const EmpathURL & id)
{
	empathDebug("filterMessage(" + QString(id.asString()) + ") called");

	empathDebug("There are " + QString().setNum(count()) + " filters to try");
	
	EmpathFilterListIterator it(*this);
	
	for (; it.current(); ++it)
		if (it.current()->url() == id.withoutMessageID())
			it.current()->filter(id);
}

	void
EmpathFilterList::raisePriority(EmpathFilter * f)
{
	// Remember highest priority is 0, and higher numbers make lower priority.
	EmpathFilterListIterator it(*this);
	
	// If the priority is the highest possible, ignore.
	if (f->priority() == 0)
		return;
	
	// Swap this item's priority with the one next to it, that currently has
	// a higher priority.
	for (; it.current(); ++it)
		if (it.current()->priority() == f->priority() - 1)
			it.current()->setPriority(it.current()->priority() + 1);
	
	f->setPriority(f->priority() - 1);
}

	void
EmpathFilterList::lowerPriority(EmpathFilter * f)
{
	// Remember highest priority is 0, and higher numbers make lower priority.
	EmpathFilterListIterator it(*this);
	
	// If the priority is the lowest possible, ignore.
	if (f->priority() == count() - 1)
		return;
	
	// Swap this item's priority with the one next to it, that currently has
	// a lower priority.
	for (; it.current(); ++it)
		if (it.current()->priority() == f->priority() + 1)
			it.current()->setPriority(it.current()->priority() - 1);
	
	f->setPriority(f->priority() + 1);
}

	void
EmpathFilterList::remove(EmpathFilter * f)
{
	EmpathFilterListIterator it(*this);
	
	// For each item that has a lower priority than this one, shift it up.
	for (; it.current(); ++it)
		if (it.current()->priority() > f->priority())
			it.current()->setPriority(it.current()->priority() - 1);
		
	QList::remove(f);
}

	void
EmpathFilterList::append(EmpathFilter * f)
{
	f->setPriority(count());
	QList::append(f);
}

