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

#ifndef EMPATH_LIST_VIEW_H
#define EMPATH_LIST_VIEW_H

// Qt includes
#include <klistview.h>
#include <qlist.h>

class QTimer;

/**
 * @short Base class for list views in Empath. Allows some funky stuff.
 * 
 * @author Wilco Greven
 */
class EmpathListView : public KListView
{
    Q_OBJECT

    public:
        
        EmpathListView(
            QWidget * parent = 0,
            const char * name = 0);

        virtual ~EmpathListView();

        enum Area { Item, OpenClose, Void };

        /**
         * When set TRUE the link isn't shown immediately when the cursor is
         * moved using the keyboard, but after a short delay. 
         */
        void setDelayedLink(bool flag) { delayedLink_ = flag; };
        bool isDelayedLink() { return delayedLink_; };

        virtual void setLinkItem(QListViewItem *);
        QListViewItem * linkItem() const
            { return linkItem_; };
        
        QListViewItem * itemAt(const QPoint & screenPos) const 
            { return QListView::itemAt(screenPos); };
        QListViewItem * itemAt(const QPoint & screenPos, 
            Area & areaAtPos) const;
       
        QList<QListViewItem> thread(QListViewItem *);
        QList<QListViewItem> subThread(QListViewItem *);
        
    protected slots:

        void s_currentChanged       (QListViewItem *);
        void s_delayedLinkTimeout   ();

    signals:
      
        void rightButtonPressed(QListViewItem *, const QPoint &, int);
        void rightButtonPressed(QListViewItem *, const QPoint &, int, Area);
        
        void linkChanged(QListViewItem *);
        void startDrag(const QList<QListViewItem> &); // Add another one for single selection(?)

    protected:

        virtual void contentsMousePressEvent    (QMouseEvent *);
        virtual void contentsMouseReleaseEvent  (QMouseEvent *);
        virtual void contentsMouseMoveEvent     (QMouseEvent *);
        virtual void keyPressEvent              (QKeyEvent *);

    private:

        QListViewItem * linkItem_;

        bool delayedLink_;
        QTimer * delayedLinkTimer_;

        bool waitForLink_;

        // DnD stuff
        QPoint pressPos_;
        QListViewItem * pressItem_;
        Area pressArea_;
        bool dragEnabled_;
        bool maybeDrag_;

};

#endif

// vim:ts=4:sw=4:tw=78
