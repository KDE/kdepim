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

#ifdef __GNUG__
# pragma implementation "EmpathMessageListItem.h"
#endif

// Qt includes
#include <qstring.h>

// KDE includes
#include <kconfig.h>
#include <kglobal.h>

// Local includes
#include "EmpathMessageListWidget.h"
#include "EmpathMessageListItem.h"
#include "EmpathUIUtils.h"
#include "EmpathConfig.h"

EmpathMessageListItem::EmpathMessageListItem(
        EmpathMessageListWidget * parent,
        EmpathIndexRecord rec)
    :    
        QListViewItem(parent),
        m(rec)
{
    niceDate_ = KGlobal::locale()->formatDateTime(rec.date().qdt());
    _init();
}

EmpathMessageListItem::EmpathMessageListItem(
        EmpathMessageListItem * parent,
        EmpathIndexRecord rec)
    :    
        QListViewItem(parent),
        m(rec)
{
    niceDate_ = KGlobal::locale()->formatDateTime(rec.date().qdt());
    _init();
}

EmpathMessageListItem::~EmpathMessageListItem()
{
}

    void
EmpathMessageListItem::_init()
{    
    setText(0, m.subject());
    setText(2, niceDate_);
    
    QString sizeStr;
    
    // Why does using floats not work ?
    // Anyway, this should handle up to 9999 Mb (nearly 10G).
    // I hope that 10G mail messages won't exist during my lifetime ;)
    
    Q_UINT32 size_(m.size());

    if (size_ < 1024) {
        
        sizeStr = "%1 B";
        sizeStr = sizeStr.arg((Q_UINT32)size_, 4);
    
    } else {
    
        if (size_ < 1048576) {
    
            sizeStr = "%1 kB";
            sizeStr = sizeStr.arg((Q_UINT32)(size_ / 1024.0), 4);
    
        } else {
    
            sizeStr = "%1 MB";
            sizeStr = sizeStr.arg((Q_UINT32)(size_ / 1048576.0), 4);
        }
    }
    
    setText(3, sizeStr);
    
    RMM::RMailbox sender_(m.sender());
    
    if (sender_.phrase().isEmpty()) {
        sender_.assemble();
        setText(1, sender_.asString());
    }
        
    else {

        QString s = sender_.phrase();
        if (s.left(1) == "\"") {
            s.remove(0, 1);
            if (s.right(1)    == "\"")
                s.remove(s.length() - 1, 1);
        }
        setText(1, s);
    }
    
    dateStr_.sprintf("%016x", m.date().asUnixTime());
    
    sizeStr_.sprintf("%016x", size_);
}

    void
EmpathMessageListItem::setup()
{
    widthChanged();
    int ph = pixmap(0) ? pixmap(0)->height() : 0;
    int th = QFontMetrics(KGlobal::generalFont()).height();
    setHeight(QMAX(ph, th));
}

    QString
EmpathMessageListItem::key(int column, bool) const
{
    QString s;
    
    switch (column) {
        
        case 0:
            s = text(0);
            break;
            
        case 1:
            s = text(1);
            break;
            
        case 2:
            s = dateStr_;
            break;
            
        case 3:
            s = sizeStr_;
            break;
            
        default:
            break;
    }
    
    return s;
}

    void
EmpathMessageListItem::setStatus(RMM::MessageStatus s)
{
    m.setStatus(s);
}

    void
EmpathMessageListItem::paintCell(
    QPainter * p, const QColorGroup & cg, int column, int width, int align)
{
    if (m.status() & RMM::Read)
        QListViewItem::paintCell(p, cg, column, width, align);

    else {

        KConfig * c(KGlobal::config());
        using namespace EmpathConfig;
        c->setGroup(GROUP_DISPLAY);
        QColor col = c->readColorEntry(UI_NEW, &DFLT_NEW);
        QColorGroup modified(cg);
        modified.setColor(QColorGroup::Text, col);
        QListViewItem::paintCell(p, modified, column, width, align);
    }
}

// vim:ts=4:sw=4:tw=78
