/*
    Twister - PIM app for KDE

    Copyright 2000
        Rik Hemsley <rik@kde.org>
    
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

// KDE includes
#include <kconfig.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kglobal.h>

// Local includes
#include "TwisterSideBar.h"

TwisterSideBar::TwisterSideBar(QWidget * parent)
    : QIconView(parent, "TwisterSideBar")
{
    setSelectionMode(Single);
    setGridX(72);
    setGridY(48);
    setArrangement(TopToBottom);
    setItemsMovable(false);
    setWordWrapIconText(false);
    setSorting(false);
    setShowToolTips(false);
    setAutoArrange(true);

    new QIconViewItem(this, "Inbox", BarIcon("sideBarMail"));
    new QIconViewItem(this, "Calendar", BarIcon("sideBarCalendar"));
    
    setFixedWidth(sizeHint().width());
    
    QObject::connect(
        this,   SIGNAL(currentChanged(QIconViewItem *)),
        this,   SLOT(s_currentChanged(QIconViewItem *)));
}

TwisterSideBar::~TwisterSideBar()
{
    // Empty.
}

    void
TwisterSideBar::s_currentChanged(QIconViewItem * i)
{
    qDebug("TwisterSideBar::s_currentChanged(%s)", i->text().ascii());
    emit(switchWidget(i->text()));
}

// vim:ts=4:sw=4:tw=78
#include "TwisterSideBar.moc"
