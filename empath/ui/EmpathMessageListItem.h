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
#include <RMM_Address.h>
#include <RMM_DateTime.h>
#include <RMM_Enum.h>

class EmpathMessageListWidget;

/**
 * Encapsulation of an EmpathIndexRecord in a QListViewItem, which is can be
 * shown in EmpathMessageListWidget.
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

        QString             id()        const   { return m_.id();        }
        RMM::RMessageID &   messageID()         { return m_.messageID(); }
        RMM::RMessageID &   parentID()          { return m_.parentID();  }
        QString             subject()   const   { return m_.subject();   }
        RMM::RAddress &     sender()            { return m_.sender();    }
        RMM::RDateTime &    date()              { return m_.date();      }
        RMM::MessageStatus  status()    const   { return m_.status();    }
        Q_UINT32            size()      const   { return m_.size();      }
        
        void setStatus(RMM::MessageStatus);
        
        static void initStatic();
        
        const char * className() const { return "EmpathMessageListItem"; }
        
    protected:

        virtual void paintCell(QPainter *, const QColorGroup &, int, int, int);

    private:

        void _init();

        QPixmap & _statusIcon(RMM::MessageStatus);

        static QPixmap * px_;
        static QPixmap * px_read_;
        static QPixmap * px_marked_;
        static QPixmap * px_replied_;

        static QPixmap * px_read_marked_;
        static QPixmap * px_read_replied_;
        static QPixmap * px_marked_replied_;
        static QPixmap * px_read_marked_replied_;

        static QColor * unreadColour_;

        EmpathIndexRecord m_;
        
        QString dateStr_;
        QString sizeStr_;
};

typedef QList<EmpathMessageListItem> EmpathMessageListItemList;
typedef QListIterator<EmpathMessageListItem> EmpathMessageListItemIterator;

#endif

// vim:ts=4:sw=4:tw=78
