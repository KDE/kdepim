/*
    Empath - Mailer for KDE
    
    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>
    
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
#include "EmpathFilter.h"
#include "EmpathFilterList.h"

EmpathFilterList::EmpathFilterList()
{ 
    // Empty.
}

EmpathFilterList::~EmpathFilterList()
{ 
    // Empty.
}

    void
EmpathFilterList::saveConfig()
{
    EmpathFilterListIterator it(*this);
    
    QStringList list;

    for (; it.current() ; ++it) {
        it.current()->save();
        list << it.current()->name();
    }
    
    KConfig * c = KGlobal::config();

    c->setGroup("General");
    
    c->writeEntry("FilterList", list, ',');
}

    void
EmpathFilterList::loadConfig()
{
    KConfig * c = KGlobal::config();
    
    c->setGroup("General");
    
    QStrList list;
    c->readListEntry("FilterList", list, ',');
    
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
    empathDebug(id.asString());
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
        
    QList<EmpathFilter>::remove(f);
}

    void
EmpathFilterList::append(EmpathFilter * f)
{
    f->setPriority(count());
    QList<EmpathFilter>::append(f);
}

// vim:ts=4:sw=4:tw=78
