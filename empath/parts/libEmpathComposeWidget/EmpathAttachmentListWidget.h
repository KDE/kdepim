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

#ifndef EMPATH_ATTACHMENT_LIST_WIDGET_H
#define EMPATH_ATTACHMENT_LIST_WIDGET_H

// Local includes
#include "EmpathListView.h"
#include "EmpathAttachmentSpec.h"

class KActionCollection;

class KAction;

/**
 * This widget shows the structure of a message.
 */
class EmpathAttachmentListWidget : public EmpathListView
{
    Q_OBJECT
    
    public:
        
        EmpathAttachmentListWidget(QWidget * parent);
        ~EmpathAttachmentListWidget();

        EmpathAttachmentSpecList attachments();

        void use(EmpathAttachmentSpecList l);

        KActionCollection * actionCollection() { return actionCollection_; }

    protected slots:
    
        void s_attachmentAdd();
        void s_attachmentEdit();
        void s_attachmentRemove();
        
    private:

        void _initActions();

        KActionCollection * actionCollection_;

        KAction * ac_attachmentAdd;
        KAction * ac_attachmentEdit;
        KAction * ac_attachmentRemove;
};

#endif

// vim:ts=4:sw=4:tw=78
