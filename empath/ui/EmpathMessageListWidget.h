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
# pragma interface "EmpathMessageListWidget.h"
#endif

#ifndef EMPATHMESSAGELISTWIDGET_H
#define EMPATHMESSAGELISTWIDGET_H

// Qt includes
#include <qpixmap.h>
#include <qlist.h>
#include <qstring.h>
#include <qpopupmenu.h>
#include <qtimer.h>
#include <qobject.h>
#include <qdragobject.h>
#include <qpoint.h>

// Local includes
#include "EmpathListView.h"
#include "EmpathDefines.h"
#include "EmpathIndexRecord.h"
#include "EmpathMessageListItem.h"
#include "EmpathURL.h"

class EmpathFolder;
class EmpathMessageListWidget;
class EmpathMainWindow;

class EmpathMarkAsReadTimer : public QObject
{
    Q_OBJECT
    
    public:
        
        EmpathMarkAsReadTimer(EmpathMessageListWidget * parent);
        ~EmpathMarkAsReadTimer();
        
        void go(EmpathMessageListItem *);
        void cancel();
        
    protected slots:

        void s_timeout();
        
    private:
        
        QTimer timer_;
        EmpathMessageListItem * item_;
        
        EmpathMessageListWidget * parent_;
};

/**
 * This is the widget that shows the threaded mail list.
 */
class EmpathMessageListWidget : public EmpathListView
{
    Q_OBJECT

    public:
    
        EmpathMessageListWidget(QWidget * parent = 0);
        
        virtual ~EmpathMessageListWidget();
        
        EmpathMessageListItem * find(RMM::RMessageID & msgId);
        EmpathMessageListItem * findRecursive(
                EmpathMessageListItem * initialItem, RMM::RMessageID & msgId);
        
        EmpathURL firstSelectedMessage();
        
        const EmpathURL & currentFolder() { return url_; }
        
        void selectTagged();
        void selectRead();
        void selectAll();
        void selectInvert();
        
        virtual void setSelected(QListViewItem *, bool);
        virtual void clearSelection();

        void listenTo(unsigned int);

    public slots:

        void s_messageDelete();

    protected slots:
    
        void s_messageMark();
        void s_messageMarkRead();
        void s_messageMarkReplied();
        void s_messageMarkMany();
        void s_messageView();
        void s_messageReply();
        void s_messageReplyAll();
        void s_messageForward();
        void s_messageBounce();
        void s_messageSaveAs();
        void s_messageCopyTo();
        void s_messagePrint();
        void s_messageFilter();

        void s_expandThread();
        void s_collapseThread();
        
        void s_rightButtonPressed   (QListViewItem *, const QPoint &, int, Area);
        void s_doubleClicked        (QListViewItem *);
        void s_linkChanged          (QListViewItem *);
        void s_startDrag            (const QList<QListViewItem> &);
        
        void s_showFolder           (const EmpathURL &, unsigned int);
        void s_headerClicked        (int);
        void s_itemGone             (const QString &);
        void s_itemCome             (const QString &);
    
    signals:
        
        void changeView(const EmpathURL &);
//        void showing();
        
    private:
        
        void _fillDisplay       (EmpathFolder *);
        
        void _setupMessageMenu();
        
        void _setSelected(EmpathMessageListItem *, bool);
        Q_UINT32 _nSelected();
        
        EmpathMessageListItem *
            _threadItem(EmpathIndexRecord & item);
        
        EmpathMessageListItem *
            _addItem(EmpathMessageListItem *, EmpathIndexRecord &);
        
        EmpathMessageListItem *
            _addItem(EmpathMessageListWidget *, EmpathIndexRecord &);
        
        void _removeItem(EmpathMessageListItem *);
        
        void getDescendants(
            EmpathMessageListItem * initialItem,
            QList<EmpathMessageListItem> * itemList);

        void append(EmpathMessageListItem * item);

        QPopupMenu  messageMenu_;
        QPopupMenu  multipleMessageMenu_;
        QPopupMenu  messageMarkMenu_;
        QPopupMenu  threadMenu_;
        
        QPixmap px_xxx_, px_Sxx_, px_xMx_, px_xxR_,
                px_SMx_, px_SxR_, px_xMR_, px_SMR_;
        
        void setStatus(EmpathMessageListItem *, RMM::MessageStatus);
        void setStatusPixmap(EmpathMessageListItem *, RMM::MessageStatus);

        EmpathURL url_;

        int lastHeaderClicked_;
        bool sortType_; // Ascending, Descending
        
        int messageMenuItemMark;
        int messageMenuItemMarkRead;
        int messageMenuItemMarkReplied;
        int messageMenuItemView;
        int messageMenuItemReply;
        int messageMenuItemReplyAll;
        int messageMenuItemForward;
        int messageMenuItemDelete;
        int messageMenuItemSaveAs;

        int sortColumn_;
        bool sortAscending_;
        
        friend class EmpathMarkAsReadTimer;
        
        EmpathMarkAsReadTimer * markAsReadTimer_;
        
        void markAsRead(EmpathMessageListItem *);
        
        /**
         * Mark with the status given
         */
        void mark(RMM::MessageStatus);

        /**
         * Flip the flag(s) in the given status
         */
        void markOne(RMM::MessageStatus);
        
        static QListViewItem * lastSelected_;
        
        EmpathMessageListItemList selected_;
        
        unsigned int listenTo_;
        
        // Order dependency
        bool                filling_;
        Q_UINT32            itemListCount_;
        // End order dependency
};

#endif

// vim:ts=4:sw=4:tw=78
