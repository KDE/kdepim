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

// Local includes
#include "EmpathComposeForm.h"

class QMultiLineEdit;
class QSplitter;
class QActionCollection;

class EmpathEnvelopeWidget;
class EmpathAttachmentListWidget;

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
        EmpathComposeWidget(EmpathComposeForm, QWidget * parent);

        ~EmpathComposeWidget();

        EmpathComposeForm composeForm();

        QActionCollection * actionCollection() { return actionCollection_; }
        
    protected slots:
        
        void s_editorDone(bool ok, QCString text);
        
    private:

        void _initActions();
    
        EmpathComposeForm composeForm_;
        
        EmpathEnvelopeWidget        * envelopeWidget_;
        QMultiLineEdit              * editorWidget_;
        EmpathAttachmentListWidget  * attachmentWidget_;

        QActionCollection * actionCollection_;
};

#endif
// vim:ts=4:sw=4:tw=78
