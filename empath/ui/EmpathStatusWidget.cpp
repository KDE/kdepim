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
# pragma implementation "EmpathStatusWidget.h"
#endif

// Local includes
#include "EmpathDefines.h"
#include "EmpathStatusWidget.h"
#include "EmpathTaskWidget.h"

EmpathStatusWidget::EmpathStatusWidget(QWidget * parent, const char * name)
    :    QWidget(parent, name),
        bottom_(0)
{
    empathDebug("ctor");
}

EmpathStatusWidget::~EmpathStatusWidget()
{
    empathDebug("dtor");
    
    QListIterator<QWidget> it(widgetList_);
    
    for (; it.current(); ++it) {
        
        it.current()->move(0, bottom_);
        bottom_ += it.current()->height();
    }

}

    int
EmpathStatusWidget::addWidget(QWidget * widget)
{
    empathDebug("addWidget() called");
    EmpathTaskWidget * tw = new EmpathTaskWidget(this, "EmpathTaskWidget");
    tw->manage(widget);
    
    widgetList_.append(tw);
    tw->move(0, bottom_);
    bottom_ += tw->height();
    tw->resize(width(), tw->height());
    return 0;
}

    void
EmpathStatusWidget::removeWidget(QWidget * widget)
{
    empathDebug("removeWidget() called");
}


// vim:ts=4:sw=4:tw=78
