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

#ifndef EMPATHSEPARATORWIDGET_H
#define EMPATHSEPARATORWIDGET_H

// Qt includes
#include <qlabel.h>
#include <qframe.h>

// Local includes
#include "EmpathUIUtils.h"

/**
 * @short Dialog separator widget
 * @author Rikkus
 */
class EmpathSeparatorWidget : public QLabel
{
    Q_OBJECT
    
    public:
    
        EmpathSeparatorWidget(QWidget * parent)
            :   QLabel(parent, "Separator")
        {
            setFrameStyle(QFrame::HLine | QFrame::Sunken);
            setLineWidth(1);
            setFixedHeight(dialogSpace);
        }
};

#endif

// vim:ts=4:sw=4:tw=78
