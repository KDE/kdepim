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
# pragma implementation "EmpathMessageStructureItem.h"
#endif

// Qt includes
#include <qstring.h>
#include <qpixmap.h>
#include <qfont.h>

// KDE includes
#include <kglobal.h>

// Local includes
#include "EmpathMessageStructureItem.h"
#include "EmpathUIUtils.h"

EmpathMessageStructureItem::EmpathMessageStructureItem(
        EmpathMessageStructureItem    * parent,
        RMM::RBodyPart                & part)
    :    
        QListViewItem    (parent),
        part_            (part)
{
    _init();
}

EmpathMessageStructureItem::EmpathMessageStructureItem(
        QListView        * parent,
        RMM::RBodyPart    & part)
    :    
        QListViewItem    (parent),
        part_            (part)
{
    _init();
}

EmpathMessageStructureItem::~EmpathMessageStructureItem()
{
    // Empty.
}

    void
EmpathMessageStructureItem::_init()
{    
    QString sizeStr;
    
    setPixmap(0,
        empathIcon("mime-" + RMM::mimeTypeToIconName(
                part_.mimeType(), part_.mimeSubType())));
        
    setText(0, RMM::mimeTypeEnum2Str(part_.mimeType()));
    setText(1, RMM::mimeSubTypeEnum2Str(part_.mimeSubType()));
    
    Q_UINT32 size_ = part_.size();
    
    if (size_ < 1024) {
        
        sizeStr = "%1 bytes";
        sizeStr = sizeStr.arg((Q_UINT32)size_, 4);
        
    } else {
        
        if (size_ < 1048576) {
        
            sizeStr = "%1 Kb";
            sizeStr = sizeStr.arg((Q_UINT32)(size_ / 1024.0), 4);
        
        } else {
        
            sizeStr = "%1 Mb";
            sizeStr = sizeStr.arg((Q_UINT32)(size_ / 1048576.0), 4);
        }
    }

    setText(2, sizeStr);
}

    void
EmpathMessageStructureItem::setup()
{
    widthChanged();
    int ph = pixmap(0) ? pixmap(0)->height() : 0;
    int th = QFontMetrics(KGlobal::generalFont()).height();
    setHeight(QMAX(ph, th));
}

// vim:ts=4:sw=4:tw=78
