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
# pragma interface "EmpathFolderListItem.h"
#endif

#ifndef EMPATHFOLDERLISTITEM_H
#define EMPATHFOLDERLISTITEM_H

// Qt includes
#include <qlistview.h>
#include <qlist.h>

// Local includes
#include "EmpathURL.h"

class EmpathFolderListItem : public QObject, public QListViewItem
{
    Q_OBJECT

    public:
        
        EmpathFolderListItem(QListView * parent, const EmpathURL & url);
        EmpathFolderListItem(QListViewItem * parent, const EmpathURL & url);
        virtual ~EmpathFolderListItem();
        
        virtual void setup();
        
        const EmpathURL & url() const { return url_; }

        QString key(int, bool) const;
        
        void tag(bool) { tagged_ = true; }
        bool isTagged() { return tagged_; }
        
        void setOpen(bool o);
        
    signals:
        
        void opened();
        void update();
        
    protected slots:

        void s_setCount(unsigned int, unsigned int);
    
    public slots:

        void s_update();    
    
    private:
    
        EmpathURL url_;
        bool tagged_;
};

#endif
// vim:ts=4:sw=4:tw=78
