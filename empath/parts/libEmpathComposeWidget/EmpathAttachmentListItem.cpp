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

// Qt includes
#include <qpixmap.h>

// KDE includes
#include <kapp.h>
#include <kiconloader.h>
#include <kmimetype.h>

// Local includes
#include "EmpathAttachmentListItem.h"

EmpathAttachmentListItem::EmpathAttachmentListItem(
        QListView * parent,
        const EmpathAttachmentSpec & s)
    :   QListViewItem(parent),
        spec_(s)
{
    setSpec(s);
}

EmpathAttachmentListItem::~EmpathAttachmentListItem()    
{    
}
        
    void
EmpathAttachmentListItem::setup()    
{    
    widthChanged();
    
    int th = QFontMetrics(kapp->font()).height();
    
    if (!pixmap(0))
        setHeight(th);
    else 
        setHeight(QMAX(pixmap(0)->height(), th));

}
        
    QString
EmpathAttachmentListItem::key(int, bool) const
{
    return text(0);
}

    void
EmpathAttachmentListItem::setSpec(const EmpathAttachmentSpec & s)
{
    spec_ = s;

    QString mimeType = spec_.type() + spec_.subType();

    // Apparently KMimeType::mimeType() never returns NULL.

    setPixmap(0, KMimeType::mimeType(mimeType)->pixmap(KIcon::Small));

    setText(0, spec_.filename());
    setText(1, spec_.type());
    setText(2, spec_.subType());
    setText(3, spec_.charset());
    setText(4, "TODO");
//    setText(4, spec_.encoding());
    setText(5, spec_.description());
}

// vim:ts=4:sw=4:tw=78
