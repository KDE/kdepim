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

QPixmap * EmpathMessageListItem::px_unread_               = 0L;
QPixmap * EmpathMessageListItem::px_read_                 = 0L;
QPixmap * EmpathMessageListItem::px_marked_               = 0L;
QPixmap * EmpathMessageListItem::px_replied_              = 0L;
QPixmap * EmpathMessageListItem::px_attachments_          = 0L;

QColor  * EmpathMessageListItem::unreadColour_            = 0L;

EmpathMessageListItem::EmpathMessageListItem(
        EmpathMessageListWidget * parent,
        const EmpathIndexRecord & rec)
    :    
        QObject(),
        QListViewItem(parent),
        rec_(rec)
{
    _init();
}

EmpathMessageListItem::EmpathMessageListItem(
        EmpathMessageListItem * parent,
        const EmpathIndexRecord & rec)
    :    
        QObject(),
        QListViewItem(parent),
        rec_(rec)
{
    _init();
}

EmpathMessageListItem::~EmpathMessageListItem()
{
    // Empty.
}

    void
EmpathMessageListItem::startAutoMarkTimer()
{
    KConfig * c(KGlobal::config());
    
    c->setGroup("EmpathMessageListWidget");

    int waitTime(c->readNumEntry("AutoMarkReadTimeout"));

    startTimer(waitTime * 1000);
}

    void
EmpathMessageListItem::cancelAutoMarkTimer()
{
    killTimers();
}

    void
EmpathMessageListItem::timerEvent(QTimerEvent * e)
{
    killTimers();
    setStatus(
        EmpathIndexRecord::Status(rec_.status() | EmpathIndexRecord::Read)
    );
}

    void
EmpathMessageListItem::setRecord(const EmpathIndexRecord & rec)
{
    rec_ = rec;
    _init();
}

    void
EmpathMessageListItem::_init()
{ 
    setOpen(true);

    // Subject
    setText(0, rec_.subject());

    // Status
    _setStatusIcons();

    // Sender
    QString s = rec_.senderName();

    if (s.isEmpty())
        s = rec_.senderAddress();
        
    else {

        if (s.left(1) == "\"") {
            s.remove(0, 1);
            if (s.right(1) == "\"")
                s.remove(s.length() - 1, 1);
        }
    }
    
    setText(5, s);

    QDateTime dt = rec_.date();

    // Adjust for time zone. TODO: Adjust for _local_ time zone.
    dt.setTime(dt.time().addSecs(60 * rec_.timeZone()));

    // Date 
    setText(6, KGlobal::locale()->formatDate(dt.date(), true));
   
    // Size
    QString sizeStr;
    
    // Why does using floats not work ?
    // Anyway, this should handle up to 9999 Mb (nearly 10G).
    // I hope that 10G mail messages won't exist during my lifetime ;)
    
    Q_UINT32 size_(rec_.size());

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
    
    setText(7, sizeStr);
    
    int sinceEpoch = QDateTime().secsTo(dt);

    // These are used only for sorting.
    dateStr_.sprintf("%016x", sinceEpoch);
    sizeStr_.sprintf("%016x", size_);
}

    void
EmpathMessageListItem::setup()
{
    widthChanged();
    int th = QFontMetrics(KGlobal::generalFont()).height();
    setHeight(QMAX(20, th)); // 20 is just a little bigger than our icons.
}

    QString
EmpathMessageListItem::key(int column, bool) const
{
    QString s;
    
    switch (column) {
        
        case 0:
            s = rec_.subject(); 
            break;
            
        case 1:
            s = rec_.status() & EmpathIndexRecord::Read ? '1' : '0';
            break;

        case 2:
            s = rec_.status() & EmpathIndexRecord::Marked ? '1' : '0';
            break;

        case 3:
            s = rec_.status() & EmpathIndexRecord::Replied ? '1' : '0';
            break;

        case 4:
            s = rec_.hasAttachments() ? '1' : '0';
            break;
        
        case 5:
            s = text(5);
            break;
            
        case 6:
            s = dateStr_;
            break;
            
        case 7:
            s = sizeStr_;
            break;
            
        default:
            break;
    }
    
    return s;
}

    void
EmpathMessageListItem::setStatus(EmpathIndexRecord::Status status)
{
    if (status != rec_.status())
        emit(mark(rec_.id(), EmpathIndexRecord::Status(status), this));
}
    
    void
EmpathMessageListItem::_setStatusIcons()
{
    if (rec_.status() & EmpathIndexRecord::Read)
       setPixmap(1, *px_read_);
    else
       setPixmap(1, *px_unread_);

    if (rec_.status() & EmpathIndexRecord::Marked)
       setPixmap(2, *px_marked_);
    
    if (rec_.status() & EmpathIndexRecord::Replied)
       setPixmap(3, *px_replied_);
    
    if (!rec_.hasAttachments()) // FIXME this is backwards :)
       setPixmap(4, *px_attachments_);
}

    void
EmpathMessageListItem::paintCell(
    QPainter * p, const QColorGroup & cg, int column, int width, int align)
{
    if (rec_.status() & EmpathIndexRecord::Read)
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

    px_unread_              = BOLLOX("tree");
    px_read_                = BOLLOX("tree-read");
    px_marked_              = BOLLOX("tree-marked");
    px_replied_             = BOLLOX("tree-replied");
    px_attachments_         = BOLLOX("tree-attachments");

#undef BOLLOX

    KConfig * c(KGlobal::config());
    c->setGroup("EmpathMessageListWidget");
    unreadColour_ = new QColor(c->readColorEntry("UnreadMessageColor"));
}

// vim:ts=4:sw=4:tw=78
