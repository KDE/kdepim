/*
    Empath - Mailer for KDE
    
    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org
    
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
# pragma interface "EmpathMailboxList.h"
#endif

#ifndef EMPATHMAILBOXLIST_H
#define EMPATHMAILBOXLIST_H

// Qt includes
#include <qdict.h>
#include <qstring.h>

// Local includes

#include "EmpathMailbox.h"
#include "EmpathURL.h"

typedef QDictIterator<EmpathMailbox> EmpathMailboxListIterator;

/**
 * @short The internal mailbox list
 * @author Rikkus
 */
class EmpathMailboxList : public QObject, public QDict<EmpathMailbox>
{
    Q_OBJECT

    public:
    
        EmpathMailboxList();
        ~EmpathMailboxList();
        
        /**
         * @internal
         */
        void loadConfig();
        /**
         * @internal
         */
        void saveConfig() const;
        
        /**
         * Remove the given mailbox from the list.
         */
        bool remove(const EmpathURL &);

        /**
         * @return the folder with the given url, unless not found, in which
         * case 0.
         */
        EmpathFolder * folder(const EmpathURL & folderURL) const;

        void triggerMailCheck();

        EmpathMailbox * createNew(EmpathMailbox::Type);

    signals:
        
        /**
         * Emitted when the on-screen folder lists are out of sync and need to
         * be updated.
         */
        void updateFolderLists();

        /**
         * Tell each mailbox to get new mail now.
         */
        void checkMail();

    private:

        /**
         * Append the given mailbox to the list.
         */
        void _append(EmpathMailbox * mailbox);
 
        QString uniqueName();
};

#endif

// vim:ts=4:sw=4:tw=78
