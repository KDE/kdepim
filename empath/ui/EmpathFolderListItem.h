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

#ifndef EMPATH_FOLDER_LIST_ITEM_H
#define EMPATH_FOLDER_LIST_ITEM_H

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

        EmpathFolderListItem * child(const QString &);

        QString key(int, bool) const;
        
        void tag(bool) { tagged_ = true; }
        bool isTagged() { return tagged_; }
        
        void setOpen(bool o);

    signals:
        
        void opened();
        
    protected slots:

        void s_setCount(unsigned int, unsigned int);

    protected:

        virtual void paintCell(QPainter * p, const QColorGroup &, int, int, int);
    
    private:
    
        EmpathURL url_;
        bool tagged_;

        QList<EmpathFolderListItem> childList_;
};

#endif
// vim:ts=4:sw=4:tw=78
