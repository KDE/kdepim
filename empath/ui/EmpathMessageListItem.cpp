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
#include "RMM_Enum.h"

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
        EmpathIndexRecord & rec)
    :    
        QListViewItem(parent),
        m_(rec)
{
    _init();
}

EmpathMessageListItem::EmpathMessageListItem(
        EmpathMessageListItem * parent,
        EmpathIndexRecord & rec)
    :    
        QListViewItem(parent),
        m_(rec)
{
    _init();
}

EmpathMessageListItem::~EmpathMessageListItem()
{
    // Empty.
}

    void
EmpathMessageListItem::setRecord(EmpathIndexRecord & rec)
{
    m_ = rec;
    _init();
}

    void
EmpathMessageListItem::_init()
{ 
    setOpen(true);

    // Subject
    setText(0, m_.subject());
 
    // Status
    setPixmap(1, _statusIcon(m_.status()));
 
    QString s = m_.senderName();

    if (s.isEmpty())
        setText(2, m_.senderAddress());
        
    else {

        if (s.left(1) == "\"") {
            s.remove(0, 1);
            if (s.right(1) == "\"")
                s.remove(s.length() - 1, 1);
        }

        setText(2, s);
    }

    QDateTime dt = m_.date();

    // Adjust for time zone. TODO: Adjust for _local_ time zone.
    dt.setTime(dt.time().addSecs(60 * m_.timeZone()));

    // Date 
    setText(3, KGlobal::locale()->formatDate(dt.date(), true));
   
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
    
    int sinceEpoch = QDateTime().secsTo(dt);

    // These are used only for sorting.
    dateStr_.sprintf("%016x", sinceEpoch);
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
            s = m_.subject(); 
            break;
            
        case 1:
            s = text(0); // TODO
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
EmpathMessageListItem::setStatus(unsigned int status)
{
    m_.setStatus(status);
    setPixmap(1, _statusIcon(status));
}

    QPixmap &
EmpathMessageListItem::_statusIcon(unsigned int status)
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
