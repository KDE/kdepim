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

#ifndef EMPATH_MESSAGE_LIST_WIDGET_H
#define EMPATH_MESSAGE_LIST_WIDGET_H

// Qt includes
#include <qpixmap.h>
#include <qlist.h>
#include <qstring.h>
#include <qpopupmenu.h>
#include <qpoint.h>
#include <qdict.h>

// Local includes
#include "EmpathIndexRecord.h"
#include "EmpathListView.h"
#include "EmpathURL.h"

class EmpathMessageListItem;
class KActionCollection;
class KInstance;
class KAction;
class KToggleAction;

/**
 * This is the widget that shows the threaded mail list.
 */
class EmpathMessageListWidget : public EmpathListView
{
    Q_OBJECT

    public:
        
        EmpathMessageListWidget(QWidget * parent);

        virtual ~EmpathMessageListWidget();
        
        EmpathURL       firstSelected();
        EmpathURLList   selection();
        
        void setIndex(const EmpathURL &);

        KActionCollection * actionCollection() { return actionCollection_; }
 
    protected:
        
        virtual void setSelected(QListViewItem *, bool);
        virtual void clearSelection();

        virtual bool event(QEvent *);

    public slots:

        void s_forward();
        void s_bounce();
        void s_remove();
        void s_copyTo();
        void s_moveTo();
        void s_print();
        void s_filter();

        void s_toggleHideRead();
        void s_toggleThread();

        void s_selectTagged();
        void s_selectRead();
        void s_selectAll();
        void s_selectInvert();
   
        void s_goPrevious();
        void s_goNext();
        void s_goNextUnread();
        
        void s_threadExpand();
        void s_threadCollapse();

        void s_messageMark();
        void s_messageMarkRead();
        void s_messageMarkReplied();
        void s_messageMarkMany();

        void s_itemGone(const EmpathIndexRecord &);
        void s_itemCome(const EmpathIndexRecord &);
        
    protected slots:

        void s_showFolder           (const EmpathURL & url)
        { setIndex(url); }
        void s_rightButtonPressed   (QListViewItem *, const QPoint &, int, Area);
        void s_doubleClicked        (QListViewItem *);
        void s_currentChanged       (QListViewItem *);
        void s_startDrag            (const QList<QListViewItem> &);
        
        void s_updateActions        (QListViewItem *);

    signals:

        void messageActivated(const EmpathURL &);

    private:
       
        class ThreadNode {

            public:

                ThreadNode(EmpathIndexRecord * data)
                    :
                    data_(data)
                {
                    childList_.setAutoDelete(true);
                }

                ~ThreadNode()
                {
                    delete data_;
                    data_ = 0L;
                }

                EmpathIndexRecord * data()
                {
                    return data_;
                }
            
                const QList<ThreadNode> & childList() const
                {
                    return childList_;
                }

                void addChild(ThreadNode * n)
                {
                    childList_.append(n);
                }

            private:

                EmpathIndexRecord * data_;

                QList<ThreadNode> childList_; 
        };

        void _saveColumnSizes();
        void _restoreColumnSizes();
         
        void _fillDisplay();
        void _fillNormal();
        void _fillThreading();
       
        EmpathMessageListItem * _threadItem(const EmpathIndexRecord & item);

        void _createThreads(
            ThreadNode * root,
            EmpathMessageListItem * parent = 0L
        );
       
        EmpathMessageListItem * _createListItem(
            const EmpathIndexRecord &,
            EmpathMessageListItem * = 0L
        );
        
        void _init();
        void _connectUp();
    
        void _setSelected(EmpathMessageListItem *, bool);
 
        void _markOne(EmpathIndexRecord::Status);

        KActionCollection * actionCollection_;
        
        QPopupMenu  * messageMenu_;
        QPopupMenu  * multipleMessageMenu_;
        QPopupMenu  * messageMarkMenu_;
        QPopupMenu  * threadMenu_;
 
        QPixmap px_unread_, px_read_, px_marked_, px_attachments_, px_replied_;
        
        QList<EmpathMessageListItem> selected_;
        QDict<EmpathIndexRecord> index_;
        
        int sortColumn_;
        bool sortAscending_;
        bool thread_;
 
        // Order dependency
        bool hideRead_;
        bool filling_;
        // End order dependency

        EmpathURL waitingForIndex_;
        EmpathURL folder_;
};

#endif

// vim:ts=4:sw=4:tw=78
