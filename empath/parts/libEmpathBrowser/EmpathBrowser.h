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

#ifndef EMPATH_BROWSER_H
#define EMPATH_BROWSER_H

// Qt includes
#include <qwidget.h>

// KDE includes
#include <kparts/part.h>
#include <klibloader.h>

// KDE includes
#include <kparts/part.h>

// Local includes
#include "EmpathURL.h"
#include "EmpathJob.h"

#include "RMM_Message.h"

class EmpathBrowser : public QWidget
{
    Q_OBJECT

    public:
        
        EmpathBrowser(QWidget * parent);
        ~EmpathBrowser();

    protected slots:

        void s_showFolder(const EmpathURL &);
        void s_changeView(const QString &);
        void s_reply(const QString &);
        void s_replyAll(const QString &);
        void s_forward(const QString &);
        void s_bounce(const QString &);
        void s_remove(const QStringList &);
        void s_save(const QString &);
        void s_copy(const QStringList &);
        void s_move(const QStringList &);
        void s_print(const QStringList &);
        void s_filter(const QStringList &);
        void s_view(const QString &);

        void s_toggleHideRead();
        void s_toggleThread();

        void s_retrieveJobComplete(EmpathRetrieveJob);

    signals:

        void changeView(RMM::RMessage &);
        void setIndex(const QDict<EmpathIndexRecord> &);
        void toggleHideRead();
        void toggleThread();

    private:

        void _connectUp();

        EmpathURL currentFolder_;

        KParts::ReadWritePart * folderListWidget_;
        KParts::ReadWritePart * messageListWidget_;
        KParts::ReadOnlyPart  * messageViewWidget_;
};

class EmpathBrowserPartFactory : public KLibFactory
{
    Q_OBJECT

    public:

        EmpathBrowserPartFactory();
        virtual ~EmpathBrowserPartFactory();

        virtual QObject * create(
            QObject * parent = 0,
            const char * name = 0,
            const char * classname = "QObject",
            const QStringList & args = QStringList());

        static KInstance * instance();

    private:

        static KInstance * instance_;
};

class EmpathBrowserPart : public KParts::ReadWritePart
{
    Q_OBJECT

    public:
        
        EmpathBrowserPart(QWidget * parent = 0, const char * name = 0);
        virtual ~EmpathBrowserPart();
        void _initActions();

    protected slots:

    signals:
 
    protected:

        virtual bool openFile() { return false; }
        virtual bool saveFile() { return false; }

        void enableAllActions(bool);

    private:

        EmpathBrowser * widget_;
};


#endif
// vim:ts=4:sw=4:tw=78
