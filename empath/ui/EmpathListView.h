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
# pragma interface "EmpathListView.h"
#endif

#ifndef EMPATHLISTVIEW_H
#define EMPATHLISTVIEW_H

// Qt includes
#include <qlistview.h>

// Local includes
#include "Empath.h"

class QTimer;

/**
 * @short Base class for list views in Empath. Allows some funky stuff.
 * 
 * @author Wilco Greven
 */
class EmpathListView : public QListView
{
    Q_OBJECT

    public:
        
        EmpathListView(
            QWidget * parent = 0,
            const char * name = 0);

        virtual ~EmpathListView();

        enum UpdateAction { DoNothing, Revert, Update };

        void setUpdateLink(bool flag, UpdateAction actionOnUpdate = DoNothing ); 
        bool isUpdateLink() { return updateLink_; };

        /**
         * When set TRUE the link isn't shown immediately when the cursor is
         * moved using the keyboard, but after a short delay. 
         */
        void setDelayedLink(bool flag) { delayedLink_ = flag; };
        bool isDelayedLink() { return delayedLink_; };

    protected slots:

        void s_currentChanged   (QListViewItem *);
        void s_updateLink       ();
        void s_updateLink       (QListViewItem *);
        virtual void s_showLink (QListViewItem *);
        virtual void s_showing  ();          

    signals:
        
        void showLink(QListViewItem *);

    protected:

        virtual void startDrag (QListViewItem *);
        
        virtual void contentsMousePressEvent    (QMouseEvent *);
        virtual void contentsMouseReleaseEvent  (QMouseEvent *);
        virtual void contentsMouseMoveEvent     (QMouseEvent *);
        virtual void keyPressEvent              (QKeyEvent *);

    private:

        bool updateLink_;
        QListViewItem * linkedItem_;

        bool delayedLink_;
        QTimer * delayedLinkTimer_;

        bool waitForLink_;

        // DnD stuff
        QPoint pressPos_;
        bool dragEnabled_;
        bool maybeDrag_;

};

#endif

// vim:ts=4:sw=4:tw=78
