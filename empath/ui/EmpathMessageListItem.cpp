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
#include <klocale.h>
#include <kiconloader.h>

// Local includes
#include "EmpathMessageListWidget.h"
#include "EmpathMessageListItem.h"
#include "EmpathUIUtils.h"
#include "EmpathConfig.h"

QPixmap * EmpathMessageListItem::px_                      = 0L;
QPixmap * EmpathMessageListItem::px_read_                 = 0L;
QPixmap * EmpathMessageListItem::px_marked_               = 0L;
QPixmap * EmpathMessageListItem::px_replied_              = 0L;
QPixmap * EmpathMessageListItem::px_read_marked_          = 0L;
QPixmap * EmpathMessageListItem::px_read_replied_         = 0L;
QPixmap * EmpathMessageListItem::px_marked_replied_       = 0L;
QPixmap * EmpathMessageListItem::px_read_marked_replied_  = 0L;

QColor  * EmpathMessageListItem::unreadColour_            = 0L;

EmpathMessageListItem::EmpathMessageListItem(
        EmpathMessageListWidget * parent,
        EmpathIndexRecord rec)
    :    
        QListViewItem(parent),
        m_(rec)
{
    _init();
}

EmpathMessageListItem::EmpathMessageListItem(
        EmpathMessageListItem * parent,
        EmpathIndexRecord rec)
    :    
        QListViewItem(parent),
        m_(rec)
{
    _init();
}

EmpathMessageListItem::~EmpathMessageListItem()
{
}

    void
EmpathMessageListItem::_init()
{ 
    setOpen(true);

    // Subject
    setText(0, m_.subject());
 
    // Status
    setPixmap(1, _statusIcon(m_.status()));
 
    // Sender
    RMM::RAddress sender_(m_.sender());

    if (sender_.phrase().isEmpty())
        setText(2, sender_.asString());
        
    else {

        QString s = sender_.phrase();

        if (s.left(1) == "\"") {
            s.remove(0, 1);
            if (s.right(1) == "\"")
                s.remove(s.length() - 1, 1);
        }

        setText(2, s);
    }

    // Date 
    QString niceDate = KGlobal::locale()->formatDate(m_.date().qdt().date(), true);
    setText(3, niceDate);
   
    // Size
    QString sizeStr;
    
    // Why does using floats not work ?
    // Anyway, this should handle up to 9999 Mb (nearly 10G).
    // I hope that 10G mail messages won't exist during my lifetime ;)
    
    Q_UINT32 size_(m_.size());

    if (size_ < 1024) {
        
        sizeStr = i18n("%1 B");
        sizeStr = sizeStr.arg((Q_UINT32)size_, 4);
    
    } else {
    
        if (size_ < 1048576) {
    
            sizeStr = i18n("%1 kB");
            sizeStr = sizeStr.arg((Q_UINT32)(size_ / 1024.0), 4);
    
        } else {
    
            sizeStr = i18n("%1 MB");
            sizeStr = sizeStr.arg((Q_UINT32)(size_ / 1048576.0), 4);
        }
    }
    
    setText(4, sizeStr);

   // These are used only for sorting.
    dateStr_.sprintf("%016x", m_.date().asUnixTime());
    sizeStr_.sprintf("%016x", size_);
}

    void
EmpathMessageListItem::setup()
{
    widthChanged();
    int ph = pixmap(1) ? pixmap(1)->height() : 0;
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
            s = text(0); // TODO: sort on status
            break;
        
        case 2:
            s = text(2);
            break;
            
        case 3:
            s = dateStr_;
            break;
            
        case 4:
            s = sizeStr_;
            break;
            
        default:
            break;
    }
    
    return s;
}

    void
EmpathMessageListItem::setStatus(RMM::MessageStatus status)
{
    m_.setStatus(status);
    setPixmap(1, _statusIcon(status));
}

    QPixmap &
EmpathMessageListItem::_statusIcon(RMM::MessageStatus status)
{
    bool read = status & RMM::Read;
    bool marked = status & RMM::Marked;
    bool replied = status & RMM::Replied;

    if (read)
        if (marked)
            if (replied)
                return *px_read_marked_replied_;
            else
                return *px_read_marked_;
        else
            return *px_read_;
    else
        if (marked)
            if (replied)
                return *px_marked_replied_;
            else
                return *px_marked_;
        else
            return *px_;

    return *px_;
}

    void
EmpathMessageListItem::paintCell(
    QPainter * p, const QColorGroup & cg, int column, int width, int align)
{
    if (m_.status() & RMM::Read)
        QListViewItem::paintCell(p, cg, column, width, align);

    else {

        QColorGroup modified(cg);
        modified.setColor(QColorGroup::Text, *unreadColour_);
        QListViewItem::paintCell(p, modified, column, width, align);
    }
}

    void
EmpathMessageListItem::initStatic()
{
#define BOLLOX(a) new QPixmap(KGlobal::iconLoader()->loadIcon((a)))

    px_                     = BOLLOX("tree");
    px_read_                = BOLLOX("tree-read");
    px_marked_              = BOLLOX("tree-marked");
    px_replied_             = BOLLOX("tree-replied");
    px_read_marked_         = BOLLOX("tree-read-marked");
    px_read_replied_        = BOLLOX("tree-read-replied");
    px_marked_replied_      = BOLLOX("tree-marked-replied");
    px_read_marked_replied_ = BOLLOX("tree-read-marked-replied");

#undef BOLLOX

    KConfig * c(KGlobal::config());
    using namespace EmpathConfig;
    c->setGroup(GROUP_DISPLAY);
    unreadColour_ = new QColor(c->readColorEntry(UI_NEW, DFLT_NEW));
}

// vim:ts=4:sw=4:tw=78
