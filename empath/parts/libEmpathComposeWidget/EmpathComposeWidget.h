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

#ifndef EMPATH_COMPOSE_WIDGET_H
#define EMPATH_COMPOSE_WIDGET_H

// Qt includes
#include <qwidget.h>

// KDE includes
#include <kparts/part.h>
#include <klibloader.h>

// Local includes
#include "EmpathComposePart.h"

class QSplitter;
class KActionCollection;

class EmpathEnvelopeWidget;
class EmpathAttachmentListWidget;

namespace KTextEditor
{
    class Document;
    class View;
}

/**
 * The container for the various widgets used when composing.
 */
class EmpathComposeWidget : public QWidget
{
    Q_OBJECT

    public:
        
        /**
         * Pass a compose form in, so we've got something
         * to work with.
         */
        EmpathComposeWidget(QWidget * parent);

        ~EmpathComposeWidget();

        void setForm(const EmpathComposeForm &);

        EmpathComposeForm form();

        KActionCollection * actionCollection() { return actionCollection_; }
        
    protected slots:
        
        void s_editorDone(bool ok, QCString text);

        void s_sendNow();
        void s_sendLater();
        void s_postpone();
        void s_confirmDelivery();
        void s_encrypt();
        void s_digitallySign();
        void s_sign();
        
    private:

        void _initActions();
    
        EmpathComposeForm composeForm_;
        
        EmpathEnvelopeWidget        * envelopeWidget_;
        KTextEditor::Document       * editorPart_;
        KTextEditor::View           * editorView_;
        EmpathAttachmentListWidget  * attachmentWidget_;

        KActionCollection * actionCollection_;
};

class EmpathComposePartFactory : public KLibFactory
{
    Q_OBJECT

    public:

        EmpathComposePartFactory();
        virtual ~EmpathComposePartFactory();

        virtual QObject * create(
            QObject * parent = 0,
            const char * name = 0,
            const char * classname = "QObject",
            const QStringList & args = QStringList());

        static KInstance * instance();

    private:

        static KInstance * instance_;
};

class MyEmpathComposePart : public EmpathComposePart
{
    Q_OBJECT

    public:

        MyEmpathComposePart(QWidget * parent = 0, const char * name = 0);
        virtual ~MyEmpathComposePart();

        void setForm(const EmpathComposeForm &);

    private:

        void _initActions();
        EmpathComposeWidget * widget_;
};

#endif
// vim:ts=4:sw=4:tw=78
