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
# pragma interface "EmpathMessageListItem.h"
#endif

#ifndef EMPATHMESSAGELISTITEM_H
#define EMPATHMESSAGELISTITEM_H

// Qt includes
#include <qstring.h>
#include <qlistview.h>

// Local includes
#include "EmpathDefines.h"
#include "EmpathIndexRecord.h"
#include <RMM_MessageID.h>
#include <RMM_Mailbox.h>
#include <RMM_DateTime.h>
#include <RMM_Enum.h>

class EmpathMessageListWidget;

/**
 * @internal
 */
class EmpathMessageListItem : public QListViewItem
{
    public:
    
        EmpathMessageListItem(
            EmpathMessageListWidget * parent,
            EmpathIndexRecord rec);

        EmpathMessageListItem(
            EmpathMessageListItem * parent,
            EmpathIndexRecord rec);

        ~EmpathMessageListItem();
        
        virtual void setup();

        QString key(int, bool) const;

        QString             id()        const   { return m.id();        }
        RMM::RMessageID &   messageID()         { return m.messageID(); }
        RMM::RMessageID &   parentID()          { return m.parentID();  }
        QString             subject()   const   { return m.subject();   }
        RMM::RMailbox &     sender()            { return m.sender();    }
        RMM::RDateTime &    date()              { return m.date();      }
        RMM::MessageStatus  status()    const   { return m.status();    }
        Q_UINT32            size()      const   { return m.size();      }
        
        const char * className() const { return "EmpathMessageListItem"; }
        
        void setStatus(RMM::MessageStatus);
        
    private:

        void _init();

        EmpathIndexRecord    m;
        
        QString                niceDate_;
        QString                dateStr_;
        QString                sizeStr_;
};

typedef QList<EmpathMessageListItem> EmpathMessageListItemList;
typedef QListIterator<EmpathMessageListItem> EmpathMessageListItemIterator;

#endif

// vim:ts=4:sw=4:tw=78
