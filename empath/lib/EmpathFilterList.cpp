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

// KDE includes
#include <kconfig.h>
#include <kapp.h>

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
	
	int c = 0;

	for (; it.current() ; ++it) {
		it.current()->setId(c++);
		it.current()->save();
	}
	
	empathDebug("Saving number of filters");
	KConfig * config = kapp->getConfig();
	config->setGroup(EmpathConfig::EmpathConfig::GROUP_GENERAL);
	config->writeEntry(EmpathConfig::EmpathConfig::KEY_NUMBER_OF_FILTERS, count());
}

	void
EmpathFilterList::load()
{
	empathDebug("load() called");
	
	KConfig * c = kapp->getConfig();
	c->setGroup(EmpathConfig::EmpathConfig::GROUP_GENERAL);
	
	Q_UINT32 numberOfFilters =
		c->readUnsignedNumEntry(EmpathConfig::EmpathConfig::KEY_NUMBER_OF_FILTERS);
	
	empathDebug("Number of filters == " + QString().setNum(numberOfFilters));

	EmpathFilter * filter;
	
	for (Q_UINT32 i = 0 ; i < numberOfFilters ; ++i) {
		filter = new EmpathFilter;
		filter->load(i);
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

