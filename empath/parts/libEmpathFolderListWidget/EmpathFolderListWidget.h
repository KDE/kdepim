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

#ifndef EMPATH_FOLDER_LIST_WIDGET_H
#define EMPATH_FOLDER_LIST_WIDGET_H

// Qt includes
#include <qlist.h>
#include <qpopupmenu.h>
#include <qpoint.h>

// KDE includes
#include <kparts/part.h>
#include <klibloader.h>

// Local includes
#include "EmpathListView.h"
#include "EmpathURL.h"
#include "EmpathJob.h"

class EmpathFolderListItem;
class EmpathFolder;
class EmpathMailbox;

class EmpathFolderListWidget : public EmpathListView
{
    Q_OBJECT

    public:
        
        EmpathFolderListWidget(QWidget * parent);

        ~EmpathFolderListWidget();
        
        EmpathURL selected() const;

        unsigned int id() const;

    protected slots:

        void s_rightButtonPressed(QListViewItem *, const QPoint &, 
            int, Area);
        void s_mailboxCheck();
        void s_mailboxProperties();
        void s_update();
        void s_newFolder();
        void s_removeFolder();
        void s_setUpAccounts();
        void s_openChanged();
        void s_openCurrent();
        void s_linkChanged(QListViewItem *);
        void s_startDrag(const QList<QListViewItem> &);
        
        void s_jobFinished(EmpathRemoveFolderJob) { s_update(); }
        void s_jobFinished(EmpathCreateFolderJob) { s_update(); }

    signals:

        void showFolder(const EmpathURL & url);

    protected:

        void contentsDragMoveEvent      (QDragMoveEvent *);
        void contentsDragEnterEvent     (QDragEnterEvent *);
        void contentsDragLeaveEvent     (QDragLeaveEvent *);
        void contentsDropEvent          (QDropEvent *);

    private:

        enum OverType { Folder, Mailbox };
        
        void _addMailbox(EmpathMailbox * mailbox);
        void _addChildren(
            EmpathMailbox * m,
            EmpathFolder * item,
            EmpathFolderListItem * parent);
        
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

class EmpathFolderListPartFactory : public KLibFactory
{
    Q_OBJECT

    public:

        EmpathFolderListPartFactory();
        virtual ~EmpathFolderListPartFactory();

        virtual QObject * create(
            QObject * parent = 0,
            const char * name = 0,
            const char * classname = "QObject",
            const QStringList & args = QStringList());

        static KInstance * instance();

    private:

        static KInstance * instance_;
};

class EmpathFolderListPart : public KParts::ReadWritePart
{
    Q_OBJECT

    public:
        
        EmpathFolderListPart(QWidget * parent = 0, const char * name = 0);
        virtual ~EmpathFolderListPart();
        void _initActions();

    protected slots:

    signals:

        void showFolder(const EmpathURL & url);

    protected:

        virtual bool openFile() { return false; }
        virtual bool saveFile() { return false; }

        void enableAllActions(bool);

    private:

        EmpathFolderListWidget * widget_;
};


#endif

// vim:ts=4:sw=4:tw=78
