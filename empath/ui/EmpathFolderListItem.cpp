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
#include <qfont.h>
#include <qstring.h>
#include <qfontmetrics.h>
#include <qpixmap.h>
#include <qstringlist.h>

// KDE includes
#include <kconfig.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>

// Local includes
#include "EmpathFolderListItem.h"
#include "EmpathFolder.h"
#include "EmpathMailbox.h"
#include "EmpathDefines.h"
#include "Empath.h"

EmpathFolderListItem::EmpathFolderListItem(
        QListView * parent,
        const EmpathURL & url)
    :
        QListViewItem(parent),
        url_(url),
        tagged_(false)
{
    KConfig * c(KGlobal::config());
    
    c->setGroup("EmpathFolderListWidget");
    
    QStringList l = c->readListEntry("FolderListItemsOpen", ',');
    
    if (l.find(url_.asString()) != l.end())
        setOpen(true);

    EmpathMailbox * m(empath->mailbox(url_));
    
    if (m == 0) {
        empathDebug("Can't find the mailbox !!!!");
        return;
    }

    QObject::connect(
        m,      SIGNAL(countUpdated(unsigned int, unsigned int)),
        this,   SLOT(s_setCount(unsigned int, unsigned int)));
    
    setText(0, m->name());
    setText(1, "...");
    setText(2, "...");
    setPixmap(0, BarIcon(m->pixmapName()));
}

EmpathFolderListItem::EmpathFolderListItem(
        QListViewItem * parent,
        const EmpathURL & url)
    :
        QListViewItem(parent),
        url_(url),
        tagged_(false)
{
    KConfig * c(KGlobal::config());
    
    c->setGroup("EmpathFolderListWidget");

    QStringList l = c->readListEntry("FolderListItemsOpen", ',');
    
    if (l.find(url_.asString()) != l.end())
        setOpen(true);

    EmpathFolder * f(empath->folder(url_));
    
    if (f == 0) {
        empathDebug("Can't find my folder !");
        return;
    }

    QObject::connect(
        f,      SIGNAL(countUpdated(unsigned int, unsigned int)),
        this,   SLOT(s_setCount(unsigned int, unsigned int)));
    
    QString s = url_.folderPath();

    if (s.right(1) == "/")
        s = s.remove(s.length(), 1);

    s = s.right(s.length() - s.findRev("/") - 1);
    
    setText(0, s);

    setPixmap(0, BarIcon(f->pixmapName()));
}
    
EmpathFolderListItem::~EmpathFolderListItem()
{
}

    QString
EmpathFolderListItem::key(int column, bool) const
{
    QString k;

    if (column != 0 || !url_.isFolder())    k = text(column);
    else if (url_ == empath->inbox())       k = '0';
    else if (url_ == empath->outbox())      k = '1';
    else if (url_ == empath->sent())        k = '2';
    else if (url_ == empath->drafts())      k = '3';
    else if (url_ == empath->trash())       k = '4';
    
    return k;
}

    void
EmpathFolderListItem::setup()
{
    widthChanged();
    
    int th = QFontMetrics(KGlobalSettings::generalFont()).height();
    
    if (!pixmap(0))
        setHeight(th);
    else 
        setHeight(QMAX(pixmap(0)->height(), th));
}

    void
EmpathFolderListItem::paintCell(
    QPainter * p, const QColorGroup & cg, int column, int width, int align)
{
    if ((text(1)[0] == '0') || (text(1).isEmpty()))
        QListViewItem::paintCell(p, cg, column, width, align);

    else {

        KConfig * c(KGlobal::config());
        c->setGroup("EmpathFolderListWidget");
//        QColor defaultNewColour = Qt::darkRed;
        QColor col = Qt::darkRed; // FIXME c->readColorEntry(UI_NEW, &defaultNewColour);
        QColorGroup modified(cg);
        modified.setColor(QColorGroup::Text, col);
        QListViewItem::paintCell(p, modified, column, width, align);
    }
}

    void
EmpathFolderListItem::s_setCount(unsigned int read, unsigned int unread)
{
    setText(1, QString::number(unread));
    setText(2, QString::number(read));
}

    void
EmpathFolderListItem::setOpen(bool o)
{
    emit(opened());
    QListViewItem::setOpen(o);
}

    EmpathFolderListItem *
EmpathFolderListItem::child(const QString & name)
{
    QListIterator<EmpathFolderListItem> it(childList_);

    for (; it.current(); ++it)
        if (it.current()->name() == name)
            return it.current();

    EmpathURL url(url_);
    url.setFolderPath(url.folderPath() + "/" + name);
    return new EmpathFolderListItem(this, url);
}

// vim:ts=4:sw=4:tw=78
#include "EmpathFolderListItem.moc"
