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
# pragma interface "EmpathAttachmentListWidget.h"
#endif

#ifndef EMPATHATTACHMENTLISTWIDGET_H
#define EMPATHATTACHMENTLISTWIDGET_H

// Qt includes
#include <qwidget.h>
#include <qlistview.h>
#include <qlist.h>
#include <qstring.h>
#include <qpushbutton.h>

// Local includes
#include "EmpathDefines.h"
#include "EmpathAttachmentListItem.h"
#include <RMM_Message.h>

/**
 * This widget shows the structure of a message.
 */
class EmpathAttachmentListWidget : public QListView
{
    Q_OBJECT
    
    public:
        
        EmpathAttachmentListWidget(
            QWidget * parent = 0, const char * name = 0);

        ~EmpathAttachmentListWidget();

        void use(const RMM::RMessage &);
        
        void addAttachment();
        void editAttachment();
        void removeAttachment();
        
    private:

        QListView    * lv_attachments_;
        RMM::RMessage    message_;
};

#endif

// vim:ts=4:sw=4:tw=78
