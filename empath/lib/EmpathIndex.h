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
# pragma interface "EmpathIndex.h"
#endif

#ifndef EMPATHMESSAGEDESCRIPTIONLIST
#define EMPATHMESSAGEDESCRIPTIONLIST

// Qt includes
#include <qdict.h>

// Local includes
#include "RMM_MessageID.h"
#include "EmpathIndexRecord.h"

class EmpathFolder;
class EmpathMessageList;

typedef QDictIterator<EmpathIndexRecord> EmpathIndexIterator;

class EmpathIndex : public QDict<EmpathIndexRecord>
{
    public:
        
        EmpathIndex();
        ~EmpathIndex();

        EmpathIndexRecord * messageDescription(RMM::RMessageID & id) const;
        
        void setFolder(EmpathFolder * parent) { folder_ = parent; }

        /**
         * Count the number of messages stored.
         */

        Q_UINT32 countUnread() const;

        /**
         * Sync up the message list with the mailbox.
         */

        void sync();

        EmpathFolder * folder() { return folder_; }

        const char * className() const { return "EmpathMessageList"; }

    protected:
        
        virtual int compareItems(void *, void *);
        
        EmpathFolder * folder_;
        
        QDateTime mtime_;
};

#endif

// vim:ts=4:sw=4:tw=78
