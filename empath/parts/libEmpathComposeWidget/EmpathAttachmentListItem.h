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

#ifndef EMPATH_ATTACHMENT_LIST_ITEM_H
#define EMPATH_ATTACHMENT_LIST_ITEM_H

// Qt includes
#include <qlist.h>
#include <qlistview.h>

// Local includes
#include "EmpathAttachmentSpec.h"

class EmpathAttachmentListItem : public QListViewItem
{
    public:
        
        EmpathAttachmentListItem(
            QListView * parent, const EmpathAttachmentSpec &);

        virtual ~EmpathAttachmentListItem();
        
        virtual void setup();
        
        const EmpathAttachmentSpec & spec() const { return spec_; }
        void setSpec(const EmpathAttachmentSpec & s);

        QString key(int, bool) const;
        
        const char * className() const { return "EmpathAttachmentListItem"; }
        
    private:
    
        EmpathAttachmentSpec spec_;
};

typedef QList<EmpathAttachmentListItem> EmpathAttachmentList;
typedef QListIterator<EmpathAttachmentListItem> EmpathAttachmentListIterator;

#endif
// vim:ts=4:sw=4:tw=78
