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


#ifndef EMPATHFILTERLIST_H
#define EMPATHFILTERLIST_H

// Qt includes
#include <qlist.h>

// Local includes
#include "EmpathFilter.h"
#include "EmpathURL.h"

typedef QListIterator<EmpathFilter> EmpathFilterListIterator;

/**
 * @short The internal filter list
 * 
 * Empath always has exactly one of these. Don't construct it.
 * @author Rikkus
 */
class EmpathFilterList : public QList<EmpathFilter>
{
    public:

        /**
         * @internal
         */
        EmpathFilterList();
        /**
         * @internal
         */
        virtual ~EmpathFilterList();

        /**
         * Load settings.
         * Called by Empath on startup.
         */
        void loadConfig();
        /**
         * Save settings.
         * Called by Empath on startup.
         */
        void saveConfig();

        /**
         * Filter the given message.
         * Goes through the list and asks each filter to look at the given
         * URL.
         */
        void filter(const EmpathURL &);

        /**
         * Raise the priority of a filter by one.
         */
        void raisePriority(EmpathFilter *);
        /**
         * Lower the priority of a filter by one.
         */
        void lowerPriority(EmpathFilter *);

        /**
         * Remove the given filter, if it exists.
         */
        void remove(EmpathFilter *);
        /**
         * Append the given filter.
         */
        void append(EmpathFilter *);

        const char * className() const { return "EmpathFilterList"; }
};

#endif
// vim:ts=4:sw=4:tw=78
