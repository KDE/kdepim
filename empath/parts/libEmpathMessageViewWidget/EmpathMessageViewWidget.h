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

#ifndef EMPATH_MESSAGE_VIEW_WIDGET_H
#define EMPATH_MESSAGE_VIEW_WIDGET_H

// Qt includes
#include <qwidget.h>

// KDE includes
#include <kparts/browserextension.h>
#include <klibloader.h>

// Local includes
#include <RMM_Message.h>

class KAction;
class KInstance;
class EmpathMessageHeaderViewWidget;
class EmpathMessageTextViewWidget;
class EmpathMessageAttachmentViewWidget;

class EmpathMessageViewBrowserExtension;

class EmpathMessageViewWidget : public QWidget
{
    Q_OBJECT

    public:
        
        EmpathMessageViewWidget(QWidget * parent = 0, const char * name = 0);
        virtual ~EmpathMessageViewWidget();

        void compose();
        void reply();
        void replyAll();
        void forward();
        
        void setMessage(RMM::RMessage &);

    private:
        
        EmpathMessageTextViewWidget         * textView_;
        EmpathMessageHeaderViewWidget       * headerView_;
        EmpathMessageAttachmentViewWidget   * attachmentView_;
};

class EmpathMessageViewPartFactory : public KLibFactory
{
    Q_OBJECT

    public:

        EmpathMessageViewPartFactory();
        virtual ~EmpathMessageViewPartFactory();

        virtual QObject * create(
            QObject * parent = 0,
            const char * name = 0,
            const char * classname = "QObject",
            const QStringList & args = QStringList());

        static KInstance * instance();

    private:

        static KInstance * instance_;
};

class EmpathMessageViewPart : public KParts::ReadOnlyPart
{
    Q_OBJECT

    public:
        
        EmpathMessageViewPart(QWidget * parent = 0, const char * name = 0);
        virtual ~EmpathMessageViewPart();

        void compose();
        void reply();
        void replyAll();
        void forward();

    protected:

        virtual bool openFile();

        void enableAllActions(bool);

    private:

        EmpathMessageViewWidget * w;
        
        EmpathMessageViewBrowserExtension * extension_;

        KAction
            * messageCompose_,
            * messageReply_,
            * messageReplyAll_,
            * messageForward_;
};


class EmpathMessageViewBrowserExtension : public KParts::BrowserExtension
{
    Q_OBJECT

    friend class EmpathMessageViewPart;

    public:

        EmpathMessageViewBrowserExtension(EmpathMessageViewPart *);
        virtual ~EmpathMessageViewBrowserExtension() {}

    public slots:

        void compose();
        void reply();
        void replyAll();
        void forward();
};

#endif

// vim:ts=4:sw=4:tw=78
