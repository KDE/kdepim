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
# pragma interface "EmpathSeparatorWidget.h"
#endif

#ifndef EMPATHSEPARATORWIDGET_H
#define EMPATHSEPARATORWIDGET_H

// Qt includes
#include <qstring.h>
#include <qframe.h>
#include <qlabel.h>
#include <qwidget.h>

/**
 * @short Dialog separator widget
 * @author Rikkus
 */
class EmpathSeparatorWidget : public QWidget
{
    Q_OBJECT
    
    public:
    
        EmpathSeparatorWidget(const QString &, QWidget * parent);
        ~EmpathSeparatorWidget();

        QString text() const;
        void setText(const QString &);
        
    protected:

        QLabel  * l_text_;
};

#endif

// vim:ts=4:sw=4:tw=78
