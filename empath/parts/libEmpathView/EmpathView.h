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

#ifndef EMPATH_VIEW_H
#define EMPATH_VIEW_H

// Qt includes
#include <qwidget.h>

// KDE includes
#include <kparts/part.h>
#include <klibloader.h>

// Local includes
#include "EmpathURL.h"

class EmpathFolderListWidget;
class EmpathMessageListWidget;

class EmpathView : public QWidget
{
    Q_OBJECT

    public:

        EmpathView(QWidget * parent);
        ~EmpathView();

        EmpathFolderListWidget  * folderListWidget_;
        EmpathMessageListWidget * messageListWidget_;
        KParts::ReadOnlyPart    * messageViewPart_;
};

class EmpathViewPartFactory : public KLibFactory
{
    Q_OBJECT

    public:

        EmpathViewPartFactory();
        virtual ~EmpathViewPartFactory();

        virtual QObject * create(
            QObject * parent = 0,
            const char * name = 0,
            const char * classname = "QObject",
            const QStringList & args = QStringList());

        static KInstance * instance();

    private:

        static KInstance * instance_;
};

class EmpathViewPart : public KParts::ReadWritePart
{
    Q_OBJECT

    public:

        EmpathViewPart(QWidget * parent = 0, const char * name = 0);
        virtual ~EmpathViewPart();

    protected slots:

        void s_showFolder(const EmpathURL &);
        void s_messageView();
        void s_messageCompose();
        void s_messageReply();
        void s_messageReplyAll();
        void s_messageForward();
        void s_messageDelete();
        void s_messageBounce();
        void s_messageSaveAs();
        void s_messageCopyTo();
        void s_messageMoveTo();
        void s_messageMarkMany();
        void s_messagePrint();
        void s_messageFilter();
        void s_threadExpand();
        void s_threadCollapse();
        void s_goPrevious();
        void s_goNext();
        void s_goNextUnread();
        void s_messageMark();
        void s_messageMarkRead();
        void s_messageMarkReplied();
        void s_toggleHideRead();
        void s_toggleThread();

    signals:

        void showFolder(const EmpathURL &);

    protected:

        virtual bool openFile() { return false; }
        virtual bool saveFile() { return false; }

    private:

        void _initActions();
        EmpathView * widget_;
};


#endif
// vim:ts=4:sw=4:tw=78
