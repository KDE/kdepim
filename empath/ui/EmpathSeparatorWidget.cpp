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
# pragma implementation "EmpathSeparatorWidget.h"
#endif

// Qt includes
#include <qframe.h>
#include <qlayout.h>

// Local includes
#include "EmpathSeparatorWidget.h"

EmpathSeparatorWidget::EmpathSeparatorWidget
        (QWidget * parent)
    :   QLabel(parent, "Separator")
{
    setFrameStyle(QFrame::HLine | QFrame::Sunken);
    setLineWidth(2);
    setFixedHeight(8);
}

EmpathSeparatorWidget::~EmpathSeparatorWidget()
{
    // Empty.
}

// vim:ts=4:sw=4:tw=78
