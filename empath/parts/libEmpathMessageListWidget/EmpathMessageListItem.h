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
#include <qlist.h>

// Local includes
#include "EmpathIndexRecord.h"

class EmpathMessageListWidget;

/**
 * Encapsulation of an EmpathIndexRecord in a QListViewItem, which is can be
 * shown in EmpathMessageListWidget.
 */
class EmpathMessageListItem : public QObject, public QListViewItem
{
    Q_OBJECT

    public:
    
        EmpathMessageListItem(
            EmpathMessageListWidget * parent,
            const EmpathIndexRecord & rec);

        EmpathMessageListItem(
            EmpathMessageListItem * parent,
            const EmpathIndexRecord & rec);

        ~EmpathMessageListItem();

        void setRecord(const EmpathIndexRecord &);
        void setStatus(EmpathIndexRecord::Status);

        QString key(int, bool) const;

        QString         id()            const { return rec_.id();             }
        QString         messageID()     const { return rec_.messageID();      }
        QString         parentID()      const { return rec_.parentID();       }
        QString         subject()       const { return rec_.subject();        }
        QString         senderName()    const { return rec_.senderName();     }
        QString         senderAddress() const { return rec_.senderAddress();  }
        QDateTime       date()          const { return rec_.date();           }
        int             timeZone()      const { return rec_.timeZone();       }
        unsigned int    size()          const { return rec_.size();           }
        EmpathIndexRecord::Status status() const
        { return rec_.status(); }
    
        void startAutoMarkTimer();
        void cancelAutoMarkTimer();
        
        static void initStatic();

        virtual void setup();
        
    protected:

        virtual void paintCell(QPainter *, const QColorGroup &, int, int, int);
        
        void timerEvent(QTimerEvent *);

    signals:

        void mark(
            const QString &,
            EmpathIndexRecord::Status,
            EmpathMessageListItem *);

    private:

        void _init();
        void _setStatusIcons();
        
        static QPixmap * px_unread_;
        static QPixmap * px_read_;
        static QPixmap * px_marked_;
        static QPixmap * px_replied_;
        static QPixmap * px_attachments_;

        static QColor * unreadColour_;
        
        QString dateStr_;
        QString sizeStr_;

        EmpathIndexRecord rec_;
};

typedef QList<EmpathMessageListItem> EmpathMessageListItemList;
typedef QListIterator<EmpathMessageListItem> EmpathMessageListItemIterator;

#endif
// vim:ts=4:sw=4:tw=78
