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
# pragma implementation "EmpathFilterListItem.h"
#endif

// Qt includes
#include <qfont.h>
#include <qstring.h>
#include <qfontmetrics.h>
#include <qpixmap.h>

// KDE includes
#include <kglobal.h>

// Local includes
#include "EmpathUIUtils.h"
#include "EmpathFilterListItem.h"
#include "EmpathFilter.h"
#include "EmpathDefines.h"

EmpathFilterListItem::EmpathFilterListItem(
        QListView * parent,
        EmpathFilter * _filter)
    :
        QListViewItem(parent, _filter->name()),
        filter_(_filter)
{
    empathDebug("ctor");
    setPixmap(0, empathIcon("filter"));
    setText(1, QString().setNum(filter_->priority()));
}

EmpathFilterListItem::~EmpathFilterListItem()
{
}

    QString
EmpathFilterListItem::key(int, bool) const
{
    QString tmpString;
    tmpString.sprintf("%08x", filter_->priority());
    return tmpString;
}

    void
EmpathFilterListItem::setup()
{
    empathDebug("setup() called");
    
    widthChanged();
    int ph = pixmap(0) ? pixmap(0)->height() : 0;
    int th = QFontMetrics(KGlobal::generalFont()).height();
    setHeight(QMAX(ph,th));
}

    EmpathFilter *
EmpathFilterListItem::filter() const
{
    return filter_;
}

// vim:ts=4:sw=4:tw=78
