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
# pragma interface "EmpathFolderWidget.h"
#endif

#ifndef EMPATHFOLDERWIDGET_H
#define EMPATHFOLDERWIDGET_H

// Qt includes
#include <qpixmap.h>
#include <qlist.h>
#include <qpopupmenu.h>
#include <qpoint.h>

// Local includes
#include "EmpathListView.h"
#include "EmpathDefines.h"
#include "EmpathFolder.h"
#include "EmpathMailbox.h"
#include "EmpathFolderListItem.h"
#include "EmpathURL.h"
#include "Empath.h"

class EmpathFolderWidget : public EmpathListView
{
    Q_OBJECT

    public:
        
        EmpathFolderWidget(
            QWidget * parent = 0,
            const char * name = 0,
            bool waitForShown = false);

        ~EmpathFolderWidget() { empathDebug("dtor"); }
        
        void update();

        EmpathURL selected() const;

    protected slots:

        void s_rightButtonPressed(QListViewItem *, const QPoint &, int);
        void s_folderProperties();
        void s_mailboxCheck();
        void s_mailboxProperties();
        void s_update();
        void s_newFolder();
        void s_removeFolder();
        void s_setUpAccounts();
        void s_openChanged();
        void s_openCurrent();
        void s_showLink(QListViewItem *);
        
    signals:

        void showFolder(const EmpathURL & url);

    protected:

        void startDrag(QListViewItem *);
        
        void contentsDragMoveEvent      (QDragMoveEvent *);
        void contentsDragEnterEvent     (QDragEnterEvent *);
        void contentsDragLeaveEvent     (QDragLeaveEvent *);
        void contentsDropEvent          (QDropEvent *);

    private:

        enum OverType { Folder, Mailbox };
        
        void _addMailbox(const EmpathMailbox & mailbox);
        void _addChildren(EmpathFolder * item, EmpathFolderListItem * parent);
        
        EmpathFolderListItem * find(const EmpathURL &);
        
        QPopupMenu folderPopup_;
        QPopupMenu mailboxPopup_;
        QPopupMenu otherPopup_;
        
        QList<EmpathFolderListItem> itemList_;
        
        EmpathFolderListItem    * popupMenuOver;
        OverType                popupMenuOverType;
        
        QTimer *        autoOpenTimer;
        int             autoOpenTime;
        QListViewItem * dropItem;

        QStrList dragContents_;
        
        int autoscrollMargin;
        void startAutoScroll();
        void stopAutoScroll();
};

#endif

// vim:ts=4:sw=4:tw=78
